# QUASH (Quite a Shell)

QUASH is a custom Unix-based shell implemented in C, designed to simulate functionalities of popular shells like Bash and Csh. The project involved collaborative design and development, focusing on key shell features such as command parsing, job control, I/O redirection, environment variable management, and piping.

## Key Features

- **Command Parsing:** Interprets and executes user input commands.
- **Job Control:** Supports background and foreground job management.
- **I/O Redirection:** Handles input/output redirection to files and other commands.
- **Environment Variables:** Allows manipulation and management of environment variables.
- **Piping:** Supports piping output from one command to the input of another.
- **Built-in Functions:** Implemented several built-in functions like `echo`, `cd`, etc.
- Refrained from using system(3) calls in the entire project.

## Technologies

- **C Programming Language**
- **Unix System Calls**
- **Process Management**
- **Memory Management** (Valgrind used to eliminate memory errors)
- **Shell Scripting Concepts**

## Authors

- **Liam Aga**
- **Conner Glazner**

## Collaboration

This project was developed in collaboration with **Conner Glazner**, where we worked together to design and implement the core functionalities.
Git-Hub: Cglaz123

## How to Run QUASH

Go to directory where the "src" and Makefile is.
Run "make test"

To clean quash use:
"make clean"
