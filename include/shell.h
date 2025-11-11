#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30
#define HISTORY_SIZE 20   /* store at least last 20 commands */
#define PROMPT "FCIT> "

// Declare global history variables
extern char *history[HISTORY_SIZE];
extern int history_count;

// Function prototypes
char* read_cmd(char* prompt, FILE* fp);
char** tokenize(char* cmdline);
int execute(char** arglist);

int handle_builtin(char **arglist); /* returns 1 if builtin handled, 0 otherwise */

/* history-related helpers */
void add_to_history(const char *cmd);
int shell_history(char **args);

/* !n command support */
int handle_bang_command(char **cmdline);

#endif /* SHELL_H */
