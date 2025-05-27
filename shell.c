#include "ksh.h"

void shell_init(shell_state_t* state) {
    if (getcwd(state->cwd, sizeof(state->cwd)) == NULL) {
        perror("ksh: getcwd() error");
        strcpy(state->cwd, "/");
    }

    state->last_exit_status = 0;
    state->should_exit = 0;

    printf("KSH - Simple Shell v1.0\n");
    printf("Type 'help' for available commands.\n");    
}


void shell_mainloop(shell_state_t* state) {
    char* input;
    char** args;
    char prompt[MAX_PROMPT_SIZE];
    int status = 1;

    while (status && !state->should_exit) {
        update_prompt(state, prompt, sizeof(prompt));
        printf("%s", prompt);
        fflush(stdout);
        
        input = input_string(stdin, 64);
        
        if (strlen(input) == 0) {
            free(input);
            continue;
        }

        append_history(input);

        if (strchr(input, '|') != NULL) {
            int num_cmds;
            char*** commands = parse_pipe(&num_cmds, input);
            
            if (commands && num_cmds > 0) {
                status = execute_pipeline(num_cmds, commands, state);
                free_commands(commands, num_cmds);
            }
        } else {
            args = split_args(input);
            if (args && args[0]) {
                status = execute_command(args, state);
            }
            free(args);
        }

        free(input);
    }
}

void shell_cleanup(shell_state_t* state) {
    free_history();
    printf("Exiting...\n");
}

void update_prompt(shell_state_t* state, char* prompt, size_t size) {
    if (getcwd(state->cwd, sizeof(state->cwd)) == NULL) {
        perror("ksh: getcwd() error");
    }
    snprintf(prompt, size, "\033[1;32m%s\033[0m \033[1;34m>:\033[0m ", state->cwd);
}

void print_error(const char* message) {
    fprintf(stderr, "\033[1;31mError:\033[0m %s\n", message);
}