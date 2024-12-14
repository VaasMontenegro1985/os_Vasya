#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <fcntl.h> 
#include <unistd.h> 
#include <sys/mman.h> 
#include <sys/wait.h> 
#include <pthread.h> 
#include "shared.h" 

int main() { 

    int shm_fd = shm_open(SHARED_SEG_NAME, O_CREAT | O_RDWR, 0644);  
    if (shm_fd == -1) {  
        perror("shm_open");  
        exit(EXIT_FAILURE);  
    } 

    if (ftruncate(shm_fd, SHM_SIZE) == -1) {  
        perror("ftruncate");  
        exit(EXIT_FAILURE);  
    } 

    SharedData *shared_data = (SharedData *)mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);  
    if (shared_data == MAP_FAILED) {  
        perror("mmap");  
        exit(EXIT_FAILURE);  
    } 

    pthread_mutexattr_t mutex_attr;  
    pthread_mutexattr_init(&mutex_attr);  
    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);  
    pthread_mutex_init(&shared_data->mutex, &mutex_attr); 

    printf("Input filename: ");  
    scanf("%s", shared_data->filename); 

    pid_t pid = fork(); 

    if (pid == -1) {  
        perror("fork");  
        exit(EXIT_FAILURE);  
    } 

    if (pid == 0) {  
        char *const *null = NULL;  
        if (execv("./child.out", null) == -1) {  
            perror("execv error");  
            return 1;  
        }  
    } else {  
        sleep(1); 

        shared_data->sequenceNumber = 0; 

        while (1) {
            pthread_mutex_lock(&shared_data->mutex);  
            printf("Input number: ");  
            scanf("%d", &->numbeshared_datar);  
            shared_data->sequenceNumber++;  
            pthread_mutex_unlock(&shared_data->mutex); 

            sleep(1);

            pthread_mutex_lock(&shared_data->mutex);  
            int errorCode = shared_data->errorCode;  
            pthread_mutex_unlock(&shared_data->mutex); 

            if (errorCode == 1) {    
                printf("Writing is done!!!"); 
                wait(NULL);  
                pthread_mutex_destroy(&shared_data->mutex);  
                munmap(shared_data, SHM_SIZE);  
                shm_unlink("/shared_memory");  
                exit(EXIT_SUCCESS);  
            }  
        }  
    } 
}