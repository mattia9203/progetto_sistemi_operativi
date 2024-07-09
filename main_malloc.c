#define _GNU_SOURCE
#include "main_malloc.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>

void* pseudo_malloc(BuddyAllocator* alloc,int size){
    int page_size = sysconf(_SC_PAGESIZE);
    if (size <= 0.25*page_size){
        printf("\nALLOCAZIONE CON BUDDY ALLOCATOR\n");
        void* p = BuddyAllocator_malloc(alloc,size);
        if (!p){
            printf("MEMORIA BUDDY ALLOCATOR PIENA");
            return NULL;
        }
        printf("\nBLOCCO ALLOCATO CON BUDDY : %p di dimensione %d\n",p,(int)size);
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
        printf("\nBLOCCO ALLOCATO CON MMAP : %p di dimensione %d\n\n",p+sizeof(int),(int)size);
        return (void *)(p+sizeof(int));
    }
}

void pseudo_free(BuddyAllocator* alloc,void** block){
    if (*block == NULL){
        printf("\nBLOCCO NON ALLOCATO\n");
        return;
    }
    int page_size = sysconf(_SC_PAGESIZE);
    int *p = (int*)(*block);
    int size = *(p - 1);
    if (size <= 0.25*page_size){
        printf("\nLIBERO BLOCCO COL BUDDY ALLOCATOR:%p di dimensione %d\n",block,size);
        BuddyAllocator_free(alloc,*block);
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
