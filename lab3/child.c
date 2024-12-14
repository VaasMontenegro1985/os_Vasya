#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <fcntl.h> 
#include <unistd.h> 
#include <sys/mman.h> 
#include <pthread.h> 
#include <stdbool.h> 
#include "shared.h" 

int isPrime (int n) {   
   if (n <= 3) return 1;   
   
   for (int i = 2; i * i <= n; i++) {   
       if (n % i == 0) return 0;   
   }   
  
   return 1;   
}   
int main() { 
   int shm_fd = shm_open(SHARED_SEG_NAME, O_RDWR, 0644); 
   if (shm_fd == -1) { 
       perror("shm_open"); 
       exit(EXIT_FAILURE); 
   } 

   SharedData *shared_data = (SharedData *)mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); 
   if (shared_data == MAP_FAILED) { 
       perror("mmap"); 
       exit(EXIT_FAILURE); 
   } 

   int commitN = 0; 

   pthread_mutex_lock(&shared_data->mutex); 
   int file = open(shared_data->filename, O_CREAT | O_WRONLY | O_TRUNC, 0666); 
   pthread_mutex_unlock(&shared_data->mutex); 

   if (file == -1) { 
       perror("open"); 
       exit(EXIT_FAILURE); 
   } 

   sleep(1); 

   while (1) { 
       pthread_mutex_lock(&shared_data->mutex); 
       int number = shared_data->number; 
       if (number >= 0 && !isPrime(number)) { 
           if (commitN != shared_data->sequenceNumber) { 
               commitN = shared_data->sequenceNumber; 

               char str[100]; 
               sprintf(str, "%d\n", number); 

               write(file, str, strlen(str)); 
           } 
            
           shared_data->errorCode = 0;
       } else { 
           shared_data->errorCode = 1;
           close(file); 
           pthread_mutex_unlock(&shared_data->mutex); 
           munmap(shared_data, SHM_SIZE); 
           exit(EXIT_SUCCESS); 
       } 
       pthread_mutex_unlock(&shared_data->mutex); 
       sleep(1); 
   } 

   munmap(shared_data, SHM_SIZE); 
   close(file); 
   exit(EXIT_SUCCESS); 
}