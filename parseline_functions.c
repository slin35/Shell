#include "parseline.h"

int AmbiguousInputFlag = 0;
int AmbiguousOutputFlag = 0;
int TooManyArgumentsFlag = 0;
int InvalidNullCommandFlag = 0;

void parseCmdLine(char *cmdline, Command list_of_commands[MAX_CMD_PIPE+1], int *numStages) {
    char *token;
    char *rest = cmdline;
    int count=0;
    
    while ((token = strtok_r(rest, "|", &rest))) {

        if (strcmp(token, " ")==0) {
        /*    perror("invalid null command");
            exit(1);*/
            InvalidNullCommandFlag = 1;
            return ;
        }
        
        strcpy(list_of_commands[count].cmdline, token);
        count++;       
    }
    
    *numStages = count;
}

int is_valid_file(char *filename) {
    if (filename == NULL) {
        return 0;
    }
    if (strcmp(filename, "<")==0) {
        return 0;
    }
    if (strcmp(filename, ">")==0) {
        return 0;
    }
    if (strcmp(filename, "|")==0) {
        return 0;
    }
    
    return 1;
}

void parse_argv_helper(char *command, char *argv[MAX_CMD_ARGS+10]) {
    char *token = NULL;
    int count = 0;
    
    token = strtok(command, " ");
    while (token!=0) {
        argv[count] = token;
        token = strtok(0, " ");
        
        count++;
    }
    argv[count] = NULL;
}


void parse_argv(Command list_of_commands[MAX_CMD_PIPE+1], int index, char *argv[MAX_CMD_ARGS+10], int numStages) {
    int i=0, k=0;
    
    if (strcmp(argv[0], ">")==0 || strcmp(argv[0], "<")==0 || strcmp(argv[0], "|")==0) {
   /*     perror("invalid null command");
        exit(1);*/
        InvalidNullCommandFlag = 1;
        return ;
    }
    
    list_of_commands[index].inputFlag=0;
    list_of_commands[index].outputFlag=0;
    
    while (argv[i]!= NULL) {
        
        if (strcmp(argv[i], "<")==0){
            if (!list_of_commands[index].inputFlag && is_valid_file(argv[i+1])) {

                list_of_commands[index].inputFlag=1;
                strcpy(list_of_commands[index].input, argv[i+1]);        

                i+=2;
            }
            else {
                printf("%s: bad input redirecton\n", argv[0]);
                exit(1);
            }            
        }
        else if (strcmp(argv[i], ">")==0) {
            if (!list_of_commands[index].outputFlag && is_valid_file(argv[i+1])) {
                list_of_commands[index].outputFlag=1;
                strcpy(list_of_commands[index].output, argv[i+1]);         

                i+=2;
            }
            else {
                printf("%s: bad output redirection\n", argv[0]);
                exit(1);
            }
        }
        else {
            list_of_commands[index].argv[k] = argv[i];
            k++;
            i++;
        }
    }
    
    list_of_commands[index].argv[k]=NULL;
    list_of_commands[index].argc=k;
    
    if (index==0 && !list_of_commands[index].inputFlag) {
        list_of_commands[index].inputFlag = 2;
        strcpy(list_of_commands[index].input, "original stdin");
    }
    if (index==numStages-1 && !list_of_commands[index].outputFlag) {
        list_of_commands[index].outputFlag = 2;
        strcpy(list_of_commands[index].output, "original stdout");
    }
    
    if (k > MAX_CMD_ARGS) {
     //   printf("%s: \n", list_of_commands[index].argv[0]);
        printf("Too many arguments.\n");
        TooManyArgumentsFlag = 1;
    }
}

void ambiguous_input_output(Command list_of_commands[MAX_CMD_PIPE+1], int numStages) {
    int i;
    for (i = 0; i < numStages; i++) {

        if (i != 0 && list_of_commands[i].inputFlag) {
            printf("%s: Ambiguous Input\n", list_of_commands[i].argv[0]);
            AmbiguousInputFlag = 1;
            break;
        }
        if (i != numStages-1 && list_of_commands[i].outputFlag) {
            printf("%s: Ambiguous Output\n", list_of_commands[i].argv[0]);
            AmbiguousOutputFlag = 1;
            break;
        }

    }

}

void parseCommand(char *cmdline, Command list_of_commands[MAX_CMD_PIPE+1], int numStages) {
    int index = 0;
    char *argv[MAX_CMD_ARGS+10];
   // char command[MAX_CMD_LEN+1];
  
    while (index < numStages) {
        
     //   strcpy(command, list_of_commands[index].cmdline);
        
        parse_argv_helper(list_of_commands[index].cmdline, argv);      
        parse_argv(list_of_commands, index, argv, numStages);
        
        index++;
    }
    
    ambiguous_input_output(list_of_commands, numStages);    
}

