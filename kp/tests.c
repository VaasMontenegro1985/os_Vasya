#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "poweroftwo.c"
#include "free_blocks.c"
void testPowerOfTwoAllocatorPerformance() 
{ 
    size_t allocator_memory_size = 1024;
    Allocator allocator = create_allocator(allocator_memory_size); 
 
    printf("~~~ Power Of Two Allocator ~~~\n"); 
 
    clock_t start = clock(); 
    size_t used_memory = 0; 
    int memory_size = 0; 
    void *blocks[10]; 
    int block_size = 128; 
    for (int i = 0; i < 10; i++) 
    { 
        blocks[i] = allocate_memory(allocator, 100); 
        printf("%p\n", (char*)blocks[i]); 
        if (blocks[i]) { 
            memory_size += block_size; 
            used_memory += 100;} 
    } 
 
    clock_t end = clock(); 
 
    double utilization = (double)used_memory / memory_size; 
    printf("Memory Utilization: %.2f%%\n", utilization * 100); 
    double allocation_time = (double)(end - start) / CLOCKS_PER_SEC; 
    printf("Allocation Speed: %.6f seconds\n", allocation_time); 
 
    start = clock(); 
    for (int i = 0; i < 10; i++) 
    { 
        if (blocks[i]) 
            deallocate_memory(allocator, blocks[i]); 
    } 
    end = clock(); 
    double deallocation_time = (double)(end - start) / CLOCKS_PER_SEC; 
    printf("Deallocation Speed: %.6f seconds\n", deallocation_time); 
 
    delete_allocator(allocator); 
    printf("Buddy Allocator Test Complete.\n"); 
} 

void testFreeListAllocatorPerformance()
{
    size_t memory_size = 1024;
    void *memory = malloc(memory_size);

    FreeListAllocator *freeListAllocator = createMemoryAllocator(memory, memory_size);

    printf("~~~ Free List Allocator ~~~\n");

    size_t used_memory = 0;
    void *blocks[10];
    for (int i = 0; i < 10; i++)
    {
        blocks[i] = alloc(freeListAllocator, 100);
        
        printf("%p\n", (char*)blocks[i]); 
        if (blocks[i])
            used_memory += 100;
    }

    double utilization = (double)used_memory / memory_size;
    printf("Memory Utilization: %.2f%%\n", utilization * 100);

    clock_t start = clock();
    for (int i = 0; i < 1000; i++)
    {
        void *ptr = alloc(freeListAllocator, 64);
        if (ptr)
            freeBlock(freeListAllocator, ptr);
    }
    clock_t end = clock();
    double allocation_time = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Allocation Speed: %.6f seconds\n", allocation_time);

    start = clock();
    for (int i = 0; i < 10; i++)
    {
        if (blocks[i])
            freeBlock(freeListAllocator, blocks[i]);
    }
    end = clock();
    double deallocation_time = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Deallocation Speed: %.6f seconds\n", deallocation_time);

    free(memory);
    printf("Free List Allocator Test Complete.\n");
}