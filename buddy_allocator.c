#include "buddy_allocator.h"
#include <assert.h>
#include <stdio.h>
#include <math.h>

//definiamo alcune funzioni per gestire il buddy e muoverci tra gli indici
int levelIdx(size_t idx){
    return (int)floor(log2(idx+1));
}

int buddy(int idx){
    if (idx == 0) return 0;
    else if (idx%2) return idx+1;
    return idx-1;
}

int parent(int idx){
    return (int)(idx-1)/2;
}

int first(int level){
    return (1 << level)-1;        //ritorna il primo indice del livello level(2^levels -1)
}

int start(int idx){
    return (idx-(first(levelIdx(idx))));   //ritorna l'offset tra il nodo di indice idx e il primo indice del livello corrispondente
}

int is_power_of_two(int size) {
    return size && !(size & (size - 1));          //verifica che size è potenza di due (utile per funzione init)
}

void BuddyAllocator_init(BuddyAllocator* alloc, int num_levels, char* buf, int buf_size, char* bitmap_buf, int bitmap_buf_size, int min_bucket_size){
    
    alloc->num_levels = num_levels;
    alloc->buf=buf;
    alloc->min_bucket_size = min_bucket_size;
    

    int bits = (1 << (num_levels+1))-1;           //bit necessari per bitmap(=2^(num_levels+1))-1
    int bitmap_size = BitMap_getBytes(bits);

    assert("\nERRORE INIT: MEMORIA NON ABBASTANZA GRANDE PER CONTENERE BITMAP\n" && bitmap_buf_size>=BitMap_getBytes(bits));
    assert("\nERRORE INIT: BUFFER PUNTA A NULL\n" && buf);
    assert("\nERRORE INIT: BUF SIZE DEVE ESSERE >0\n" && buf_size>0);
    assert("\nERRORE INIT: MEMORIA PER LA BITMAP NULL\n" && bitmap_buf);

    if (!is_power_of_two(buf_size)){
        buf_size = min_bucket_size << num_levels;       //se dimensione non è potenza di due allora non usiamo tutta la memoria ma solo buf_size = min_bucket_size * 2^num_levels
    }

    alloc->buf_size = buf_size;
    BitMap_init(&alloc->bitmap,bits,bitmap_buf);

    printf("\nINIZIALIZZAZIONE BUDDY ALLOCATOR\n");
    printf("\nMemoria gestita dal Buddy Allocator: %d bytes",alloc->buf_size);
    printf("\nBit usati per la Bitmap : %d\n",bits);
    printf("\nMemoria usata per la BitMap: %d bytes\n",bitmap_size);
    printf("\nNumero di livelli del buddy e dimensione del blocco minimo : %d e %d\n",alloc->num_levels,alloc->min_bucket_size);

}

void* BuddyAllocator_malloc(BuddyAllocator* alloc,int size){
    if (size <= 0){
        printf("INSERIRE SIZE MAGGIORE DI 0\n");
        return NULL;
    }

    if (size+2*sizeof(int) > alloc->buf_size) {
        printf("NON ABBASTANZA MEMORIA PER SODDISFARE LA RICHIESTA DI %d bytes\n",size);
        return NULL;
    }

    
    size+=sizeof(int);               //bisogna tener conto dei byte usati per l'indice della bitmap

    int level =(int) (floor(log2(alloc->buf_size / size)));
    if (level > alloc->num_levels ) level = alloc->num_levels;        //se livello troppo profondo assegnamo il massimo

    printf("\nRICHIESTA ALLOCAZIONE DI %d bytes ,LIVELLO DEL BLOCCO : %d\n",size,level);

    //ora bisogna trovare un blocco libero utilizzando la bitmap, facciamo prima il caso in cui il livello sia il primo cosi da avere un solo indice da controllare
    int idx_free=-1;

    if (level == 0){
        if (!BitMap_getBit(&alloc->bitmap,0)){ 
            idx_free = 0;
            printf("INDICE LIBERO : %d\n",idx_free);}
    } else{
        //se il livello non è il primo allora avremo piu indici per quel livello e quindi ci servirà un ciclo per controllarli
        for (int i = first(level); i < first(level+1); i++){
            if (!BitMap_getBit(&alloc->bitmap,i)){
                idx_free = i;
                printf("INDICE LIBERO : %d\n",idx_free);
                break;
            }
        }
    }
    if (idx_free == -1){
        printf("NESSUN BLOCCO LIBERO TROVATO AL LIVELLO %d\n",level);
        return NULL;}

    //prepariamo l'indirizzo da restituire
    int dim_lvl = alloc->min_bucket_size * (1 << (alloc->num_levels - levelIdx(idx_free)));         //calcola la dimensione dei blocchi nel livello che ci interessa
    char* block = alloc->buf + start(idx_free) * dim_lvl;    
    //come visto a lezione, invece di passare soltanto l'indirizzo passiamo anche l'indice e la dimemsione del blocco, che ci sarà utile nella free
    ((int *)block)[0] = idx_free;
    //((int *)block)[1] = size;
    

    //ora aggiorniamo la bitmap settando a 1 tutti i successori e i predecessori dell'indice
    set_successors_and_predecessors (&alloc->bitmap,idx_free,1,1);     //successori
    set_successors_and_predecessors (&alloc->bitmap,idx_free,1,0);     //predecessori
    printf("\nALLOCATO BLOCCO DI MEMORIA %p DI DIMENSIONE %d bytes AL LIVELLO %d CON INDICE BITMAP %d\n",block+sizeof(int),dim_lvl,level,idx_free);
    
    return (void*) (block + sizeof(int));
}

void BuddyAllocator_free(BuddyAllocator* alloc, void* block){
    if (block == NULL){
        printf("\nBLOCCO NON ALLOCATO\n");
        return;
    }
    int *p = (int*)block;
    p--;
    int idx_free = *p;
    int size = alloc->min_bucket_size * (1 << (alloc->num_levels - levelIdx(idx_free)));         //calcola la dimensione dei blocchi nel livello che ci interessa
    printf("\nLIBERO BLOCCO COL BUDDY ALLOCATOR:%p di dimensione %d\n",block,size);
    //possiamo ora ritrovare l'indice avendolo inserito in precedenza prima dell'indirizzo
    if (BitMap_getBit(&alloc->bitmap,idx_free) == 0){                      //controllo sul double free
        printf("\nINDICE LIBERO : DOUBLE FREE\n");
        return;
    }
    printf("\nBLOCCO DA LIBERARE: %p E INDICE BITMAP DA LIBERARE: %d\n",block,idx_free);
    
    BitMap_setBit(&alloc->bitmap,idx_free,0);
    //ora settiamo a 0 i discendenti che avevamo settato a 1 quando abbiamo rilasciato il blocco
    set_successors_and_predecessors(&alloc->bitmap,idx_free,0,1);
    release_mem(&alloc->bitmap,idx_free);       //funzione in cui controlliamo se buddy libero cosi da unirli e farlo ricorsivamente fino al livello più alto possibile
    printf("\nBLOCCO LIBERATO\n\n");

    return;
}

void set_successors_and_predecessors(BitMap* bitmap,int idx,int value,int succ){
    if (succ == 0){
        BitMap_setBit(bitmap,idx,value);
        if (idx != 0) set_successors_and_predecessors(bitmap,parent(idx),value,succ);       //se non è root facciamo la ricorsione per gli i predecessori
    } else {
        if (idx < bitmap->num_bits){
            BitMap_setBit(bitmap,idx,value);
            set_successors_and_predecessors(bitmap,idx * 2 + 1,value,succ);      //figlio sx
            set_successors_and_predecessors(bitmap,idx * 2 + 2,value,succ);      //figlio dx
        }
    }
}

void release_mem(BitMap* bitmap,int idx){
    if (idx == 0) return;                        //root non ha il buddy
    int buddy_idx = buddy(idx);                    
    if (!BitMap_getBit(bitmap,buddy_idx)){         //buddy libero?
        printf("\nBUDDY DI %d : %d LIBERO\n",idx,buddy_idx);
        printf("\nDATO CHE BUDDY LIBERO ESEGUIAMO MERGE AL LIVELLO %d\n",levelIdx(idx));
        BitMap_setBit(bitmap,parent(idx),0);
        release_mem(bitmap,parent(idx));
    }else{
        printf("\nBUDDY DI %d : %d NON LIBERO, QUINDI NIENTE OPERAZIONE DI MERGE\n",idx,buddy_idx);
        return;
    }
}
