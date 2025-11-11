#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char *cmdline;
    char **arglist;

    init_readline();

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {

        if (cmdline[0] == '\0') {
            free(cmdline);
            continue;
        }

        // --- Handle !n commands ---
        if (cmdline[0] == '!') {
            if (handle_bang_command(&cmdline)) {
                free(cmdline);
                continue;
            }
        }

        // --- Add to custom history exactly once ---
        add_to_history(cmdline);

        // --- Tokenize and execute ---
        if ((arglist = tokenize(cmdline)) != NULL) {
            if (!handle_builtin(arglist)) {
                execute(arglist);
            }

            for (int i = 0; arglist[i] != NULL; i++)
                free(arglist[i]);
            free(arglist);
        }

        free(cmdline);
    }

    printf("\nShell exited.\n");

    custom_history_cleanup();
    return 0;
}
