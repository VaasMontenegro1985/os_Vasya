#include <string.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <fcntl.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
 
int main () { 
    char filename[30]; 
    printf("Input filename: "); 
    scanf("%s", filename); 
 
    int pipe1[2];   
    int pipe2[2];  
 
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) { 
        perror("pipe"); 
        exit(EXIT_FAILURE); 
    } 

    pid_t pid = fork(); 

    if (pid == -1) { 
        perror("fork"); 
        exit(EXIT_FAILURE); 
    } 

    if (pid == 0) { 
        close(pipe1[1]); 
        close(pipe2[0]); 

        if (dup2(pipe1[0], 0) == -1) { 
            perror("dup2"); 
            exit(EXIT_FAILURE); 
        } 

        if (dup2(pipe2[1], 2) == -1) { 
            perror("dup2"); 
            exit(EXIT_FAILURE); 
        } 

        char *const *null = NULL; 
        if (execv("./child.out", null) == -1) { 
            perror("execv error"); 
            return 1; 
        } 
    } else { 
        close(pipe1[0]); 
        close(pipe2[1]); 
    
        if (write(pipe1[1], filename, strlen(filename) + 1) == -1) { 
            perror("write"); 
            exit(EXIT_FAILURE); 
        } 
        
        while (1) { 
            int number; 
            scanf("%d", &number); 
 
            if (write(pipe1[1], &number, sizeof(number)) == -1) { 
                perror("write"); 
                exit(EXIT_FAILURE); 
            } 
 
            int errorCode; 
 
            if (read(pipe2[0], &errorCode, sizeof(errorCode)) == -1) { 
                perror("read"); 
                exit(EXIT_FAILURE); 
            } 
 
            if (errorCode == 1) break; 
        } 

        printf("Writing is done!!!\n");
        close(pipe1[1]); 
        close(pipe2[0]); 
 
        exit(EXIT_SUCCESS); 
    } 
}