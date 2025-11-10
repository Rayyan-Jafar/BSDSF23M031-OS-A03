# BSDSF23M031-OS-A03
Creating a Shell
ROLL_NO-OS-A03/
├── src/
│   ├── main.c         # Entry point of the shell
│   ├── shell.c        # Handles input loop and parsing
│   └── execute.c      # Executes user commands (fork–exec–wait)
├── include/
│   └── shell.h        # Function declarations and constants
├── obj/
│   ├── main.o         # Object file for main.c
│   ├── shell.o        # Object file for shell.c
│   └── execute.o      # Object file for execute.c
├── bin/
│   └── myshell        # Final compiled executable
├── Makefile           # Build automation script
└── README.md          # Project documentation
