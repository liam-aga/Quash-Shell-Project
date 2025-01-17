//execute.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>  // Added for MAXIMUM_PATH
#include "parsing.h" // Add this line to include the Command struct definition
#include <stdbool.h> // For bool

#define MAXIMUM_CMD_LENGTH 1024
#define MAXIMUM_ARGUMENTS 64
#define MAXIMUM_JOBS 32

#ifndef MAXIMUM_PATH // idk
#define MAXIMUM_PATH 4096 // idk
#endif  // idk

// Structure to track background jobs
typedef struct {
    int jobId;
    pid_t pid;
    char command[MAXIMUM_CMD_LENGTH];
    int runningStatus;
} Job;

// Global variables
Job jobs[MAXIMUM_JOBS];
int nextJobandID = 1;

// Function declarations
void initJobs();
void addJobs(pid_t pid, const char* command);
void remove_a_Job(int jobId);
void checking_Jobss();
void executeCmd(char* command, int background);
void builtinCmdHandler(char** args);
void redirectionHandler(char** args);
void handlingPipedCmds(char** commands, int numCommands);

// Initialize job array
void initJobs() {
    //  Loop through the maximum number of jobs
    for (int i = 0; i < MAXIMUM_JOBS; i++) {
        jobs[i].jobId = 0;  // Set job ID to 0 indicating no job assigned
        jobs[i].pid = 0; // Set process ID to 0 indicating no job running
        jobs[i].runningStatus = 0;
    }
}

// Add a new background job
void addJobs(pid_t pid, const char* command) {
     // Loop through the job array to find an empty slot for a new job
    for (int i = 0; i < MAXIMUM_JOBS; i++) {
        // Check if the current job slot is free
        if (!jobs[i].runningStatus) {
            jobs[i].jobId = nextJobandID++; // Assign a new job ID and increment the counter
            jobs[i].pid = pid; // set PID for a new job
            // Copy the command string to the job structure, ensuring it fits within the buffer
            strncpy(jobs[i].command, command, MAXIMUM_CMD_LENGTH - 1);
            jobs[i].runningStatus = 1;
            printf("Background job started: [%d] %d %s\n", // changed
                   jobs[i].jobId, (int)jobs[i].pid, jobs[i].command);
            break;
        }
    }
}

// Remove a completed job
void remove_a_Job(int jobId) {
       // Loop through the job array to find the job with the specified jobId
    for (int i = 0; i < MAXIMUM_JOBS; i++) {
        // Check if the job ID matches the one to be removed
        if (jobs[i].jobId == jobId) {
            printf("Completed: [%d] %d %s\n", 
                   jobs[i].jobId, (int)jobs[i].pid, jobs[i].command);
            jobs[i].runningStatus = 0; // Mark the job as not running
            jobs[i].jobId = 0; // reset JOB ID to show its no longer active
            jobs[i].pid = 0;  // reset process ID
            break;
        }
    }
}

// Check status of background jobs
void checking_Jobss() {
    for (int i = 0; i < MAXIMUM_JOBS; i++) {
        // Proceed only if the job is currently running
        if (jobs[i].runningStatus) {
            int status;
            // Use waitpid to check the status of the background job without blocking
            pid_t result = waitpid(jobs[i].pid, &status, WNOHANG);
            // If the job has completed, remove it from the job list
            if (result > 0) {
                remove_a_Job(jobs[i].jobId);
            }
        }
    }
}


// Handle built-in commands
void builtinCmdHandler(char** args) {
      // Check if the command is "cd" (change directory)
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            // Change to HOME if no argument is provided
            chdir(getenv("HOME"));
        } else {
            // Check if the argument is an environment variable and expand it
            if (args[1][0] == '$') {
                const char* env_value = getenv(args[1] + 1); // Get value after '$'
                if (env_value != NULL) {
                    // Change to the directory specified by the environment variable
                    if (chdir(env_value) != 0) {
                        perror("cd failed");
                    }
                } else {
                    // Print error if the environment variable does not exist
                    perror("cd failed: No such file or directory");
                }
            } else {
                // Attempt to change to the specified directory
                if (chdir(args[1]) != 0) {
                    perror("cd failed");
                }
            }
        }
        
        // Update the PWD environment variable to reflect the current working directory
        char cwd[MAXIMUM_PATH];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            setenv("PWD", cwd, 1);
        } else {
            perror("getcwd() error");
        }
    }
        // Check if the command is "pwd" (print working directory)
    else if (strcmp(args[0], "pwd") == 0) {
        char cwd[MAXIMUM_PATH];
         // Get the current working directory and print it
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("getcwd() error");
        }
    }
        // Check if the command is "echo"
    else if (strcmp(args[0], "echo") == 0) {
    //printf("Debug: Arguments passed to echo:\n");

    // Loop through each argument to process it
    for (int i = 1; args[i] != NULL; i++) {
        //printf("args[%d]: %s\n", i, args[i]); // Debug print for each argument

        // Initialize a buffer to build the output string
        char buffer[MAXIMUM_PATH];
        memset(buffer, 0, sizeof(buffer));  // clear buffer
        int bufferIndex = 0;  // reset buffer index

        // Process each part of the argument
        for (int j = 0; args[i][j] != '\0'; j++) {
            if (args[i][j] == '$') {
                // Detect start of environment variable
                j++; // Move past the '$' to get var name
                char varName[100];  // buffer
                int k = 0;  // index for var name extension

                // Extract the variable name until '/' or end of string
                while (args[i][j] != '/' && args[i][j] != '\0') {
                    varName[k++] = args[i][j++];
                }
                varName[k] = '\0'; // Null-terminate the variable name
                j--; // Adjust `j` to point to the last character processed for the next iteration

                // Get environment variable value
                char* env_val = getenv(varName);
                if (env_val) {
                    // If the environment variable exists, append its value to the buffer
                    strncpy(buffer + bufferIndex, env_val, sizeof(buffer) - bufferIndex - 1);
                    bufferIndex += strlen(env_val);
                } else {
                    printf("Debug: No value found for variable: %s\n", varName);
                }
            } else {
                // If not an environment variable, append the regular character to the buffer
                buffer[bufferIndex++] = args[i][j];
            }
        }
        // Output the processed buffer for the argument
        printf("%s", buffer);
        // Print a space if there are more arguments to follow
        if (args[i + 1] != NULL) printf(" ");
    }
    printf("\n");
}

    else if (strcmp(args[0], "export") == 0) {
   if (args[1] != NULL) {
       // Separate variable name from value at '='
       char* equals = strchr(args[1], '=');
       if (equals) {
           *equals = '\0'; // Split the variable name from its value
           // Now we need to set the variable
           char* value = equals + 1; // Value starts after '='
           // Expand $HOME and other environment variables manually
           if (strcmp(args[1], "PATH") == 0 && strstr(value, "$HOME")) {
               char* home_value = getenv("HOME");
               if (home_value) {
                   // Replace $HOME with the actual path
                   char new_value[MAXIMUM_CMD_LENGTH];
                   snprintf(new_value, sizeof(new_value), "%s%s", home_value, value + 5); // assuming value starts with $HOME
                   setenv(args[1], new_value, 1); // Set the environment variable
               }
           } else {
               setenv(args[1], value, 1); // Set the environment variable normally
           }
       }
   }
}

    else if (strcmp(args[0], "jobs") == 0) {
        for (int i = 0; i < MAXIMUM_JOBS; i++) {
            if (jobs[i].runningStatus) {
                // Print the job ID, process ID, and command of each running job
                printf("[%d] %d %s\n", 
                       jobs[i].jobId, (int)jobs[i].pid, jobs[i].command);
            }
        }
    }
    else if (strcmp(args[0], "kill") == 0) {
        // Check if an argument is provided for the kill command
       if (args[1] != NULL) {
        // Check if the argument is a job ID (formatted as %1)
           if (args[1][0] == '%' && isdigit(args[1][1])) {
               // Handle job ID like %1
               int jobId = atoi(args[1] + 1); // Skip the '%' character
               int found = 0; // Flag to check if job is found
               for (int i = 0; i < MAXIMUM_JOBS; i++) {
                // Look for the job in the jobs array
                   if (jobs[i].jobId == jobId && jobs[i].runningStatus) {
                    // Attempt to terminate the job's process
                       if (kill(jobs[i].pid, SIGTERM) == 0) {
                           printf("Killed job [%d]\n", jobId);
                       } else {
                           perror("kill failed");
                       }
                       found = 1;
                       break;
                   }
               }
               if (!found) {
                   printf("No such job: [%d]\n", jobId);
               }
           } else {
               // Handle termination using PID directly
               char* jobStr = args[1];
               if (jobStr[0] == '[') {
                   jobStr++;
                   jobStr[strlen(jobStr)-1] = '\0';
               }
               
               // Convert to integer PID
               int pid = atoi(jobStr);
               // try and terminate
               if (kill(pid, SIGTERM) == 0) {
                   printf("Killed process %d\n", pid);
               } else {
                   perror("kill failed");
               }
           }
       } else {
           printf("Usage: kill [job_id | pid]\n");
       }
   }
}



// Setup I/O redirection
void redirectionHandler(char** args) {
            // Check for input redirection
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0) {
            int fd = open(args[i + 1], O_RDONLY);  // open file for reading
            if (fd < 0) {
                perror("Input redirection failed");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
            args[i] = NULL;
        }
             // Check for output redirection (overwrite)
        else if (strcmp(args[i], ">") == 0) {
            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);  // for writing
            if (fd < 0) {
                perror("Output redirection failed");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[i] = NULL;
        }
           // Check for output redirection (append)
        else if (strcmp(args[i], ">>") == 0) {
            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);  // open file for appending
            if (fd < 0) {
                perror("Output redirection failed");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);  // close file descriptor
            args[i] = NULL; // Null-terminate the argument list for exec
        }
    }
}

// Execute piped commands
void handlingPipedCmds(char** commands, int numCommands) {
    int pipes[MAXIMUM_ARGUMENTS][2];
    
    // Create all pipes first
    for (int i = 0; i < numCommands - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe");
            exit(1);
        }
    }

    // Execute each command
    for (int i = 0; i < numCommands; i++) {
        pid_t pid = fork();  // create child
        if (pid == 0) {  // child
            // Set up input from previous pipe
            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            
            // Set up output to next pipe if not the last cmd
            if (i < numCommands - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            
            // Close all pipe file descriptors in child process
            for (int j = 0; j < numCommands - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // Parse and execute the command
            Command* cmd = parseCommand(commands[i]);
            if (cmd == NULL) {
                perror("Failed to parse command");
                exit(1);
            }
            execvp(cmd->args[0], cmd->args);
            perror("execvp");
            free_this_Command(cmd);
            exit(1);
        }
    }

    // Parent closes all pipe file descriptors
    for (int i = 0; i < numCommands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all children to finish
    for (int i = 0; i < numCommands; i++) {
        wait(NULL);
    }
}

// Main command execution function
void executeCmd(char* cmdline, int background) {
    // First check if the command contains pipes
    char* pipe_commands[MAXIMUM_ARGUMENTS];
    int num_commands = 0;
    
    // Split the command by pipes
    char* cmd_copy = strdup(cmdline); // Create a duplicate of the command line
    char* token = strtok(cmd_copy, "|"); // Tokenize by pipe symbol
    while (token != NULL && num_commands < MAXIMUM_ARGUMENTS) {
        pipe_commands[num_commands++] = strdup(token);
        token = strtok(NULL, "|"); // Get next command
    }
    free(cmd_copy);

    // Expand environment variables in each command
    for (int i = 0; i < num_commands; i++) {
        char* command = pipe_commands[i];
        char expanded_command[MAXIMUM_CMD_LENGTH] = ""; // Buffer for the expanded command
        char* arg = strtok(command, " "); // Split by spaces

        while (arg != NULL) {
                  // Check if the argument is an environment variable
            if (arg[0] == '$') {
                char* env_value = getenv(arg + 1); // Expand variable
                if (env_value) {
                    strcat(expanded_command, env_value);  // append if val not found
                } else {
                    strcat(expanded_command, ""); // Handle non-existent variable
                }
            } else {
                strcat(expanded_command, arg);  // appemnd regular arg
            }
            strcat(expanded_command, " "); // Add space after each argument
            arg = strtok(NULL, " "); // Get next argument
        }

        // Now, replace the command with the expanded command
        pipe_commands[i] = strdup(expanded_command);
    }

    // Execute the commands as before (handle pipes and regular execution)
    if (num_commands > 1) {
        handlingPipedCmds(pipe_commands, num_commands);
        // Clean up aloocated memory for cmds
        for (int i = 0; i < num_commands; i++) {
            free(pipe_commands[i]);
        }
        return;
    }

    // Execute the single command
    Command* cmd = parseCommand(pipe_commands[0]);
    // Continue with your command execution logic...

    // Handle built-in commands with redirection
    if (cmd->arg_count > 0 && (
        strcmp(cmd->args[0], "cd") == 0 || 
        strcmp(cmd->args[0], "pwd") == 0 ||
        strcmp(cmd->args[0], "echo") == 0 ||
        strcmp(cmd->args[0], "export") == 0 ||
        strcmp(cmd->args[0], "jobs") == 0 ||
        strcmp(cmd->args[0], "kill") == 0 )) {
        
        // Save stdout if we need to redirect
        int saved_stdout = -1;
        if (cmd->output_file) {
            saved_stdout = dup(STDOUT_FILENO);
            int flags = O_WRONLY | O_CREAT;
            flags |= (cmd->append_output ? O_APPEND : O_TRUNC);
            mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;  // 0644
            int fd = open(cmd->output_file, flags, mode);
            if (fd == -1) {
                perror("Failed to open output file");
                free_this_Command(cmd);
                return;
            }
            if (dup2(fd, STDOUT_FILENO) == -1) {
                perror("Failed to redirect output");
                close(fd);
                free_this_Command(cmd);
                return;
            }
            close(fd);
        }

        // Execute the built-in command
        builtinCmdHandler(cmd->args);

        // Restore stdout if we redirected it
        if (saved_stdout != -1) {
            dup2(saved_stdout, STDOUT_FILENO);
            close(saved_stdout);
        }

        free_this_Command(cmd);
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        // Handle input redirection
        if (cmd->input_file) {
            int fd = open(cmd->input_file, O_RDONLY); // Open the input file for reading
            if (fd == -1) {
                perror("Failed to open input file");
                exit(1);
            }
            if (dup2(fd, STDIN_FILENO) == -1) {
                perror("Failed to redirect input");
                exit(1);
            }
            close(fd); // Close the file descriptor after duplicating
        }

        // Handle output redirection
        if (cmd->output_file) {
                  // Set flags for opening output file
            int flags = O_WRONLY | O_CREAT;
            flags |= (cmd->append_output ? O_APPEND : O_TRUNC); // Append or truncate based on command
            mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;  // 0644
            int fd = open(cmd->output_file, flags, mode);
            if (fd == -1) {
                perror("Failed to open output file");
                exit(1);
            }
            if (dup2(fd, STDOUT_FILENO) == -1) {
                perror("Failed to redirect output");
                exit(1);
            }
            close(fd);
        }

        // Execute command using execvp
        execvp(cmd->args[0], cmd->args); // Replace child process with the command
        perror("Command not found"); // Report error if command execution fai
        exit(1);
    } else if (pid > 0) {
        // Parent process
        if (background || cmd->is_background) {
             // If command is background, add it to job list without waiting
            addJobs(pid, cmdline);
            // If foreground, wait for the command to finish
        } else {
            waitpid(pid, NULL, 0);
        }
    } else {
        perror("fork");
    }

    // Free command resources after execution
    free_this_Command(cmd);
}


int main() {
    char command[MAXIMUM_CMD_LENGTH];  // Buffer to hold user command
    initJobs(); // Initialize job tracking

    printf("Welcome to Quash!\n");
    printf("[QUASH]$ ");
    fflush(stdout);

    while (fgets(command, sizeof(command), stdin) != NULL) {
        // Remove trailing newline
        command[strcspn(command, "\n")] = '\0'; // Strip newline character from command


        // Check for exit commands
        if (strcmp(command, "exit") == 0 || strcmp(command, "quit") == 0) {
            break;
        }

        // Check for empty command
        if (strlen(command) == 0) {
            printf("[QUASH]$ ");
            continue;
        }

        // Check for background execution
        int background = 0;
        if (command[strlen(command) - 1] == '&') {
            background = 1;
            command[strlen(command) - 1] = '\0';
            // Remove any trailing spaces
            while (command[strlen(command) - 1] == ' ') {
                command[strlen(command) - 1] = '\0';
            }
        }

        // Check status of background jobs
        checking_Jobss();  // update job statuses

        // Execute the command
        executeCmd(command, background);

        printf("[QUASH]$ ");
        fflush(stdout);
    }

    printf("Bye, see ya later!\n");  // Bye message
    return 0;
}
