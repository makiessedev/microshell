#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

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

int main(int argc, char **argv, char **envp)
{
    int i = 0;
    int status = 0;

    if (argc > 1)
    {
        while (argv[i] && argv[++i])
        {
            argv += i;
            i = 0;
            int num_cmds = 1;
            while (argv[i] && strcmp(argv[i], ";"))
            {
                if (!strcmp(argv[i], "|"))
                    num_cmds++;
                i++;
            }
            if (i && !strcmp(argv[0], "cd"))
                status = cd(argv, i);
            else if (i)
                status = run_pipeline(argv, envp, i, num_cmds);
        }
    }
    return status;
}
