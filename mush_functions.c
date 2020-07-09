#include "mush.h"


int sigint = 0;

void process_args(int argc, char *argv[], FILE *filePtr, int mode) {
    char cmdline[1025];
    Command list_of_commands[MAX_CMD_PIPE+1];
    int numStages;
    
    if (mode) {
        printf("8-P ");
        fflush(stdout);
    }
    errno = 0;
    while (1) {
	signal(SIGINT, sigint_handler);
        if (fgets(cmdline, 1024, filePtr) == NULL) {
            if (errno == EINTR) {
		printf("\n");
                break;
            }
            if (feof(filePtr)) {
                printf("\n");
                break;
		}
            printf("\n");
            continue;
        }
        
        if (sigint) {
            printf("8-P ");
            fflush(stdout);
            sigint = 0;
            continue;
        }
        strip_newline_and_tab(cmdline);

        /*check for cd command*/
        if (check_cd(cmdline, mode))
            continue;
     
        /*parsing one cmdline and return a list of commands to execute*/
        parseCmdLine(cmdline, list_of_commands, &numStages);
        
        /*pipe too deep error*/
        if (check_num_stages(numStages, mode))
            continue;
          
        parseCommand(cmdline, list_of_commands, numStages);
        
        if (raiseAmbiguousInputOutputError(mode) || raiseTooManyArgumentsError(mode) || raiseInvalidNullCommandError(mode)) 
            continue;
        
        execute_list_of_commands(list_of_commands, numStages);

       if (mode) {
            printf("8-P ");
            fflush(stdout);
       }
    }

}

int check_num_stages(int numStages, int mode) {
    if (numStages > MAX_CMD_PIPE) {
        printf("Pipeline too deep.\n");
        if (mode) {
            printf("8-P ");
            fflush(stdout);
            }
        return 1;
    }
    return 0;
}

void strip_newline_and_tab(char *cmdline) {
    int i;
    
    for (i = 0; i < strlen(cmdline); i++) {
        if (cmdline[i] == '\t' || cmdline[i] =='\n') {
            cmdline[i] = ' ';
        }
    }
}

void strip_newline(char *string) {
    int i;
    for (i = 0; i < strlen(string); i++) {
        if (string[i]==' ' || string[i] == '\n') 
            string[i]='\0';
    }
}

int check_cd(char *cmdline, int mode) {
    char *token;
    int return_value = 0;
    
    if (strncmp(cmdline, "cd", 2)==0) {
        token = strtok(cmdline, " ");
        token = strtok(NULL, cmdline);
        strip_newline(token);
        if ((return_value = chdir(token)) < 0) {
            perror(token);
            
        }
        
        if (mode) {
            printf("8-P ");
            fflush(stdout);
        }
        return 1;
    }
    
    return 0;
}

void execute_list_of_commands(Command list_of_commands[MAX_CMD_PIPE+1], int numStages) {
    int index;
    int pipe1[2];
    pid_t child;
    
 
    for (index = 0; index < numStages; index++) {
        
        /* Manage parent pipe ends */
        manage_parent_pipe(list_of_commands[index], index, numStages, pipe1);
        
        /* Fork */
        if ((child = fork()) == 0) 
        {
            /* Manage child pipe ends */
	  manage_child_pipe(index, numStages);
            /* Redirect file descriptors with dup2 */
            redirect_fds(list_of_commands[index], index, numStages);
            /* Execute (execvp) */
            execvp(list_of_commands[index].argv[0], list_of_commands[index].argv);
            
            perror(list_of_commands[index].argv[0]);
        }    
    }
    
    if (numStages > 1 && ((numStages -1) % 2)) {
        close(3);
    }
    else if (numStages > 1 && !((numStages - 1) % 2)){
        close(4);
    }
    
    /* Wait for children to terminate */  
    cleanup(numStages);
    
}

void manage_parent_pipe(Command command, int index, int numStages, int pipefd[]) {
    if (numStages == 1) {
        return;
    }
    else {
        if (index == numStages - 1) {
            if (numStages == 2) {
                close(4);
            }
            else if (numStages > 2 && (index %2)) {
                close(5);
                close(4);
            }
            else if (numStages > 2 && !(index %2)) {
                close(5);
                close(3);
            }
            return;
        }
        else if (index == 0) {
            if (pipe(pipefd) == -1) {
                perror("pipe");
                exit(1);
            }
        }
        else if (index == 1) {
            close(4);
            if (pipe(pipefd) == -1) {
                perror("pipe");
                exit(1);
            }
        }
        else if (index % 2) {
            close(5);
            close(4);
            if (pipe(pipefd) == -1) {
                perror("pipe");
                exit(1);
            }
        }
        else {
            close(5);
            close(3);
            if (pipe(pipefd) == -1) {
                perror("pipe");
                exit(1);
            }
        }
        
    }
}

/* cat reallyBigFile.txt | head -41 | tail -7 */

void manage_child_pipe(int index, int numStages) {
    if (index % 2 != 0 && index != numStages - 1) {
        close(4);
    }
    else if (index % 2 == 0 && index != numStages - 1) {
      close(3);
    }
}

void redirect_input(int fdread, Command command) {
    if (command.inputFlag==1) {
        if ((fdread = open(command.input, O_RDONLY, 0)) < 0) {
            perror("invalid file");
            exit(1);
        }
        dup2(fdread, STDIN_FILENO);
    }   
}

void redirect_output(int fdwrite, Command command) {
    if (command.outputFlag == 1) {
        if ((fdwrite = open(command.output, O_RDWR | O_CREAT, 0666)) < 0) {
            perror("invalid file");
            exit(1);
        }
        dup2(fdwrite, STDOUT_FILENO); 
    }
     
}
void redirect_fds(Command command, int index, int numStages) {
    int fdread=0;
    int fdwrite=0;
    
    if (numStages == 1) {
        redirect_input(fdread, command);         
        redirect_output(fdwrite, command);                     
    }
    else if (index == 0) {
        redirect_input(fdread, command);
        dup2(4, STDOUT_FILENO);
    }
    else if (index == numStages - 1) {
        redirect_output(fdwrite, command);
        if (index % 2) {
            dup2(3, STDIN_FILENO);
        }
        else {
            dup2(4, STDIN_FILENO);
        }
    }
    else if (index % 2) {
        dup2(3, STDIN_FILENO);
        dup2(5, STDOUT_FILENO);
    }
    else {
        dup2(4, STDIN_FILENO);
        dup2(5, STDOUT_FILENO);
    }
   
}

void cleanup(int numStages) {
    int i;
    for (i=0; i < numStages; i++) 
        wait(NULL);
}

int raiseAmbiguousInputOutputError(int mode)  {
    if (AmbiguousOutputFlag || AmbiguousInputFlag) {
        AmbiguousInputFlag = 0;
        AmbiguousOutputFlag = 0;
        if (mode) {
            printf("8-P ");
            fflush(stdout);
        }
        
        return 1;
    }
    return 0;    
}

int raiseTooManyArgumentsError(int mode) {
    if (TooManyArgumentsFlag) {
        TooManyArgumentsFlag = 0;
        if (mode) {
            printf("8-P ");
            fflush(stdout);
        }
        return 1;
    }
    return 0;
}

int raiseInvalidNullCommandError(int mode) {
    if (InvalidNullCommandFlag) {
        InvalidNullCommandFlag = 0;
        if (mode) {
            printf("8-P ");
            fflush(stdout);
        }
        return 1;
    }
    return 0;
}

void sigint_handler(int signum) {
    sigint = 1;
}
