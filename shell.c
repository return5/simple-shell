//--------------------------------------------- information ---------------------------------------------------------
//A simple shell built in C. Built solely because it seemed interesting and educational. Written by: github.com/return5
//Licensed: MIT
//
//  Copyright © 2020 <return5>
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//--------------------------------------------- headers ------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

//--------------------------------------------- macros -------------------------------------------------------------
#define DEL " \r\t\n"
#define EXIT 2090237503  //hash value for "exit"
#define HELP 2090324718  //hash value for "help"
#define CD 5863276       //hash value for "cd"

//--------------------------------------------- prototypes ---------------------------------------------------------
int hashString(char *str);
char **reSizeArgs(const int buff,char **args);
int builtIn(const char *const *args);
char *getLine(void);
char **getArgs(const char *const line);
int execCmd(const char *const *args);
void childProcess(const char *const *args);
void parentProcess(const pid_t pid);
int exitShell(void);
void printHelp(void);
void changeDir(const char *const *args);
void shellLoop(void);

//--------------------------------------------- code ----------------------------------------------------------------
//prints the help prompt
void printHelp(void) {
    printf("simple shell program written by github/return5.\n");
    printf("at prompt type in commands then hit enter.\n");
    printf("list of builtins:\n");
    printf("\texit - exits the shell.\n");
    printf("\tcd   - change current directory.\n");
    printf("\thelp - prints this help page.\n");
}

//change the current directory
void changeDir(const char *const *args) {
   if(chdir(args[1]) == -1) {
       fprintf(stderr,"Error in cd. try again\n");
   }
}

//exit the current shell.
int exitShell(void) {
    return 0;
}

//get a line of input from terminal
char *getLine(void) {
    size_t buff = 0;
    char *str   = NULL;
    if(getline(&str,&buff,stdin) != -1) {
        return str;
    }
    else {
        exit(EXIT_FAILURE);
    }
}

//resize the args array using realloc
char **reSizeArgs(const int buff,char **args) {
    char **temp = realloc(args,buff * sizeof(char *));
    if(temp == NULL) {
        temp = realloc(args,buff * sizeof(char *));
    }
    return temp;
}

//get each token from line and place it into args array
char **getArgs(const char *const line) {
    size_t buff  = 5;
    size_t i     = 0;
    char **args  = malloc(sizeof(char *) * buff);
    char *token  = strtok((char *)line,DEL);
    if(token == NULL) {
        exit(EXIT_FAILURE);
    }
    do {
        args[i++] = token;
        if(i >= buff) {
            buff += 5;
            args = reSizeArgs(buff,args);
        }
        token = strtok(NULL,DEL);
    } while(token != NULL);
    return args;
}

//hash string using DJB2.
int hashString(char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

//check if cmd is one of the built ins. If it is, run that command, otherwise return -1
int builtIn(const char *const * args) {
    switch(hashString((char *)args[0])) {
        case EXIT:
            return exitShell();
        case HELP:
            printHelp();
            return 1;
        case CD:
            changeDir(args);
            return 1;
        default: //cmd is not a built in
            return -1;  
    }
    return 0;
}


//child process executes here.
void childProcess(const char *const *args) {
    //try to execute command. if error it returns -1. 
    if(execvp(args[0],(char *const *)args) == -1) {
        fprintf(stderr,"error with %s\n",args[0]);
        exit(EXIT_FAILURE);
    }
}

//parent process executes here. it waits for child process to exit.
void parentProcess(const pid_t pid) {
    int status;
    do {
        waitpid(pid,&status,WUNTRACED);
    }while(!WIFEXITED(status) && !WIFSIGNALED(status));
}

//execute the command given by the user
int execCmd(const char *const *args) {
    const int status = builtIn(args);
    if(status == -1) {
        pid_t pid = fork();
        if(pid == 0) {
            childProcess(args);
            return 0;
        }
        else if( pid > 0) {
            parentProcess(pid);
            return 1;
        }
        else {
            fprintf(stderr,"error. fork went badly.\n");
            return 1;
        }
    }
    return status;
}

//take user commands, execute, then take another command. Rinse repeat.
void shellLoop(void) {
     int status = 1;
    do {
        printf(">");
        const char *const line   = getLine();
        const char *const *args  = (const char *const *)getArgs(line);
        status                   = execCmd(args);
        free((void *)line);
        free((void *)args);
     }while(status);
}

int main(void) {
    shellLoop();
    return EXIT_SUCCESS;
}

