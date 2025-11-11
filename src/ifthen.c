#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

/* Trim leading/trailing spaces */
static void trim(char *s) {
    char *end;
    while(*s == ' ' || *s == '\t') s++;
    end = s + strlen(s) - 1;
    while(end > s && (*end == ' ' || *end == '\t' || *end == '\n')) *end-- = '\0';
}

/* Execute a single line and return exit status */
int execute_line_get_status(const char *line) {
    char **tok = tokenize(strdup(line));
    if (!tok) return -1;

    pid_t pid = fork();
    if (pid == 0) {
        execute(tok, 0);
        for (int i=0; tok[i]!=NULL; i++) free(tok[i]);
        free(tok);
        exit(0);
    } else {
        int status;
        waitpid(pid, &status, 0);
        for (int i=0; tok[i]!=NULL; i++) free(tok[i]);
        free(tok);
        return WEXITSTATUS(status);
    }
}

/* Handle multi-line if ... then ... [else ...] fi */
void handle_if_block(char *first_line) {
    char then_block[10][MAX_LEN];
    char else_block[10][MAX_LEN];
    int then_count = 0, else_count = 0;
    int in_else = 0;

    char *line = strdup(first_line);
    trim(line);

    char if_cmd[MAX_LEN];
    if (strncmp(line, "if ", 3) == 0) strcpy(if_cmd, line + 3);
    else strcpy(if_cmd, "");

    while (1) {
        char *next_line = readline("> ");
        if (!next_line) break;
        trim(next_line);

        if (strcmp(next_line, "then") == 0) { free(next_line); continue; }
        else if (strcmp(next_line, "else") == 0) { in_else = 1; free(next_line); continue; }
        else if (strcmp(next_line, "fi") == 0) { free(next_line); break; }

        if (!in_else) strcpy(then_block[then_count++], next_line);
        else strcpy(else_block[else_count++], next_line);

        free(next_line);
    }

    free(line);

    int status = execute_line_get_status(if_cmd);

    if (status == 0) {
        for (int i = 0; i < then_count; i++) execute_line_get_status(then_block[i]);
    } else {
        for (int i = 0; i < else_count; i++) execute_line_get_status(else_block[i]);
    }
}
