#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>

char *history[HISTORY_SIZE];
int history_count = 0;

/* ---------- Readline Completion Functions ---------- */
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

/* ---------- Initialize Readline ---------- */
void init_readline(void) {
    rl_attempted_completion_function = custom_completion;
}

/* ---------- Read Command ---------- */
char* read_cmd(char* prompt, FILE* fp) {
    (void)fp; // ignore FILE*
    char *cmdline = readline(prompt);

    // only add to readline history (arrow keys)
    if (cmdline && *cmdline) {
        add_history(cmdline);
    }

    return cmdline; // caller must handle custom history
}

/* ---------- Tokenizer ---------- */
char** tokenize(char* cmdline) {
    if (!cmdline || cmdline[0] == '\0' || cmdline[0] == '\n') return NULL;

    char **arglist = malloc(sizeof(char*) * (MAXARGS + 1));
    for (int i = 0; i < MAXARGS + 1; i++) {
        arglist[i] = malloc(sizeof(char) * ARGLEN);
        bzero(arglist[i], ARGLEN);
    }

    char *cp = cmdline;
    char *start;
    int len;
    int argnum = 0;

    while (*cp != '\0' && argnum < MAXARGS) {
        while (*cp == ' ' || *cp == '\t') cp++;
        if (*cp == '\0') break;

        start = cp;
        len = 1;
        while (*++cp != '\0' && *cp != ' ' && *cp != '\t') len++;

        strncpy(arglist[argnum], start, len);
        arglist[argnum][len] = '\0';
        argnum++;
    }

    if (argnum == 0) {
        for (int i = 0; i < MAXARGS + 1; i++) free(arglist[i]);
        free(arglist);
        return NULL;
    }

    arglist[argnum] = NULL;
    return arglist;
}

/* ---------- Custom History ---------- */
void add_to_history(const char *cmd) {
    if (history_count == HISTORY_SIZE) {
        free(history[0]);
        for (int i = 1; i < HISTORY_SIZE; i++) history[i - 1] = history[i];
        history_count--;
    }
    history[history_count++] = strdup(cmd);
}

const char *custom_history_get(int n) {
    if (n <= 0 || n > history_count) return NULL;
    return history[n - 1];
}

int custom_history_count(void) {
    return history_count;
}

void custom_history_cleanup(void) {
    for (int i = 0; i < history_count; i++) free(history[i]);
    history_count = 0;
}

/* ---------- Built-in Commands ---------- */
int handle_builtin(char **arglist) {
    if (!arglist || !arglist[0]) return 0;

    if (strcmp(arglist[0], "exit") == 0) {
        if (arglist[1]) {
            char *endptr = NULL;
            long code = strtol(arglist[1], &endptr, 10);
            if (endptr != arglist[1] && *endptr == '\0') exit((int)code);
            fprintf(stderr, "exit: numeric argument required\n");
            exit(EXIT_FAILURE);
        }
        exit(0);
    }

    if (strcmp(arglist[0], "cd") == 0) {
        char *dir = arglist[1] ? arglist[1] : getenv("HOME");
        if (!dir) dir = "/";
        if (chdir(dir) != 0) fprintf(stderr, "cd: %s: %s\n", dir, strerror(errno));
        return 1;
    }

    if (strcmp(arglist[0], "help") == 0) {
        printf("Shell built-in commands:\n");
        printf("  exit [n]       Exit the shell (optional status n)\n");
        printf("  cd [dir]       Change current directory\n");
        printf("  help           Display this help message\n");
        printf("  history        Show command history\n");
        printf("  jobs           Job control not yet implemented\n");
        return 1;
    }

    if (strcmp(arglist[0], "jobs") == 0) {
        printf("Job control not yet implemented.\n");
        return 1;
    }

    if (strcmp(arglist[0], "history") == 0) {
        return shell_history(arglist);
    }

    return 0;
}

/* ---------- History Built-in ---------- */
int shell_history(char **args) {
    (void)args;
    for (int i = 0; i < history_count; i++)
        printf("%d %s\n", i + 1, history[i]);
    return 1;
}

/* ---------- Handle !n ---------- */
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
