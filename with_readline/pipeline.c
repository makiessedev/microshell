#include "./header.h"

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