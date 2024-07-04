#include "Bitmap.h"

#define MAX_LEVELS 16

typedef struct {
    BitMap bitmap;        //bitmap del buddy allocator
    int num_levels;       //numero livelli dell'albero
    char* buf;         //buffer di memoria da gestire
    int buf_size;      //dimensione memoria
    int min_bucket_size;  //dimensione minima pagina
} BuddyAllocator ;


void BuddyAllocator_init(BuddyAllocator* alloc, int num_levels, char* buf, 
                        int buf_size, char* bitmap_buf, int bitmap_buf_size, 
                        int min_bucket_size);
 
void* BuddyAllocator_malloc(BuddyAllocator* alloc, int size);

void BuddyAllocator_free(BuddyAllocator* alloc,void* mem);

void set_successors_and_predecessors(BitMap* bitmap,int idx, int value,int succ);    //setta i successori se succ=1 e i predecessori su succ=0 di num_bit a value

void release_mem(BitMap* bitmap,int idx_free);         //funzione che libera la memoria controllando se buddy libero così da riunire i buddy 
