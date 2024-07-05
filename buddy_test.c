#include "buddy_allocator.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE (1024*1024)           //1MB di memoria
#define NUM_LEVELS 17
#define MEMORY_SIZE 64000                 //memoria per bitmap                                               
#define MIN_BUCKET_SIZE (BUFFER_SIZE>>(NUM_LEVELS))

char buffer[BUFFER_SIZE];
char bitmap[MEMORY_SIZE];

BuddyAllocator alloc;
int main(int argc,char** argv){



    BuddyAllocator_init(&alloc,NUM_LEVELS,buffer,BUFFER_SIZE,bitmap,MEMORY_SIZE,MIN_BUCKET_SIZE);
    printf("BUDDY ALLOCATOR INIZIALIZZATO\n");
    void *p1=BuddyAllocator_malloc(&alloc,128000);
    void *p2=BuddyAllocator_malloc(&alloc,100);
    void *p3=BuddyAllocator_malloc(&alloc,200000);
    void *p4=BuddyAllocator_malloc(&alloc,512000);
    BuddyAllocator_free(&alloc,p1);
    BuddyAllocator_free(&alloc,p2);
    BuddyAllocator_free(&alloc,p3);
    BuddyAllocator_free(&alloc,p4);
 
    return 0;
}