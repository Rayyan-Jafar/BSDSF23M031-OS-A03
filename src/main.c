#include "shell.h"

int handle_bang_command(char **cmdline) {
    if ((*cmdline)[0] != '!')
        return 0; // not a !n command

    int n = atoi((*cmdline) + 1);
    if (n <= 0 || n > history_count) {
        printf("No such command in history: %d\n", n);
        return 1; // handled (invalid or out of range)
    }

    // Replace cmdline with the n-th history command
    free(*cmdline);
    *cmdline = strdup(history[n - 1]);
    printf("%s\n", *cmdline); // Echo the command being executed
    return 0;
}

int main() {
    char *cmdline;
    char **arglist;

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {

        // --- Handle !n command before tokenization ---
        // --- Handle !n command before tokenization ---
        if (cmdline[0] == '!') {
            if (handle_bang_command(&cmdline)) {
                free(cmdline);
                continue; // skip if invalid !n
            } else {
                /* Successfully resolved !n -> cmdline now contains the resolved command.
                Add the resolved command to history so re-executed command appears there. */
                if (cmdline != NULL && cmdline[0] != '\0') {
                    add_to_history(cmdline);
                }
            }
        }


        // --- Tokenize and execute ---
        if ((arglist = tokenize(cmdline)) != NULL) {

            if (!handle_builtin(arglist)) {
                execute(arglist);  // External command
            }

            // Free tokens
            for (int i = 0; arglist[i] != NULL; i++) {
                free(arglist[i]);
            }
            free(arglist);
        }

        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}
