#ifndef KSH_H
#define KSH_H

#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <linux/limits.h>

#define BUF_SIZE 256
#define TOKEN_BUF_SIZE 64
#define TOKEN_DELIM " \t\r\n\a"
#define NUM_CMDS 2
#define MAX_PROMPT_SIZE 512

typedef struct {
    char cwd[PATH_MAX];
    int last_exit_status;
    int should_exit;
} shell_state_t;

typedef struct {
    char* name;
    int (*function)(char** args, shell_state_t* state);
    char* description;
} builtin_command_t;

builtin_command_t* get_builtins(void);
int get_builtin_count(void);
int find_builtin(char* command_name);

int builtin_cd(char** args, shell_state_t* state);
int builtin_help(char** args, shell_state_t* state);
int builtin_exit(char** args, shell_state_t* state);
int builtin_pwd(char** args, shell_state_t* state);
int builtin_echo(char** args, shell_state_t* state);
int builtin_history(char** args, shell_state_t* state);
void append_history(char* command);
void free_history();

void shell_init(shell_state_t* state);
void shell_mainloop(shell_state_t* state);
void shell_cleanup(shell_state_t* state);
void update_prompt(shell_state_t* state, char* prompt, size_t size);
void print_error(const char* message);

int execute_pipeline(int num_cmds, char*** commands, shell_state_t* state);
int execute_command(char** args, shell_state_t* state);
int launch_command(char** args, shell_state_t* state);

char* input_string(FILE* fp, size_t size);
char** split_args(char* input);
char*** parse_pipe(int* num_cmds, char* line);
void free_commands(char*** commands, int num_cmds);

#endif