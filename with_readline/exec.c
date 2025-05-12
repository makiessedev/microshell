#include "./header.h"

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