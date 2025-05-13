#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

int exec(char **cmds, char **env, int i, int *pipes, int cmd_index, int num_cmds) {
    int pid;

    pid = fork();
    if (!pid) {
        if (cmd_index > 0) 
            dup2(pipes[(cmd_index -1) * 2], 0);
        if (cmd_index < num_cmds -1)
            dup2(pipes[cmd_index * 2 + 1], 1);
        for (int j = 0; j < 2 * (num_cmds -1 ); j++)
            close(pipes[j]);
        cmds[i] = 0;
        execve(cmds[0], cmds, env);
    }
    return (pid);
}

void pipeline(char **cmds, char **env, int i, int num_cmds) {
    int *pipes = NULL;
    int *pids = malloc(sizeof(int *) * num_cmds);

    if (num_cmds > 1) {
        pipes = malloc(sizeof(int *) * 2 * (num_cmds - 1));
        for (int j = 0; j < num_cmds - 1; j++)
            pipe(pipes + (j * 2));
    }

    int cmd_index = 0;
    int cmds_count = 0;
    while (cmd_index < num_cmds) {
        int j = 0;
        while (cmds_count + j < i && strcmp(cmds[j], "|"))
            j++;
        cmds[j] = 0;
        pids[cmd_index] = exec(cmds, env, i, pipes,  cmd_index, num_cmds);
        cmds += j + 1;
        cmds_count += j + 1;
        cmd_index++;
    }

    for (int j = 0; j < 2 * (num_cmds - 1); j++)
        close(pipes[j]);
    for (int j = 0; j < num_cmds; j++)
        waitpid(pids[j], NULL, 0);
}

void printer(char **mat) {
    int i = 0;
    while(mat[i])
        printf("%s =>", mat[i++]);
}

char **parser(char *input, int *token_count) {
    int i = 0;
    char **tokens = malloc(sizeof(char *) * 10);
    char *token = strtok(input, " \t\n");
    while (token) {
        tokens[i++] = strdup(token);
        token = strtok(NULL, " \t\n");
    }
    tokens[i] = NULL;
    *token_count = i;
    return (tokens);
}

int main(int ac, char **av, char **env)  {
    char *input;
    char **tokens;
    int i = 0;
    int token_count;

    while(1) {
        input = readline("> ");
        if (!input)
            break ;
        if (*input) {
            add_history(input);
        }
        tokens = parser(input, &token_count);
        i = 0;
        while (i < token_count) {
            int start = i;
            int cmds_count = 1;

            while (i < token_count) {
                if (!strcmp(tokens[i], "|"))
                    cmds_count++;
                i++;
            }
            if (i > start) {
                pipeline(tokens, env, i - start, cmds_count);
            }
        }
    }
    return (0);
}