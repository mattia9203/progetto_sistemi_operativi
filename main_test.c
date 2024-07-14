#define _GNU_SOURCE
#include <stdio.h>
#include "main_malloc.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>


#define BUFFER_SIZE (1024*1024)           //1MB di memoria
#define NUM_LEVELS 17
#define PLUS_BITMAP (((1 << (NUM_LEVELS+1))-1)%8 != 0 ? 1 : 0)           //2^levels+1 -1 %8 != 0 allora 1 senno 0
#define MEMORY_SIZE (((1 << (NUM_LEVELS+1))-1)/8)+PLUS_BITMAP           //(2^levels+1 -1 )/8 + plus, memoria bitmap                                              
#define MIN_BUCKET_SIZE (BUFFER_SIZE>>(NUM_LEVELS))        //2^20 / 2^16 = 2^4 

char buffer[BUFFER_SIZE];
char bitmap[MEMORY_SIZE];

BuddyAllocator alloc;

int main(int argc,char** argv){
    void* blocks[20];

    BuddyAllocator_init(&alloc,NUM_LEVELS,buffer,BUFFER_SIZE,bitmap,MEMORY_SIZE,MIN_BUCKET_SIZE);

    printf("Proviamo ad allocare blocchi a dimensione 0 o negativa\n\n");
    void* p1=pseudo_malloc(&alloc,-1);
    void* p2=pseudo_malloc(&alloc,0);

    printf("\n\n");

    printf("\nProviamo a deallocarli anche se l'allocazione non è andata a buon fine\n\n");
    pseudo_free(&alloc,&p1);
    pseudo_free(&alloc,&p2);

    printf("\n\n");

    printf("\nProviamo ad allocare blocchi variabili per vedere se vengono chiamati in modo corretto il buddy e l'mmap\n\n");
    int j=-1;
    for (int i=1; i<5002; i+=1000){
        j++;
        blocks[j] = pseudo_malloc(&alloc,i);
    }

    printf("\n\n");

    printf("Ora deallochiamoli al contrario\n\n");
    while(j >= 0){
        pseudo_free(&alloc,&blocks[j]);
        j--;
    }
    
    printf("\n\n");

    printf("Ora proviamo a deallocarli nuovamente ma nell'ordine corretto\n\n");
    while (j < 5){
        pseudo_free(&alloc,&blocks[j]);
        j++;
    }

    printf("\n\n");

    printf("Proviamo ad allocare con mmap i blocchi dell'array blocks in cui prima avevamo allocato col buddy e viceversa\n\n");
    j=-1;
    for (int i=1; i<5002; i+=1000){
        j++;
        blocks[j] = pseudo_malloc(&alloc,5002-i);
    }

    printf("\n\n");
    
    printf("Ora deallochiamoli\n\n");
    while(j >= 0){
        pseudo_free(&alloc,&blocks[j]);
        j--;
    }


    return 0;
}