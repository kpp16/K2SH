#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <linux/limits.h>

char* builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int num_builtins() {
    return sizeof(builtin_str) / sizeof(char*);
}

int ksh_cd(char** args) {
    if (args[1] == NULL) {
        fprintf(stderr, "No path specified\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("ksh");
        }
    }

    return 1;
}

int ksh_help(char** args) {
    for (int i = 0; i < num_builtins(); i++) {
        fprintf(stdout, "%s\n", builtin_str[i]);
    }

    fprintf(stdout, "Use man <command> to learn more about other commands\n");
    return 1;
}

int ksh_exit(char **args) {
    return 0;
}

int (*builtin_fs[]) (char**) = {
    &ksh_cd,
    &ksh_help,
    &ksh_exit
};

#define BUF_SIZE 256
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

#define TOKEN_BUF_SIZE 64
#define TOKEN_DELIM " \t\r\n\a"
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

int launch(char** args) {
	pid_t pid = fork();
    int status;
    pid_t wpid;

	if (pid == -1) {
        perror("ksh");
	} else if (pid == 0) {
		// child process
        if (execvp(args[0], args) == -1) {
            perror("ksh");
        }
        exit(EXIT_FAILURE);
	} else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));        
    }

    return 1;
}

int execute(char **args) {
    if (args[0] == NULL) {
        return 1;
    }

    for (int i = 0; i < num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_fs[i])(args);
        }
    }

    return launch(args);
}


#define NUM_CMDS 2
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

int execute_pipeline(int num_cmds, char*** commands) {
    int pipes[num_cmds - 1][2];

    for (int i = 0; i < num_cmds - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_cmds; i++) {
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            if (i > 0) {
                // take input from the prev pipe
                if (dup2(pipes[i - 1][0], STDIN_FILENO) == -1) {
                    perror("dup2 stdin");
                    exit(EXIT_FAILURE);
                }
            }

            if (i < num_cmds - 1) {
                // redirect the write end of the current pipe to stdout
                if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
                    perror("dup2 stdin");
                    exit(EXIT_FAILURE);                    
                }
            }

            for (int j = 0; j < num_cmds - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // printf("%s\n", commands[i][0]);
            if (execvp(commands[i][0], commands[i]) == -1) {
                perror("ksh: ");
            }
        }
        //  else {
        //     if (i > 0) {
        //         close(pipes[i - 1][0]);
        //     }
        //     if (i < num_cmds - 1) {
        //         close(pipes[i][1]);
        //     }
        // }
    }
    for (int i = 0; i < num_cmds - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }    

    for (int i = 0; i < num_cmds; i++) {
        wait(NULL);
    }

    return 1;
}

void mainloop() {
    char* input;
    char** args;
    int status;

    do {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("ksh: getcwd() error");
        }

        fprintf(stdout, "%s >: ", cwd);
        input = input_string(stdin, 64);

        if (strchr(input, '|') != NULL) {
            int num_cmds;
            char*** commands = parse_pipe(&num_cmds, input);

            status = execute_pipeline(num_cmds, commands);

            for (int i = 0; i < num_cmds; i++) {
                free(commands[i]);
            }
            free(commands);

        } else {
            args = split_args(input);
            status = execute(args);
            free(args);
        }

        free(input);

    } while (status);
}

int main() {

    mainloop();

    return 0;
}
