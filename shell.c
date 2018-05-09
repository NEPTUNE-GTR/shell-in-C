#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>
//-------------------------------------------------------------------------------------
// Course:     COMP3430
// Section:    A01
// Assignment: 1, questions 4, 5, and 6 combined togehter all in one .c file
// Name:       Eddie Torres
// UM-ID:      umtorre8
// Semester:   Winter, 2018
// Description- This is a simple shell implementation, that sets user variables, pre-loads
// commands, and pipes commands
//-------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------
// CONSTANTS and TYPES
//-------------------------------------------------------------------------------------
typedef enum BOOL { FALSE, TRUE } Boolean;

#define DIMENSION 40
#define MAX 50
#define DELIM " \t\r\n\a \'='"

//-------------------------------------------------------------------------------------
// PROTOTYPES
//-------------------------------------------------------------------------------------
void start();
void clearShell();
void initalizeShell();
void pipedCommand(char commands[MAX]);
void validateString(char const * const string);
void arrayFill(char (*shellVariableNames)[DIMENSION]);
void preLoadCommands(char (*preLoadedCommands)[DIMENSION]);
void setShellVariable(const char * const arg, char (*shellVariableNames)[DIMENSION], char (*shellVariableCommands)[DIMENSION]);
void userInput(char (*shellVariableNames)[DIMENSION], char (*shellVariableCommands)[DIMENSION], char (*preLoadedCommands)[DIMENSION]);
//
//-------------------------------------------------------------------------------------
// FUNCTIONS
//-------------------------------------------------------------------------------------
//
int main(int argc, const char * const argv[]) 
{
    assert(argc == 1);
    assert(argv != NULL);

    if(argc == 1 && argv != NULL)
    {
        start();
    }
    return EXIT_SUCCESS;
}
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
void arrayFill( char (*array)[DIMENSION])
{
    for(int i = 0; i < DIMENSION; i++)
    {
        strcpy(array[i], "Nothing");
    }
}
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
void validateString(char const * const string )
{
  assert( NULL != string);                   // null test
  assert(!(strcmp(string, "") == 0));        // checking if string is empty
  assert ( '\0' == string[strlen(string)]);  // checking if null terminated
}
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
//clearShells the shell terminal screen
void clearShell()
{
  const char * clearShellScreen = "\e[1;1H\e[2J";
  write(STDOUT_FILENO, clearShellScreen, 12);
}
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
//the welcome portion of the shell
void initalizeShell()
{
    char * userName;
    time_t t     = time(NULL);
    struct tm tm = *localtime(&t);

    clearShell();


    printf("\n                      /\\");
    printf("\n                     /  \\");
    printf("\n                    /    \\");
    printf("\n                   /      \\");
    printf("\n               ===============");
    printf("\n                 (-*-   -*-)  ");
    printf("\n                      U      ");
    printf("\n                  \\-------/");
    printf("\n\n\n\n------------------------------------------");
    printf("\n******************************************");
    printf("\n\n\n     ****the one shell to rule them all****");
    printf("\n     ****  one shell to find them all  ****");
    printf("\n     ****  one shell to bring them all ****");
    printf("\n     ****and in the darkness bind them ****");
    printf("\n\n\t         -COMP 3430-");
    printf("\n\n\t        -Winter 2018-\n");
    printf("\n       Date: %d-%d-%d Time: %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    printf("\n******************************************");
    printf("\n------------------------------------------");
    printf("\nIf you would like to pre-load a sequence of commands, enter 'pre-load' as a command\n");

    userName = getenv("USER");
    printf("\n\n\nUSER is: @%s", userName);
    printf("\n");
    sleep(.4);
    //clearShell();
} 
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
void start()
{
    //-----------------------------------------------------------------------------------------------------------------------------//
    //new mapping in virtual address, data can be read, written to, and changes are shared
    char (*shellVariableNames)[DIMENSION] =  (char (*)[DIMENSION]) mmap(NULL, DIMENSION, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
    //if failed retruns the value of MAP_FAILED( which is (void*) - 1)
    if (shellVariableNames == MAP_FAILED) 
    {
        printf("Error mmapping shellVariableNames\n");
        EXIT_FAILURE;
    }
    else
    {
        arrayFill(shellVariableNames);
    }
    //-----------------------------------------------------------------------------------------------------------------------------// 
    //new mapping in virtual address, data can be read, written to, and changes are shared
    char (*shellVariableCommands)[DIMENSION] =  (char (*)[DIMENSION]) mmap(NULL, DIMENSION, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
    //if failed retruns the value of MAP_FAILED( which is (void*) - 1)
    if (shellVariableCommands == MAP_FAILED) 
    {
        printf("Error mmapping shellVariableCommands\n");
        EXIT_FAILURE;
    }
    else
    {
        arrayFill(shellVariableCommands);
    }
    //-----------------------------------------------------------------------------------------------------------------------------//
    char (*preLoadedCommands)[DIMENSION] =  (char (*)[DIMENSION]) mmap(NULL, DIMENSION, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
    //if failed retruns the value of MAP_FAILED( which is (void*) - 1)
    if (preLoadedCommands == MAP_FAILED) 
    {
        printf("Error mmapping shellVariableCommands\n");
        EXIT_FAILURE;
    }
    else
    {
        arrayFill(preLoadedCommands);
    }  
    initalizeShell();
    printf("@USER> ");

    userInput(shellVariableNames, shellVariableCommands, preLoadedCommands); 
}
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
void userInput(char (*shellVariableNames)[DIMENSION], char (*shellVariableCommands)[DIMENSION], char (*preLoadedCommands)[DIMENSION])
{
    pid_t  pid;
    char   line[MAX];
    char   copy[MAX];
    char * array[MAX]; 
    char * userName;
    char * token   = NULL;
    int    counter = 0;
    int    status;
    int    rc;

    userName = getenv("USER");
    
    while (fgets(line, MAX, stdin) != NULL)
    {
        strcpy(copy, line);
        pid = fork();

        //error 
        if (pid == -1) 
        {    
          printf("Error forking child, the process has failed\n");
          exit(1);
        }
        //parent process
        else if (pid != 0)
        { 
            //waiting for child to be finished
            while(wait(&status) != pid);
        }
        //child process
        else
        {
            //checking if pipe is being used
            if(strstr(copy, "|") != NULL)
            {
                pipedCommand(copy);
            }
            //checking if shell variable is being set by user
            else if(strstr(copy, "set") != NULL)
            {
                setShellVariable(copy, shellVariableNames, shellVariableCommands);
            }
            //or if pre-loading command is used
            else if(strstr(copy, "pre-load") != NULL)
            {
                preLoadCommands(preLoadedCommands);
            }
            //else just a regular command is being entered
            else
            {
                token = strtok(copy, DELIM);

                while(token != NULL) 
                {
                    array[counter++] = strdup(token);
                    token = strtok(NULL, DELIM);
                }

                array[counter] = NULL;
                
                rc = execvp(array[0], array);
                if (rc!=0) 
                {
                    perror("execv");
                    printf("Invalid command entered!\n");
                }
                counter = 0;
            } 
        }      
        printf("\r\n@%s>", userName);
    }
}
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
//function to set shell variable defined by person using the shell
void setShellVariable(const char * const arg, char (*shellVariableNames)[DIMENSION], char (*shellVariableCommands)[DIMENSION])
{
    validateString(arg);

    char    copy[MAX];
    char *  token1 = NULL;
    char *  token2 = NULL;
    Boolean flag   = FALSE;

    if (NULL != arg)
    {
        strcpy(copy, arg);
        strcpy(copy, strtok(copy, DELIM));

        token1 = strtok(NULL, DELIM);
        token2 = strtok(NULL, DELIM);

        printf("setting shell command: %s\n", arg);
        printf("Command name: %s\n", token1);
        printf("Command action: %s\n", token2);

        for(int i = 0; i < 10; i++)
        {
            //checking to see if command has been previously set by user
            if(strcmp(shellVariableNames[i], token1) == 0)
            {
                strcpy(shellVariableCommands[i], token2);
                flag = TRUE;
            }
        }

        // command not previously set by the shell user
        if(flag == FALSE)
        {
            for(int j = 0; j < 10; j++)
            {
                if(strcmp(shellVariableNames[j], "Nothing") == 0 && flag == FALSE)
                {
                    printf("checking for room: %s ,\n", shellVariableNames[j]);
                    strcpy(shellVariableNames[j]  , token1);
                    strcpy(shellVariableCommands[j], token2);
                    flag = TRUE;
                }
            }
            for(int k = 0; k < 10; k++)
            {
                printf("Command: %s , Name: %s \n", shellVariableCommands[k], shellVariableNames[k]);
            }
        }
    }
}
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
void preLoadCommands(char (*preLoadedCommands)[DIMENSION])
{
    char   line[MAX];
    int    counter = 0;
    FILE * file    = fopen(".shell_initx.txt", "r");

    assert(preLoadedCommands[0] != NULL);

    if(file == NULL)
    {
        perror("Error opening file");
        EXIT_FAILURE;
    }
    else
    {
        while(fgets(line, MAX, file ) != NULL)
        {
            printf("%s", line);
            strcpy(preLoadedCommands[counter]  , line);
            counter++;
        }
    }
}
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
void pipedCommand(char commands[MAX])
{
    pid_t  pidb;
    int    myPipefd[2];
    int    counter = 0;
    char   copy[MAX];
    char   buff[MAX]; 
    char * array[MAX];
    char * token = NULL;
    
    if(pipe(myPipefd) == -1)
    {
        perror("pipe Error");
        exit(1);
    } 

    pidb = fork();
    if (pidb == -1) 
    {    
        printf("Error forking child of child, the process has failed\n");
        exit(1);
    }


    //parent process(the original child that was forked), this child process pipes to the other child
    else if (pidb != 0)
    {
        close(myPipefd[0]);
        if(write(myPipefd[1], commands, 7) == -1)
        {
            perror("error");
            exit(1);
        }
    }
    //the child of the child, this process recives from the pipe
    else
    {
        close(myPipefd[1]);
        read(myPipefd[0], buff, MAX);

        strcpy(copy, buff);
        token = strtok(copy, "|");


        while(token) 
        {
            array[counter++] = token;
            printf("TOK: %s\n", token);

            //execl(token, "unknownprogram",NULL)
            token = strtok(NULL, "|");
        }
        printf("Command:%s Parameters: %s \n", array[0], array[1]);

        array[counter] = NULL;


        if (execvp(array[0], array) != 0) 
        {
            perror("execv");
            printf("Invalid command entered!\n");
        }
    }
}

