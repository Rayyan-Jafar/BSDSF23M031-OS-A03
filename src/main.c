#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    char *cmdline;
    char **arglist;

    init_readline();

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {
        if (cmdline[0] == '\0') { free(cmdline); continue; }

        reap_background_jobs();

        char *trimmed = cmdline;
        while (*trimmed == ' ' || *trimmed == '\t') trimmed++;

        if (strncmp(trimmed, "if ", 3) == 0 || strcmp(trimmed, "if") == 0) {
            handle_if_block(trimmed);
            free(cmdline);
            continue;
        }

        char *cmd_copy = strdup(cmdline);
        char *single_cmd = strtok(cmd_copy, ";");

        while (single_cmd != NULL) {
            while (*single_cmd == ' ' || *single_cmd == '\t') single_cmd++;
            if (*single_cmd != '\0') {
                char *scmd = strdup(single_cmd);

                if (scmd[0] == '!') handle_bang_command(&scmd);
                add_to_history(scmd);

                int background = 0;
                size_t len = strlen(scmd);
                if (len > 0 && scmd[len - 1] == '&') {
                    background = 1;
                    scmd[len - 1] = '\0';
                    while (len > 1 && (scmd[len - 2] == ' ' || scmd[len - 2] == '\t'))
                        scmd[--len - 1] = '\0';
                }

                if ((arglist = tokenize(scmd)) != NULL) {
                    if (!handle_variable_assignment(arglist)) {
                        expand_variables(arglist);
                        if (!handle_builtin(arglist)) {
                            pid_t pid = execute(arglist, background);
                            if (background && pid > 0) add_bg_job(pid, scmd);
                        }
                    }

                    for (int i = 0; arglist[i] != NULL; i++) free(arglist[i]);
                    free(arglist);
                }

                free(scmd);
            }

            single_cmd = strtok(NULL, ";");
        }

        free(cmd_copy);
        free(cmdline);
    }

    printf("\nShell exited.\n");
    cleanup_variables();
    custom_history_cleanup();
    return 0;
}
