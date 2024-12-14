#pragma once 
#include <pthread.h> 
 
#define SHM_SIZE 1024 
#define FILENAME_SIZE 30 
#define BUFFER_SIZE 30 
#define SHARED_SEG_NAME "/shared_memory" 
 
typedef struct { 
    int number; 
    char filename[FILENAME_SIZE]; 
    int sequenceNumber; 
    int errorCode; 
    pthread_mutex_t mutex; 
} SharedData;