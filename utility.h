#ifndef UTILITY_H
#define UTILITY_H
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <signal.h>
#define NUM_STR 512
enum output_redir_type{None, Append, Overwrite};
struct Inputs {
    char **command;
    enum output_redir_type output_redir;
    bool input_redir;
    char *ifile;
    char *ofile;
};
void parse_args(char* input, struct Inputs* inputs);
unsigned long splitPipeCommand(char *input, char **pipeInput);
int ownCmdChild(char** cmd);
void run_command(struct Inputs inputs);
int ownCmdParent(char** cmd);
void pipe_exec(char *input);
#endif
