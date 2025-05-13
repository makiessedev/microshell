#include "./header.h"

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
                return NULL;
            tokens = new_tokens;
        }
        tokens[count] = strdup(token);
        if (!tokens[count])
            return NULL;
        count++;
        token = strtok(NULL, " \t\n");
    }
    tokens[count] = NULL;
    *argc = count;
    return tokens;
}