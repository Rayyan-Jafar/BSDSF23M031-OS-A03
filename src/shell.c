#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>


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
    return cmdline;
}

char** tokenize(char* cmdline) {
    // Edge case: empty command line
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
        while (*cp == ' ' || *cp == '\t') cp++; // Skip leading whitespace
        
        if (*cp == '\0') break; // Line was only whitespace

        start = cp;
        len = 1;
        while (*++cp != '\0' && !(*cp == ' ' || *cp == '\t')) {
            len++;
        }
        strncpy(arglist[argnum], start, len);
        arglist[argnum][len] = '\0';
        argnum++;
    }

    if (argnum == 0) { // No arguments were parsed
        for(int i = 0; i < MAXARGS + 1; i++) free(arglist[i]);
        free(arglist);
        return NULL;
    }

    arglist[argnum] = NULL;
    return arglist;
}


int handle_builtin(char **arglist)
{
    if (arglist == NULL || arglist[0] == NULL)
        return 0; /* no command */

    /* ----- exit ----- */
    if (strcmp(arglist[0], "exit") == 0) {
        /* Optional: you can do cleanup here (free history, jobs, etc.) */
        /* If user provided a status: exit <n> */
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
        /* unreachable */
        return 1;
    }

    /* ----- cd ----- */
    if (strcmp(arglist[0], "cd") == 0) {
        char *dir = arglist[1];
        if (dir == NULL) {
            /* default to HOME if no arg */
            dir = getenv("HOME");
            if (dir == NULL) dir = "/"; /* fallback */
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
        printf("  cd [dir]       Change the current directory to 'dir'\n");
        printf("  help           Display this help message\n");
        printf("  jobs           Job control not yet implemented\n");
        return 1;
    }

    /* ----- jobs ----- */
    if (strcmp(arglist[0], "jobs") == 0) {
        printf("Job control not yet implemented.\n");
        return 1;
    }

    return 0; /* not a builtin */
}
