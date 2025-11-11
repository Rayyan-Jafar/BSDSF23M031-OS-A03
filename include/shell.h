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

/* --- Background job tracking --- */
#define MAX_BG_JOBS 50
typedef struct {
    pid_t pid;
    char cmd[MAX_LEN];
} bg_job;

extern bg_job bg_jobs[MAX_BG_JOBS];
extern int bg_count;

/* --- Shell Variables --- */
typedef struct variable {
    char *name;
    char *value;
    struct variable *next;
} variable;

extern variable *var_list;

void set_variable(const char *name, const char *value);
char* get_variable(const char *name);
void expand_variables(char **arglist);
void free_variables(void);
int handle_variable_assignment(char **arglist);
void cleanup_variables(void);

/* --- Background job management --- */
void add_bg_job(pid_t pid, const char *cmd);
void remove_bg_job(pid_t pid);
void list_bg_jobs(void);
void reap_background_jobs(void);

/* --- Input function --- */
char* read_cmd(char* prompt, FILE* fp);

/* --- Tokenization & execution --- */
char** tokenize(char* cmdline);
int execute(char** arglist, int background);
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

/* --- Execute single command & get status --- */
int execute_line_get_status(const char *line);

/* --- Handle multi-line if ... then ... [else ...] fi --- */
void handle_if_block(char *first_line);

#endif /* SHELL_H */
