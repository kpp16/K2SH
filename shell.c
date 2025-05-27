#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <linux/limits.h>

char* builtin_str[] = {
    "ksh_cd",
    "ksh_help",
    "ksh_exit"
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

    fprintf(stdout, "Use man <command> to learn more about the command\n");
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
        args = split_args(input);

        status = execute(args);

        free(input);
        free(args);

    } while (status);
}

int main() {

    mainloop();

    return 0;
}
