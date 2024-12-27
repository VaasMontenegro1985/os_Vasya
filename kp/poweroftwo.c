#include <stdio.h> 
#include <stdlib.h> 
#include <time.h>
#include <stddef.h> 
#include <math.h> 
 
typedef struct BlockPowOfTwo { 
    size_t size;        
    int is_free;        
    void* start_addr; 
    struct BlockPowOfTwo* next; 
} BlockPowOfTwo; 
 
typedef BlockPowOfTwo* Allocator; 

Allocator create_allocator(size_t memory_size) { 
    Allocator allocator = (BlockPowOfTwo*)malloc(memory_size);  
 
    allocator->size = memory_size; 
    allocator->is_free = 1; 
    allocator->start_addr = (void*)allocator; 
    allocator->next = NULL;
 
    return allocator; 
}


void* allocate_memory(Allocator allocator, size_t size) {
    size_t block_size = 1;
    if (size == 0) {
        return 0;
    }
    while (block_size < size) {
        block_size <<= 1;
    }  
    BlockPowOfTwo* current = allocator; 
 
    while (current) { 
        if (current->is_free && current->size >= block_size) { 
           
            if (current->size > block_size) { 
                
                BlockPowOfTwo* new_block = (BlockPowOfTwo*)((char*)current->start_addr + block_size); 
                new_block->size = current->size - block_size; 
                new_block->is_free = 1; 
                new_block->start_addr = (void*)new_block; 
                new_block->next = current->next; 
 
                current->size = block_size; 
                current->next = new_block; 
            } 
            current->is_free = 0;  
            return current->start_addr; 
        } 
        current = current->next; 
    } 
 
    return NULL;  
} 
 

void deallocate_memory(Allocator allocator, void* addr) { 
    BlockPowOfTwo* current = allocator; 
    while (current) { 
        if (current->start_addr == addr) { 
            current->is_free = 1;  
            return; 
        } 
        current = current->next; 
    } 
} 
 
void delete_allocator (Allocator allocator) { 
    free(allocator); 
} 
 
