#include "ksh.h"

int execute_command(char** args, shell_state_t* state) {
    if (args[0] == NULL) {
        return 1;
    }

    int builtin_index = find_builtin(args[0]);
    if (builtin_index >= 0) {
        builtin_command_t* commands = get_builtins();
        return commands[builtin_index].function(args, state);
    }

    return launch_command(args, state);
}

int launch_command(char **args, shell_state_t* state) {
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

        if (WIFEXITED(status)) {
            state->last_exit_status = WIFEXITED(status);
        }
    }

    return 1;
}

int execute_pipeline(int num_cmds, char ***commands, shell_state_t* state) {
    int pipes[num_cmds - 1][2];
    pid_t pids[num_cmds];

    for (int i = 0; i < num_cmds - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_cmds; i++) {
        pids[i] = fork();
        pid_t pid = pids[i];

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
    }
    for (int i = 0; i < num_cmds - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }    

    int last_status = 0;
    for (int i = 0; i < num_cmds; i++) {
        int status;
        waitpid(pids[i], &status, 0);

        if (i == num_cmds - 1 && WIFEXITED(status)) {
            last_status = WIFEXITED(status);
        }
    }

    state->last_exit_status = last_status;
    return 1;
}