#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

char *history[HISTORY_SIZE];
int history_count = 0;

/* ---------- Read Command ---------- */
char* read_cmd(char* prompt, FILE* fp) {
    printf("%s", prompt);
    char* cmdline = (char*) malloc(sizeof(char) * MAX_LEN);
    int c, pos = 0;

    while ((c = getc(fp)) != EOF) {
        if (c == '\n') break;
        cmdline[pos++] = c;
    }

    if (c == EOF && pos == 0) {
        free(cmdline);
        return NULL; // Handle Ctrl+D
    }
    
    cmdline[pos] = '\0';

    /* Store command in history if not empty */
    if (strlen(cmdline) > 0 && strcmp(cmdline, "history") != 0 && cmdline[0] != '!') {
        add_to_history(cmdline);
    }

    return cmdline;
}

/* ---------- Tokenizer ---------- */
char** tokenize(char* cmdline) {
    if (cmdline == NULL || cmdline[0] == '\0' || cmdline[0] == '\n') {
        return NULL;
    }

    char** arglist = (char**)malloc(sizeof(char*) * (MAXARGS + 1));
    for (int i = 0; i < MAXARGS + 1; i++) {
        arglist[i] = (char*)malloc(sizeof(char) * ARGLEN);
        bzero(arglist[i], ARGLEN);
    }

    char* cp = cmdline;
    char* start;
    int len;
    int argnum = 0;

    while (*cp != '\0' && argnum < MAXARGS) {
        while (*cp == ' ' || *cp == '\t') cp++; 
        if (*cp == '\0') break;

        start = cp;
        len = 1;
        while (*++cp != '\0' && !(*cp == ' ' || *cp == '\t')) {
            len++;
        }
        strncpy(arglist[argnum], start, len);
        arglist[argnum][len] = '\0';
        argnum++;
    }

    if (argnum == 0) {
        for(int i = 0; i < MAXARGS + 1; i++) free(arglist[i]);
        free(arglist);
        return NULL;
    }

    arglist[argnum] = NULL;
    return arglist;
}

/* ---------- Add Command to History ---------- */
void add_to_history(const char *cmd) {
    if (history_count == HISTORY_SIZE) {
        free(history[0]);
        for (int i = 1; i < HISTORY_SIZE; i++) {
            history[i - 1] = history[i];
        }
        history_count--;
    }

    history[history_count] = strdup(cmd);
    history_count++;
}

/* ---------- Built-in Commands ---------- */
int handle_builtin(char **arglist)
{
    if (arglist == NULL || arglist[0] == NULL)
        return 0; /* no command */

    /* ----- exit ----- */
    if (strcmp(arglist[0], "exit") == 0) {
        if (arglist[1] != NULL) {
            char *endptr = NULL;
            long code = strtol(arglist[1], &endptr, 10);
            if (endptr != arglist[1] && *endptr == '\0') {
                exit((int)code);
            } else {
                fprintf(stderr, "exit: numeric argument required\n");
                exit(EXIT_FAILURE);
            }
        } else {
            exit(0);
        }
        return 1;
    }

    /* ----- cd ----- */
    if (strcmp(arglist[0], "cd") == 0) {
        char *dir = arglist[1];
        if (dir == NULL) {
            dir = getenv("HOME");
            if (dir == NULL) dir = "/";
        }
        if (chdir(dir) != 0) {
            fprintf(stderr, "cd: %s: %s\n", dir, strerror(errno));
        }
        return 1;
    }

    /* ----- help ----- */
    if (strcmp(arglist[0], "help") == 0) {
        printf("Shell built-in commands:\n");
        printf("  exit [n]       Exit the shell (optional status n)\n");
        printf("  cd [dir]       Change the current directory\n");
        printf("  help           Display this help message\n");
        printf("  history        Show command history\n");
        printf("  jobs           Job control not yet implemented\n");
        return 1;
    }

    /* ----- jobs ----- */
    if (strcmp(arglist[0], "jobs") == 0) {
        printf("Job control not yet implemented.\n");
        return 1;
    }

    /* ----- history ----- */
    if (strcmp(arglist[0], "history") == 0) {
        return shell_history(arglist);
    }

    return 0; /* not a builtin */
}

/* ---------- history Built-in Implementation ---------- */
int shell_history(char **args) {
    for (int i = 0; i < history_count; i++) {
        printf("%d %s\n", i + 1, history[i]);
    }
    return 1;
}
