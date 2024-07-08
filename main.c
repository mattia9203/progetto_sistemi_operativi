#define _GNU_SOURCE
#include "buddy_allocator.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>

#define BUFFER_SIZE (1024*1024)           //1MB di memoria
#define NUM_LEVELS 18
#define MEMORY_SIZE 128000                 //memoria per bitmap                                               
#define MIN_BUCKET_SIZE (BUFFER_SIZE>>(NUM_LEVELS))

char buffer[BUFFER_SIZE];
char bitmap[MEMORY_SIZE];

size_t page_size;
BuddyAllocator alloc;

void* pseudo_malloc(size_t size){
    long page_size = sysconf(_SC_PAGESIZE);
    if (size <= 0.25*page_size){
        printf("\nALLOCAZIONE CON BUDDY ALLOCATOR\n");
        void* p = BuddyAllocator_malloc(&alloc,size);
        if (!p){
            printf("MEMORIA BUDDY ALLOCATOR PIENA");
            return NULL;
        }
        printf("\nBLOCCO ALLOCATO CON BUDDY : %p di dimensione %d\n",p,size);
        return p;
    }else{
        printf("\nALLOCAZIONE CON MMAP\n");
        int mem_size = size + sizeof(int);                     //aggiungiamo lo spazio per mettere all'inizio del blocco la grandezza cosi da sapere quale free chiamare
        void* p=mmap(NULL,mem_size,PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (p == MAP_FAILED){
            printf("\nERRORE MAP FAILED\n");
            return NULL;
        }
        ((int *)p)[0] = mem_size;
        printf("\nBLOCCO ALLOCATO CON MMAP : %p di dimensione %d\n\n",p+sizeof(int),size);
        return (void *)(p+sizeof(int));
    }
}

void pseudo_free(void** block){
    if (*block == NULL){
        printf("\nBLOCCO NON ALLOCATO\n");
        return;
    }
    int *p = (int*)(*block);
    int size = *(p - 1);
    if (size <= 0.25*page_size){
        printf("\nLIBERO BLOCCO COL BUDDY ALLOCATOR:%p di dimensione %d\n",block,size);
        BuddyAllocator_free(&alloc,*block);
        *block = NULL;
        return;
    }else{
        printf("\nLIBERO BLOCCO CON MUNMAP %p di dimensione %d\n",p,size);
        int ret=munmap((void*)(p-1),(size_t)size);
        if (ret == 0){
            printf("\nBLOCCO LIBERATO\n\n");
            *block = NULL;
        }else{
            perror("munmap");
            printf("\nBLOCCO NON LIBERATO\n\n");
        }
        return;
    }
}

int main(int argc,char** argv){

    BuddyAllocator_init(&alloc,NUM_LEVELS,buffer,BUFFER_SIZE,bitmap,MEMORY_SIZE,MIN_BUCKET_SIZE);
    
    page_size = sysconf(_SC_PAGE_SIZE);
    if (page_size == -1){
        perror("sysconf");
        return NULL;
    }

    void* p1=pseudo_malloc(1000);
    void* p2=pseudo_malloc(5000);
    pseudo_free(&p1);
    if (p1 == NULL) printf("p1 null\n");
    pseudo_free(&p2);
    if (p2 == NULL) printf("p2 null\n");

    void* p3=pseudo_malloc(1000);

    pseudo_free(&p1);
    pseudo_free(&p2);
    pseudo_free(&p3);

    return 0;
}