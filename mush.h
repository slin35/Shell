#ifndef MUSH_H
#define MUSH_H

#include "parseline.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>

#define INTERACTIVE 1
#define BATCH 0

void process_args(int argc, char *argv[], FILE *filePtr, int mode);
void strip_newline_and_tab(char *cmdline);
void strip_newline(char *string);

int check_cd(char *cmdline, int mode);
int check_num_stages(int numStages, int mode);

void execute_list_of_commands(Command list_of_commands[MAX_CMD_PIPE+1], int numStages);
void manage_parent_pipe(Command command, int index, int numStages, int pipefd[]);
void manage_child_pipe(int index, int numStages);
void redirect_fds(Command command, int index, int numStages);
void redirect_input(int fdread, Command command);
void redirect_output(int fdwrite, Command command);
void cleanup(int numStages);

int raiseAmbiguousInputOutputError(int mode);
int raiseTooManyArgumentsError(int mode);
int raiseInvalidNullCommandError(int mode); 

void sigint_handler(int signum);

#endif
