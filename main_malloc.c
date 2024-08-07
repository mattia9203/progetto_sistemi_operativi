#define _GNU_SOURCE
#include "main_malloc.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>

void* pseudo_malloc(BuddyAllocator* alloc,int size){
    if (size <= 0){
        printf("INSERIRE SIZE MAGGIORE DI 0\n\n");
        return NULL;
    }
    int page_size = sysconf(_SC_PAGESIZE);
    if (size < 0.25*page_size){
        printf("\nALLOCAZIONE CON BUDDY ALLOCATOR\n");
        void* p = BuddyAllocator_malloc(alloc,size);
        if (!p){
            printf("\nMEMORIA BUDDY ALLOCATOR PIENA\n\n");
            return NULL;
        }
        printf("\nBLOCCO ALLOCATO CON BUDDY\n\n");
        return p;
    }else{
        printf("\nALLOCAZIONE CON MMAP\n");
        int mem_size = size + sizeof(int);                     //aggiungiamo lo spazio per mettere all'inizio del blocco la grandezza cosi da sapere quale free chiamare
        printf("\nRICHIESTA ALLOCAZIONE DI %d bytes\n",mem_size);
        void* p=mmap(NULL,mem_size,PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (p == MAP_FAILED){
            printf("\nERRORE MAP FAILED\n");
            return NULL;
        }
        ((int *)p)[0] = mem_size;
        printf("\nBLOCCO ALLOCATO CON MMAP : %p di dimensione %d\n\n",p+sizeof(int),mem_size);
        return (void *)(p+sizeof(int));
    }
}

void pseudo_free(BuddyAllocator* alloc,void** block){
    if (*block == NULL){
        printf("\nBLOCCO NON ALLOCATO\n\n");
        return;
    }
    //int page_size = sysconf(_SC_PAGESIZE);
    void* buffer_start=alloc->buf;
    void* buffer_end=alloc->buf+alloc->buf_size-1;                            //funzione di sistema che restituisce la dimensione della pagina di memoria in bytes
    if (*block >= buffer_start && *block <= buffer_end){
        BuddyAllocator_free(alloc,*block);
        *block = NULL;
        return;
    }else{
        int *p = (int*)(*block);
        int size = *(p - 1);
        printf("\nLIBERO BLOCCO CON MUNMAP %p di dimensione %d\n",p,size);
        int ret=munmap((void*)(p-1),(size_t)size);
        if (ret == 0){
            printf("\nBLOCCO LIBERATO\n\n");
            *block = NULL;
        }else{
            printf("\nBLOCCO NON LIBERATO : ERRORE MUNMAP\n\n");
        }
        return;
    }
}
