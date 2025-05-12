#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

int err(char *str);
int cd(char **argv, int i);
void free_tokens(char **tokens, int argc);
char **parse_input(char *input, int *argc);
int run_pipeline(char **argv, char **envp, int i, int num_cmds);
int exec(char **argv, char **envp, int i, int *pipes, int cmd_index, int num_cmds);