#include "ksh.h"

static builtin_command_t builtins[] = {
    {"cd", builtin_cd, "Change directory"},
    {"help", builtin_help, "Display help information"},
    {"exit", builtin_exit, "Exit the shell"},
    {"pwd", builtin_pwd, "Print working directory"},
    {"echo", builtin_echo, "Display text"}
};

builtin_command_t* get_builtins(void) {
    return builtins;
}

int get_builtin_count(void) {
    return sizeof(builtins) / sizeof(builtin_command_t);
}

int find_builtin(char* command_name) {
    for (int i = 0; i < get_builtin_count(); i++) {
        if (strcmp(command_name, builtins[i].name) == 0) {
            return i;
        }
    }
    return -1;
}

int builtin_cd(char** args, shell_state_t* state) {
    char* target_dir = args[1];

    if (target_dir == NULL) {
        target_dir = getenv("HOME");
        if (target_dir == NULL) {
            print_error("cd: HOME environment variable not set");
            return 1;
        }
    }
    
    if (chdir(target_dir) != 0) {
        perror("ksh: cd");
        return 1;
    }
    
    if (getcwd(state->cwd, sizeof(state->cwd)) == NULL) {
        perror("ksh: getcwd after cd");
    }
    
    return 1;
}

int builtin_help(char** args, shell_state_t* state) {
    printf("KSH - Simple Shell\n");
    printf("Built-in commands:\n");
    
    builtin_command_t* commands = get_builtins();
    for (int i = 0; i < get_builtin_count(); i++) {
        printf("  %-10s - %s\n", commands[i].name, commands[i].description);
    }
    
    printf("\nUse 'man <command>' to learn about other commands.\n");
    printf("Supports piping with '|' operator.\n");
    return 1;
}

int builtin_exit(char** args, shell_state_t* state) {
    int exit_code = 0;
    
    if (args[1] != NULL) {
        exit_code = atoi(args[1]);
    }
    
    state->should_exit = 1;
    state->last_exit_status = exit_code;
    return 0;
}

int builtin_pwd(char** args, shell_state_t* state) {
    printf("%s\n", state->cwd);
    return 1;
}

int builtin_echo(char** args, shell_state_t* state) {
    for (int i = 1; args[i] != NULL; i++) {
        printf("%s", args[i]);
        if (args[i + 1] != NULL) {
            printf(" ");
        }
    }
    printf("\n");
    return 1;
}