#include "./header.h"

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
