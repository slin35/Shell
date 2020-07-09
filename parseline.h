#ifndef PARSELINE_H
#define PARSELINE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_CMD_LEN 1024
#define MAX_CMD_PIPE 10
#define MAX_CMD_ARGS 10


extern int AmbiguousInputFlag;
extern int AmbiguousOutputFlag;
extern int TooManyArgumentsFlag;
extern int InvalidNullCommandFlag;

typedef struct Command{
  char cmdline[MAX_CMD_LEN+1];
  int inputFlag;
  int outputFlag;
  char input[257];
  char output[257];
  int argc;
  char *argv[MAX_CMD_ARGS+10];
}Command;

void parseCmdLine(char *cmdline, Command list_of_commands[MAX_CMD_PIPE+1], int *numStages);
int is_valid_file(char *filename);
void parse_argv_helper(char *command, char *argv[MAX_CMD_ARGS+10]);
void parse_argv(Command list_of_commands[MAX_CMD_PIPE+1], int index, char *argv[MAX_CMD_ARGS+10], int numStages);
void ambiguous_input_output(Command list_of_commands[MAX_CMD_PIPE+1], int numStages);
void parseCommand(char *cmdline, Command list_of_commands[MAX_CMD_PIPE+1], int numStages);

#endif