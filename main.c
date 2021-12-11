#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //provides access to POSIX OS API
#include <sys/wait.h>
#include <setjmp.h>
#include "utility.h"

static volatile sig_atomic_t jump_active = 0;
static sigjmp_buf env;

void sigint_handler(int sig) {
    sig--;
    if (!jump_active) return;
    siglongjmp(env, 42);
} 

int main() {
    // current setup allows commands with arguments
    //Read, parse, execute loop
    signal(SIGINT, sigint_handler);
    while (1) {
        
        jump_active = 1;
        size_t in_size = 1024;
        char *input = malloc(in_size*sizeof(char));
        if (sigsetjmp(env, 1) == 42) {
            printf("\n");
        }

        printf("mumsh $ ");
        fflush(stdout);
        int len = (int) getline(&input, &in_size, stdin);
        if (len == 1) {
            free(input);
            continue;
        }
        else if (len == -1) {
            free(input);
            printf("exit\n");
            exit(0);
        }
        input[len-1] = ' ';

        jump_active = 0;
        // For commands that need to be run in the main process
        if (strcmp(input, "exit ") == 0) {
            free(input);
            printf("exit\n");
            exit(0);
        }
        // "Else" part for those that need a child process
        else {
            pipe_exec(input);
        }
        
    }
    return 0;
}
