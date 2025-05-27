#include "ksh.h"

char* input_string(FILE* fp, size_t size) {
    int buf_size = BUF_SIZE;
    int pos = 0;
    int ch;

    char* buffer = malloc(sizeof(char) * buf_size);

    if (!buffer) {
        fprintf(stderr, "input_string: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        ch = getchar();

        if (ch == EOF || ch == '\n') {
            buffer[pos] = '\0';
            break;
        } else {
            buffer[pos] = ch;
            pos++;
        }

        if (pos >= buf_size) {
            buf_size += BUF_SIZE;
            buffer = realloc(buffer, buf_size);
            if (!buffer) {
                fprintf(stderr, "input_string: allocation error\n");
                exit(EXIT_FAILURE);                
            }
        }
    }

    return buffer;
}

char** split_args(char* input) {
    int buf_size = TOKEN_BUF_SIZE;
    int pos = 0;
    char** args = malloc(buf_size * sizeof(char*));
    char* token;

    if (!args) {
        fprintf(stderr, "split_args: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(input, TOKEN_DELIM);
    while (token != NULL) {
        args[pos] = token;
        pos++;

        if (pos >= buf_size) {
            buf_size += TOKEN_BUF_SIZE;
            args = realloc(args, buf_size * sizeof(char*));
            if (!args) {
                fprintf(stderr, "split_args: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, TOKEN_DELIM);
    }

    args[pos] = NULL;
    return args;
}

char*** parse_pipe(int* num_cmds, char* line) {
    int cur_num = NUM_CMDS;
    char** commands = malloc(cur_num * sizeof(char*));
    if (!commands) {
        perror("allocation error");
        exit(EXIT_FAILURE);
    }

    int count = 0;

    char* token = strtok(line, "|");

    while (token != NULL) {
        while (*token == ' ') {
            token++;
        }

        commands[count++] = token;

        if (count >= cur_num) {
            cur_num += NUM_CMDS;
            commands = realloc(commands, cur_num * sizeof(char*));
            if (!commands) {
                perror("allocation error");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, "|");
    }

    *num_cmds = count;
    char*** split_commands = malloc(cur_num * sizeof(char**));

    for (int i = 0; i < count; i++) {
        split_commands[i] = split_args(commands[i]);
    }
    free(commands);

    return split_commands;
}

void free_commands(char*** commands, int num_cmds) {
    if (!commands) {
        return;
    }
    for (int i = 0; i < num_cmds; i++) {
        if (commands[1]) {
            free(commands[i]);
        }
    }
    free(commands);
}