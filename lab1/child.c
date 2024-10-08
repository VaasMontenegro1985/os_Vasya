#include <stdio.h> 
#include <string.h>
#include <stdlib.h> 
#include <fcntl.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h> 

int isPrime (int n) { 
    if (n <= 3) return 1; 
   
    for (int i = 2; i * i <= n; i++) { 
        if (n % i == 0) return 0; 
    } 
 
    return 1; 
} 

int main () { 
    char filename[30]; 
    char letter; 
    int i = 0; 

    while (read(0, &letter, sizeof(letter))) { 
        filename[i++] = letter; 
        if (letter == '\0') break; 
    } 

    FILE* file = fopen(filename, "w"); 

    while (1) { 
        int number; 
        int errorCode; 

        if (read(0, &number, sizeof(number)) == -1) { 
            perror("read"); 
            exit(EXIT_FAILURE); 
        } 

        if (number < 0 || isPrime(number)) errorCode = 1; 
        else { 
            fprintf(file, "%d\n", number); 
            errorCode = 0; 
        } 
 
        if (write(2, &errorCode, sizeof(errorCode)) == -1) { 
            perror("write"); 
            exit(EXIT_FAILURE); 
        } 

        if (errorCode) break; 
    } 
    
    fclose(file); 
    exit(EXIT_SUCCESS); 
} 