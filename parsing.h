#ifndef PARSING_H
#define PARSING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXIMUM_CMD_LENGTH 1024
#define MAXIMUM_ARGUMENTS 64

typedef struct {
    char** args;           
    int arg_count;         // Count and # of arguments
    char* input_file;      // Input 
    char* output_file;     // Output 
    int append_output;     // 1 for >>, 0 for >
    int is_background;     // checking for "&" 
} Command;

Command* parseCommand(char* cmdline);
void free_this_Command(Command* cmd);

#endif