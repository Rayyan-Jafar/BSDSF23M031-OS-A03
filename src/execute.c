#include "shell.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>

/* ---------- Helper function to split commands for a single pipe ---------- */
static int split_pipe(char **arglist, char **cmd1, char **cmd2) {
    int i = 0, j = 0;
    int pipe_found = 0;

    while (arglist[i] != NULL) {
        if (strcmp(arglist[i], "|") == 0) {
            pipe_found = 1;
            i++;
            break;
        }
        cmd1[j++] = arglist[i++];
    }
    cmd1[j] = NULL;

    if (!pipe_found) return 0; // no pipe

    j = 0;
    while (arglist[i] != NULL) {
        cmd2[j++] = arglist[i++];
    }
    cmd2[j] = NULL;
    return 1;
}

/* ---------- Helper function to handle redirection ---------- */
static void handle_redirection(char **argv) {
    char *input_file = NULL;
    char *output_file = NULL;
    char *args[MAXARGS + 1];
    int argc = 0;

    for (int i = 0; argv[i] != NULL; i++) {
        if (strcmp(argv[i], "<") == 0) {
            input_file = argv[++i];
        } else if (strcmp(argv[i], ">") == 0) {
            output_file = argv[++i];
        } else {
            args[argc++] = argv[i];
        }
    }
    args[argc] = NULL;

    // Input redirection
    if (input_file) {
        int fd = open(input_file, O_RDONLY);
        if (fd < 0) { perror("open"); exit(1); }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    // Output redirection
    if (output_file) {
        int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) { perror("open"); exit(1); }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

    execvp(args[0], args);
    perror("Command not found");
    exit(1);
}

int execute(char* arglist[]) {
    int status;

    // --- Check for a single pipe ---
    char *cmd1[MAXARGS + 1], *cmd2[MAXARGS + 1];
    if (split_pipe(arglist, cmd1, cmd2)) {
        int pipefd[2];
        if (pipe(pipefd) < 0) { perror("pipe"); return -1; }

        pid_t pid1 = fork();
        if (pid1 < 0) { perror("fork"); return -1; }

        if (pid1 == 0) { // left child (writer)
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[0]);
            close(pipefd[1]);
            handle_redirection(cmd1); // handle < or > in left cmd
        }

        pid_t pid2 = fork();
        if (pid2 < 0) { perror("fork"); return -1; }

        if (pid2 == 0) { // right child (reader)
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            close(pipefd[1]);
            handle_redirection(cmd2); // handle < or > in right cmd
        }

        // parent closes both ends and waits
        close(pipefd[0]);
        close(pipefd[1]);
        waitpid(pid1, &status, 0);
        waitpid(pid2, &status, 0);
        return 0;
    }

    // --- No pipe: execute single command with possible redirection ---
    pid_t cpid = fork();
    if (cpid < 0) { perror("fork failed"); exit(1); }

    if (cpid == 0) {
        handle_redirection(arglist);
    } else {
        waitpid(cpid, &status, 0);
    }

    return 0;
}
