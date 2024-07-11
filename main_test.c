#define _GNU_SOURCE
#include <stdio.h>
#include "main_malloc.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>


#define BUFFER_SIZE (1024*1024)           //1MB di memoria
#define NUM_LEVELS 17
#define MEMORY_SIZE 1<<15                 //memoria per bitmap                                               
#define MIN_BUCKET_SIZE (BUFFER_SIZE>>(NUM_LEVELS))        //2^20 - 2^16 = 2^4 

char buffer[BUFFER_SIZE];
char bitmap[MEMORY_SIZE];

BuddyAllocator alloc;

int main(int argc,char** argv){
    void* blocks[20];

    BuddyAllocator_init(&alloc,NUM_LEVELS,buffer,BUFFER_SIZE,bitmap,MEMORY_SIZE,MIN_BUCKET_SIZE);

    printf("Proviamo ad allocare blocchi a dimensione 0 o negativa\n\n");
    void* p1=pseudo_malloc(&alloc,-1);
    void* p2=pseudo_malloc(&alloc,0);
    printf("\nProviamo a deallocarli anche se l'allocazione non è andata a buon fine\n\n");
    pseudo_free(&alloc,&p1);
    pseudo_free(&alloc,&p2);

    printf("\nProviamo ad allocare blocchi variabili per vedere se vengono chiamati in modo corretto il buddy e l'mmap\n\n");
    int j=0;
    for (int i=1; i<5000; i+=1000){
        blocks[j++] = pseudo_malloc(&alloc,i);
    }
    printf("Ora deallochiamoli al contrario\n\n");
    while(j >= 0){
        pseudo_free(&alloc,&blocks[--j]);
    }
    printf("Ora riproviamo a deallocarli nuovamente ma in ordine giusto\n\n");
    while (j < 5){
        pseudo_free(&alloc,&blocks[j++]);
    }


    return 0;
}