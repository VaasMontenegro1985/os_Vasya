#include <stdio.h>
#include "tests.c"

int main()
{
    printf("Testing Free List Allocator\n");
    testFreeListAllocatorPerformance();
    printf("Testing Power Of Two Allocator\n");
    testPowerOfTwoAllocatorPerformance();
    
    return 0;
}