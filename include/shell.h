#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30
#define HISTORY_SIZE 20
#define PROMPT "FCIT> "

/* --- Global history variables --- */
extern char *history[HISTORY_SIZE];
extern int history_count;

/* --- Input function --- */
char* read_cmd(char* prompt, FILE* fp);

/* --- Tokenization & execution --- */
char** tokenize(char* cmdline);
int execute(char** arglist);
int handle_builtin(char **arglist);

/* --- History helpers --- */
void add_to_history(const char *cmd);
int shell_history(char **args);
const char *custom_history_get(int n);
int custom_history_count(void);
void custom_history_cleanup(void);

/* --- !n command support --- */
int handle_bang_command(char **cmdline);

/* --- Readline / tab completion --- */
void init_readline(void);

#endif /* SHELL_H */
