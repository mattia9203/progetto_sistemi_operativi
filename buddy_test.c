#define _GNU_SOURCE
#include "buddy_allocator.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>

#define BUFFER_SIZE (1024*1024)           //1MB di memoria
#define NUM_LEVELS 16
#define MEMORY_SIZE 1<<14                 //memoria per bitmap                                               
#define MIN_BUCKET_SIZE (BUFFER_SIZE>>(NUM_LEVELS))

char buffer[BUFFER_SIZE];
char bitmap[MEMORY_SIZE];

BuddyAllocator alloc;

int main(int argc,char** argv){

    BuddyAllocator_init(&alloc,NUM_LEVELS,buffer,BUFFER_SIZE,bitmap,MEMORY_SIZE,MIN_BUCKET_SIZE);
    
    //proviamo ad allocare blocchi maggiori di 1mb o minori di 0
    printf("Proviamo ad allocare blocchi troppo grandi o negativi per verificare la condizione sulla size\n");
    void* p1 = BuddyAllocator_malloc(&alloc,10000000);

    printf("\n\n");

    void* p2 = BuddyAllocator_malloc(&alloc,-1);

    char* blocks[20];

    printf("\nProviamo a riempire la memoria del buddy, allochiamo 4 blocchi al livello 2\n");
    for (int i=0; i<4; i++){
        blocks[i] = BuddyAllocator_malloc(&alloc,200000);
    }

    printf("\n\n");

    printf("\nDeallochiamo i blocchi precedenti\n");
    for (int i=0; i<4; i++){
        BuddyAllocator_free(&alloc,blocks[i]);
    }

    printf("\n\n");

    printf("\nRiempiamo di nuovo la memoria allocando 8 blocchi al livello 3,con uno in piu\n");
    for (int i=0; i<9; i++){
        blocks[i] = BuddyAllocator_malloc(&alloc,100000);
    }

    printf("\n\n");

    printf("\nOra che la memoria è piena proviamo ad allocare piccoli blocchi\n");
    for (int i=9; i<12; i++){
        blocks[i] = BuddyAllocator_malloc(&alloc,20);
    }

    printf("\n\n");

    printf("\nFacciamo la free su questi piccoli blocchi per vedere se da problemi(dato che memoria in precedenza piena e quindi non realmente allocati)\n");
    for (int i=9; i<12; i++){
        BuddyAllocator_free(&alloc,blocks[i]);
    }

    printf("\n\n");

    printf("\nLiberiamo gli 8 blocchi di prima\n");
    for (int i=0; i<9; i++){
        BuddyAllocator_free(&alloc,blocks[i]);
    }

    printf("\n\n");

    printf("\nProviamo a liberarli nuovamente per verificare il corretto funzionamento del buddy in caso di double free\n");
    for (int i=0; i<9; i++){
        BuddyAllocator_free(&alloc,blocks[i]);
    }

    return 0;
}