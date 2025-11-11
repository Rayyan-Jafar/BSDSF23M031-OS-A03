# ----------------------------------------
# Makefile for OS Assignment 03 - Shell with Readline
# ----------------------------------------

# Compiler and Flags
CC      = gcc
CFLAGS  = -Wall -Wextra -Iinclude
LDFLAGS = -lreadline   # Link Readline library

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Target
TARGET  = $(BIN_DIR)/myshell

# Source and Object Files
SRC_FILES = $(SRC_DIR)/main.c $(SRC_DIR)/shell.c $(SRC_DIR)/execute.c $(SRC_DIR)/ifthen.c
OBJ_FILES = $(OBJ_DIR)/main.o $(OBJ_DIR)/shell.o $(OBJ_DIR)/execute.o $(OBJ_DIR)/ifthen.o

# Default Target
all: $(TARGET)

# Build the final executable
$(TARGET): $(OBJ_FILES)
	@mkdir -p $(BIN_DIR)
	$(CC) $(OBJ_FILES) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete! Run using ./bin/myshell"

# Compile each .c file into .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean object and binary files
clean:
	@rm -rf $(OBJ_DIR)/*.o $(TARGET)
	@echo "Cleaned object and binary files."

# Rebuild project from scratch
rebuild: clean all

# Phony Targets
.PHONY: all clean rebuild
