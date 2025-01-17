#include "parsing.h"

Command* parseCommand(char* cmdline) {
    Command* cmd = malloc(sizeof(Command));
    cmd->args = malloc(MAXIMUM_ARGUMENTS * sizeof(char*));
    cmd->arg_count = 0;
    cmd->input_file = NULL;
    cmd->output_file = NULL;
    cmd->append_output = 0;
    cmd->is_background = 0;

    char* current = cmdline;
    char temp[MAXIMUM_CMD_LENGTH] = "";
    int temporary_index = 0;
    int in_quotes = 0;
    char quote_character = '\0';

    // Checking for a background execution
    size_t len = strlen(cmdline); // Check if the command should run in the background by looking for '&' at the end of the command line
    if (len > 0 && cmdline[len-1] == '&') {
        cmd->is_background = 1;
        cmdline[len-1] = '\0';
        // Remove extra spaces already after removing &
        while (len > 1 && (cmdline[len-2] == ' ' || cmdline[len-2] == '\t')) {
            cmdline[len-2] = '\0';
            len--;
        }
    }
    // Parse each character in the command line to separate arguments, handle quotes, and handle redirection
    while (*current != '\0') {
        // Handling quotes "" and ''
        if ((*current == '"' || *current == '\'') && !in_quotes) {
            in_quotes = 1;
            quote_character = *current;
            current++;   // Move to the next character after the opening quote
            continue;
        } else if (*current == quote_character && in_quotes) {  //  Closing quote found, exit quote mode
            in_quotes = 0;
            quote_character = '\0';
            current++;  
            continue;
        }
        
        // Handle redirection outside of quotations
        if (!in_quotes && (*current == '>' || *current == '<')) {
            // Save the current argument before processing redirection
            if (temporary_index > 0) {
                temp[temporary_index] = '\0';
                cmd->args[cmd->arg_count++] = strdup(temp); // Store the argument in args array
                temporary_index = 0;
            }

            char redirectional_char = *current;
            // Check for >>
            if (redirectional_char == '>' && *(current + 1) == '>') {
                cmd->append_output = 1;
                current++;  // move past the second >
            }

            // Move past the redirection character and any spaces or tabs following it
            current++;
            while (*current == ' ' || *current == '\t') current++;

            // Filename gets read here
            temporary_index = 0;
            while (*current && *current != ' ' && *current != '\t' && 
                   *current != '>' && *current != '<') {
                temp[temporary_index++] = *current++; //  Copy each character to the temp array
            }
            temp[temporary_index] = '\0'; //  Null-terminate the filename string
            current--;  // incrementing loop, so logically remove 1 from current before we do so

            if (redirectional_char == '>') {  // if detecting '>'
                cmd->output_file = strdup(temp);
            } else {
                cmd->input_file = strdup(temp);
            }
            temporary_index = 0;  // set temp to 0
        }
        // Handle spaces outside of quoted text as argument delimiters
        else if ((*current == ' ' || *current == '\t') && !in_quotes) {
            // If there is an argument collected in temp, finalize it and add to args
            if (temporary_index > 0) {
                temp[temporary_index] = '\0';
                cmd->args[cmd->arg_count++] = strdup(temp);
                temporary_index = 0;
            }
        }
        // Add current character to the current argument being collected
        else {
            temp[temporary_index++] = *current;
        }
        current++; // Move to the next character in the command line
    }

    // Check if there is any argument collected in temp
    if (temporary_index > 0) {
        temp[temporary_index] = '\0';  // Null terminate the last arg string
        cmd->args[cmd->arg_count++] = strdup(temp);
    }
    // Mark the end of the arguments array with NULL to indicate no more arguments
    cmd->args[cmd->arg_count] = NULL;


    return cmd;
}
//Frees all dynamically allocated memory associated with a Command structure.
void free_this_Command(Command* cmd) {
    if (cmd) {
        for (int i = 0; i < cmd->arg_count; i++) {  // Iterates through each argument stored in `cmd->args`, freeing each argument string.
            free(cmd->args[i]);
        }
        free(cmd->args);
        free(cmd->input_file);  // Frees any allocated memory for `input_file` and `output_file`
        free(cmd->output_file);
        free(cmd);
    }
}
