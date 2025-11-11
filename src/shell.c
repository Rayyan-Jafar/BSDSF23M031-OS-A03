#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>

char *history[HISTORY_SIZE];
int history_count = 0;

bg_job bg_jobs[MAX_BG_JOBS];
int bg_count = 0;

/* --- Shell Variables --- */
variable *var_list = NULL;

void set_variable(const char *name, const char *value) {
    variable *v = var_list;
    while (v) {
        if (strcmp(v->name, name) == 0) {
            free(v->value);
            v->value = strdup(value);
            return;
        }
        v = v->next;
    }
    variable *new_var = malloc(sizeof(variable));
    new_var->name = strdup(name);
    new_var->value = strdup(value);
    new_var->next = var_list;
    var_list = new_var;
}

char* get_variable(const char *name) {
    variable *v = var_list;
    while (v) {
        if (strcmp(v->name, name) == 0)
            return v->value;
        v = v->next;
    }
    return NULL;
}

void expand_variables(char **arglist) {
    for (int i = 0; arglist[i] != NULL; i++) {
        if (arglist[i][0] == '$') {
            char *val = get_variable(arglist[i]+1);
            if (val) {
                free(arglist[i]);
                arglist[i] = strdup(val);
            }
        }
    }
}

void free_variables(void) {
    variable *v = var_list;
    while (v) {
        variable *tmp = v;
        v = v->next;
        free(tmp->name);
        free(tmp->value);
        free(tmp);
    }
    var_list = NULL;
}

/* ---------- Background Job Management ---------- */
void add_bg_job(pid_t pid, const char *cmd) {
    if (bg_count < MAX_BG_JOBS) {
        bg_jobs[bg_count].pid = pid;
        strncpy(bg_jobs[bg_count].cmd, cmd, MAX_LEN-1);
        bg_jobs[bg_count].cmd[MAX_LEN-1] = '\0';
        bg_count++;
    }
}

void remove_bg_job(pid_t pid) {
    for (int i = 0; i < bg_count; i++) {
        if (bg_jobs[i].pid == pid) {
            for (int j = i; j < bg_count-1; j++) bg_jobs[j] = bg_jobs[j+1];
            bg_count--;
            break;
        }
    }
}

void list_bg_jobs(void) {
    for (int i = 0; i < bg_count; i++)
        printf("[%d] PID %d: %s\n", i+1, bg_jobs[i].pid, bg_jobs[i].cmd);
}

void reap_background_jobs(void) {
    int status;
    pid_t pid;
    for (int i = 0; i < bg_count; ) {
        pid = waitpid(bg_jobs[i].pid, &status, WNOHANG);
        if (pid > 0) {
            printf("\n[BG] PID %d finished: %s\n", pid, bg_jobs[i].cmd);
            remove_bg_job(pid);
        } else {
            i++;
        }
    }
}

/* ---------- Handle !n command ---------- */
int handle_bang_command(char **cmdline) {
    if (!cmdline || (*cmdline)[0] != '!') return 0;

    int n = atoi((*cmdline) + 1);
    if (n <= 0 || n > custom_history_count()) {
        printf("No such command in history: %d\n", n);
        return 1;
    }

    free(*cmdline);
    *cmdline = strdup(custom_history_get(n));
    printf("%s\n", *cmdline);
    return 0;
}

/* ---------- Shell History ---------- */
int shell_history(char **args) {
    (void)args;
    for (int i = 0; i < history_count; i++)
        printf("%d %s\n", i+1, history[i]);
    return 1;
}

/* ---------- Custom History ---------- */
void add_to_history(const char *cmd) {
    if (history_count == HISTORY_SIZE) {
        free(history[0]);
        for (int i = 1; i < HISTORY_SIZE; i++) history[i-1] = history[i];
        history_count--;
    }
    history[history_count++] = strdup(cmd);
}

const char *custom_history_get(int n) {
    if (n <= 0 || n > history_count) return NULL;
    return history[n-1];
}

int custom_history_count(void) { return history_count; }

void custom_history_cleanup(void) {
    for (int i = 0; i < history_count; i++) free(history[i]);
    history_count = 0;
    free_variables();
}

/* ---------- Readline Completion ---------- */
char* command_generator(const char *text, int state) {
    static int list_index, len;
    char *name;

    if (!state) {
        list_index = 0;
        len = strlen(text);
    }

    while (list_index < history_count) {
        name = history[list_index++];
        if (strncmp(name, text, len) == 0)
            return strdup(name);
    }

    return NULL;
}

char** custom_completion(const char *text, int start, int end) {
    (void)start;
    (void)end;

    char **matches = rl_completion_matches(text, command_generator);
    if (matches) return matches;

    return rl_completion_matches(text, rl_filename_completion_function);
}

void init_readline(void) {
    rl_attempted_completion_function = custom_completion;
}

/* ---------- Tokenizer ---------- */
char** tokenize(char* cmdline) {
    if (!cmdline || cmdline[0] == '\0' || cmdline[0] == '\n') return NULL;

    char **arglist = malloc(sizeof(char*)*(MAXARGS+1));
    for (int i=0;i<MAXARGS+1;i++) arglist[i] = malloc(sizeof(char)*ARGLEN);

    char *cp = cmdline;
    char *start;
    int len, argnum = 0;

    while (*cp != '\0' && argnum < MAXARGS) {
        while (*cp==' '||*cp=='\t') cp++;
        if (*cp=='\0') break;
        start = cp;
        len = 1;
        while (*++cp!='\0' && *cp!=' ' && *cp!='\t') len++;
        strncpy(arglist[argnum], start, len);
        arglist[argnum][len]='\0';
        argnum++;
    }

    if (argnum==0) {
        for (int i=0;i<MAXARGS+1;i++) free(arglist[i]);
        free(arglist);
        return NULL;
    }

    arglist[argnum] = NULL;
    return arglist;
}

/* ---------- Built-in Commands ---------- */
int handle_builtin(char **arglist) {
    if (!arglist || !arglist[0]) return 0;

    if (strcmp(arglist[0], "exit")==0) {
        if (arglist[1]) {
            char *endptr = NULL;
            long code = strtol(arglist[1], &endptr, 10);
            if (endptr!=arglist[1] && *endptr=='\0') exit((int)code);
            fprintf(stderr,"exit: numeric argument required\n");
            exit(EXIT_FAILURE);
        }
        exit(0);
    }

    if (strcmp(arglist[0], "cd")==0) {
        char *dir = arglist[1]?arglist[1]:getenv("HOME");
        if (!dir) dir="/";
        if (chdir(dir)!=0) fprintf(stderr,"cd: %s: %s\n", dir,strerror(errno));
        return 1;
    }

    if (strcmp(arglist[0], "help")==0) {
        printf("Shell built-in commands:\n  exit [n]\n  cd [dir]\n  help\n  history\n  jobs\n  set\n");
        return 1;
    }

    if (strcmp(arglist[0], "jobs")==0) { list_bg_jobs(); return 1; }
    if (strcmp(arglist[0], "history")==0) { return shell_history(arglist); }
    if (strcmp(arglist[0], "set")==0) {
        variable *v = var_list;
        while (v) { printf("%s=%s\n",v->name,v->value); v=v->next; }
        return 1;
    }

    return 0;
}

/* ---------- Feature 8: Variable Assignment Handler ---------- */
int handle_variable_assignment(char **arglist) {
    if (!arglist || !arglist[0]) return 0;

    char *eq = strchr(arglist[0], '=');
    if (!eq) return 0;

    if (eq == arglist[0]) return 0;

    char *name = strndup(arglist[0], eq - arglist[0]);
    char *value = strdup(eq + 1);

    if (value[0] == '"' || value[0] == '\'') {
        size_t len = strlen(value);
        if (value[len-1] == value[0]) {
            value[len-1] = '\0';
            memmove(value, value+1, len-1);
        }
    }

    set_variable(name, value);
    free(name);
    free(value);

    return 1;
}

/* ---------- Cleanup Variables ---------- */
void cleanup_variables(void) {
    free_variables();
}
char* read_cmd(char* prompt, FILE* fp) {
    char *line = NULL;
    if (fp == stdin) {
        line = readline(prompt);
        if (line && *line != '\0') add_history(line);
    } else {
        size_t n = 0;
        ssize_t len = getline(&line, &n, fp);
        if (len == -1) {
            free(line);
            return NULL;
        }
        if (line[len-1] == '\n') line[len-1] = '\0';
    }
    return line;
}

