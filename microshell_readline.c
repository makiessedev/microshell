#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

int err(char *str)
{
    while (*str)
        write(2, str++, 1);
    return 1;
}

int cd(char **argv, int i)
{
    if (i != 2)
        return err("error: cd: bad arguments\n");
    else if (chdir(argv[1]) == -1)
        return err("error: cd: cannot change directory to "), err(argv[1]), err("\n");
    return 0;
}

int exec(char **argv, char **envp, int i, int *pipes, int cmd_index, int num_cmds)
{
    int pid;

    pid = fork();
    if (pid == -1)
        return err("error: fatal\n");

    if (!pid)
    {
        if (cmd_index > 0)
        {
            if (dup2(pipes[(cmd_index - 1) * 2], 0) == -1)
                exit(err("error: fatal\n"));
        }
        if (cmd_index < num_cmds - 1)
        {
            if (dup2(pipes[cmd_index * 2 + 1], 1) == -1)
                exit(err("error: fatal\n"));
        }
        for (int j = 0; j < 2 * (num_cmds - 1); j++)
            close(pipes[j]);

        argv[i] = 0;
        execve(argv[0], argv, envp);
        err("error: cannot execute ");
        err(argv[0]);
        err("\n");
        exit(1);
    }

    return pid;
}

int run_pipeline(char **argv, char **envp, int i, int num_cmds)
{
    int *pipes = NULL;
    int *pids = malloc(sizeof(int) * num_cmds);
    int status;

    if (!pids)
        return err("error: fatal\n");

    if (num_cmds > 1)
    {
        pipes = malloc(sizeof(int) * 2 * (num_cmds - 1));
        if (!pipes)
        {
            free(pids);
            return err("error: fatal\n");
        }
        for (int j = 0; j < num_cmds - 1; j++)
        {
            if (pipe(pipes + j * 2) == -1)
            {
                free(pipes);
                free(pids);
                return err("error: fatal\n");
            }
        }
    }

    int cmd_index = 0;
    int argc = 0;
    while (cmd_index < num_cmds)
    {
        int j = 0;
        while (argc + j < i && strcmp(argv[j], "|"))
            j++;
        argv[j] = 0;
        pids[cmd_index] = exec(argv, envp, j, pipes, cmd_index, num_cmds);
        argv += j + 1;
        argc += j + 1;
        cmd_index++;
    }

    for (int j = 0; j < 2 * (num_cmds - 1); j++)
        close(pipes[j]);

    for (int j = 0; j < num_cmds; j++)
        waitpid(pids[j], &status, 0);

    free(pipes);
    free(pids);
    return WIFEXITED(status) && WEXITSTATUS(status);
}

char **parse_input(char *input, int *argc)
{
    char **tokens = NULL;
    int count = 0;
    int capacity = 10;
    char *token;

    tokens = malloc(sizeof(char *) * capacity);
    if (!tokens)
        return NULL;

    token = strtok(input, " \t\n");
    while (token)
    {
        if (count >= capacity)
        {
            capacity *= 2;
            char **new_tokens = realloc(tokens, sizeof(char *) * capacity);
            if (!new_tokens)
            {
                free(tokens);
                return NULL;
            }
            tokens = new_tokens;
        }
        tokens[count] = strdup(token);
        if (!tokens[count])
        {
            for (int j = 0; j < count; j++)
                free(tokens[j]);
            free(tokens);
            return NULL;
        }
        count++;
        token = strtok(NULL, " \t\n");
    }
    tokens[count] = NULL;
    *argc = count;
    return tokens;
}

void free_tokens(char **tokens, int argc)
{
    for (int i = 0; i < argc; i++)
        free(tokens[i]);
    free(tokens);
}

int main(int argc, char **argv, char **envp)
{
    char *input;
    int status = 0;

    while (1)
    {
        input = readline("microshell> ");
        if (!input)
            break;
        if (*input)
            add_history(input);

        int token_count;
        char **tokens = parse_input(input, &token_count);
        free(input);
        if (!tokens)
        {
            err("error: fatal\n");
            continue;
        }


        int i = 0;
        while (i < token_count)
        {
            int start = i;
            int num_cmds = 1;

            while (i < token_count && strcmp(tokens[i], ";"))
            {
                if (!strcmp(tokens[i], "|"))
                    num_cmds++;
                i++;
            }
            if (i > start)
            {
                if (!strcmp(tokens[start], "cd"))
                    status = cd(tokens + start, i - start);
                else
                    status = run_pipeline(tokens + start, envp, i - start, num_cmds);
            }

            if (i < token_count && !strcmp(tokens[i], ";"))
                i++;
        }

        free_tokens(tokens, token_count);
    }

    return status;
}
