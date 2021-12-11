#include "utility.h"


void parse_redir(char *input, struct Inputs* inputs){
    inputs->input_redir = false;
    inputs->output_redir = None;
    char *inredir = strchr(input, '<');
    if (inredir) {
        inputs->input_redir = true;
        inredir[0] = ' ';
        inredir++;
        while (inredir[0] == ' ') inredir++; // locate inredir to the first char of args
        size_t len = 0;
        char *temp = inredir;
        while (temp[0] != '\0') {
            if (temp[0] == ' ' || temp[0] == '>') break;
            temp++;
            len++;
        }
        inputs->ifile = malloc(sizeof(char)*len+1);
        inputs->ifile[len] = '\0';
        strncpy(inputs->ifile, inredir, len);
        memset(inredir, ' ', len);
    }
    char *outredir = strchr(input, '>');
    if (outredir) {
        outredir[0] = ' ';
        outredir++;
        inputs->output_redir = outredir[0] == '>'? Append : Overwrite;
        if (outredir[0] == '>') {outredir[0] = ' '; outredir++;}
        while (outredir[0] == ' ') outredir++; // locate outredir to the first char of args
        size_t len = 0;
        char *temp = outredir;
        while (temp[0] != '\0') {
            if (temp[0] == ' ' || temp[0] == '<') break;
            temp++;
            len++;
        }
        inputs->ofile = malloc(sizeof(char)*len+1);
        inputs->ofile[len] = '\0';
        strncpy(inputs->ofile, outredir, len);
        memset(outredir, ' ', len);
    }
}

void parse_args(char* input, struct Inputs* inputs) {
    // Parse the whole string from CLI into a list of strings, also get info for the redirection
    
    char *delimeter = " ";
    char *token;
    int index = 0;
    
    // To implement bash style redirection, redirection should be handled before splitting into tokens
    parse_redir(input, inputs); // Deal With redirection
    inputs->command = malloc(sizeof(char*) * NUM_STR);
    // Transform the input string into the string array
    token = strtok(input, delimeter);
    while (token != NULL) {
        inputs->command[index++] = token;
        token = strtok(NULL, delimeter);
    }
    inputs->command[index] = NULL;
}

void redirect(struct Inputs inputs) {
    int input_fd = -1;
    int output_fd = -1;
    if (inputs.input_redir) { // Perform input redirection
        input_fd = open(inputs.ifile, O_RDONLY);
        if(input_fd < 0) printf("Error opening the file %s\n", inputs.ifile);

        dup2(input_fd, 0);
        close(input_fd);
    }
    if (inputs.output_redir == Overwrite) { // Perform output redirection(Overwriting)
        output_fd = open(inputs.ofile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if(output_fd < 0) printf("Error opening the file %s\n", inputs.ofile);
        // printf("Redirecting to file %s\n", inputs.ofile);
        dup2(output_fd, 1);
        close(output_fd);
    }
    else if (inputs.output_redir == Append) { // Perform output redirection(Append)
        output_fd = open(inputs.ofile, O_WRONLY | O_APPEND | O_CREAT, 0644);
        if(output_fd < 0)
            printf("Error opening the file %s\n", inputs.ofile);
        dup2(output_fd, 1);
        close(output_fd);
    }
}

unsigned long splitPipeCommand(char *input, char **pipeInput) {
    unsigned long sz = 1;
    pipeInput[0] = strtok(input, "|");
    while ((pipeInput[sz] = strtok(NULL, "|")) != NULL) sz++;
    return sz;
}

int ownCmdChild(char** cmd)
{
    int cmdSize = 1, cmdOption = 0;
    char* cmdList[cmdSize];
    char cwd[NUM_STR];
    cmdList[0] = "pwd";
  
    for (int i = 0; i < cmdSize; i++) {
        if (strcmp(cmd[0], cmdList[i]) == 0) {
            cmdOption = i + 1;
            break;
        }
    }
  
    switch (cmdOption) {
        case 1:
            if (getcwd(cwd, sizeof(cwd))!= NULL) printf("%s\n", cwd);
            else perror("failed to run getcwd()\n");
            return 1;
        default:
            break;
    }
  
    return 0;
}

void run_command(struct Inputs inputs) {
    redirect(inputs);
    if (ownCmdChild(inputs.command) == 1) exit(0);
    else execvp(inputs.command[0], inputs.command);
}

int ownCmdParent(char** cmd)
{
    int cmdSize = 1, cmdOption = 0;
    char* cmdList[cmdSize];
    cmdList[0] = "cd";
  
    for (int i = 0; i < cmdSize; i++) {
        if (strcmp(cmd[0], cmdList[i]) == 0) {
            cmdOption = i + 1;
            break;
        }
    }
  
    switch (cmdOption) {
        case 1:
            chdir(cmd[1]);
            return 1;
        default:
            break;
    }
  
    return 0;
}


void pipe_exec(char *input) {
    int status; // dead child's status
    char **pipeInput = malloc(512*sizeof(char*));
    unsigned long numCommand = splitPipeCommand(input, pipeInput);
    pid_t *pids = malloc(numCommand * sizeof(pid_t*));
    int old_fd[2], new_fd[2];
    // printf("pipe id is %d and %d", fd[0], fd[1]);
    struct Inputs** batch_command = malloc(numCommand * sizeof(struct Inputs*));

    for (unsigned long i = 0; i < numCommand; i++) {
        pids[i] = 0;
        if (i != numCommand - 1) pipe(new_fd);
        batch_command[i] = malloc(sizeof(struct Inputs));
        parse_args(pipeInput[i], batch_command[i]);
        if (ownCmdParent(batch_command[i]->command) == 0) {
        if ((pids[i] = fork()) < 0) {
            perror("fail to fork");
            abort();
        } else if (pids[i] == 0) {
            signal(SIGINT, SIG_DFL);

            if (i != 0) {
                dup2(old_fd[0], 0);
                close(old_fd[0]);
                close(old_fd[1]);
            }
            if (i != numCommand - 1) {
                dup2(new_fd[1], 1);
                close(new_fd[1]);
                close(new_fd[0]);
            }
            run_command(*batch_command[i]);
        }
        if (i != 0) {
            close(old_fd[0]);
            close(old_fd[1]);
        }
        if (i != numCommand - 1) {
            old_fd[0] = new_fd[0];
            old_fd[1] = new_fd[1];
        }
        }
    }

    for (unsigned long i = 0; i < numCommand; i++) {
        if (pids[i] != 0) waitpid(pids[i], &status, WUNTRACED);
        free(batch_command[i]->command);
        if (batch_command[i]->input_redir) free(batch_command[i]->ifile);
        if (batch_command[i]->output_redir != None) free(batch_command[i]->ofile);
        free(batch_command[i]);
        // printf("Process %d completed\n", pids[i]);
    }

    free(input);
    free(pids);
    free(pipeInput);
    free(batch_command);
}
