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
    else if (idx&0x1) return idx+1;
    return idx-1;
}

int parent(int idx){
    return (int)(idx-1)/2;
}

int first(int level){
    return (1 << level)-1;        //ritorna il primo indice del livello level
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

    assert("MEMORIA NON ABBASTANZA GRANDE PER CONTENERE BITMAP" && bitmap_buf_size>=BitMap_getBytes(bits));

    if (!is_power_of_two(buf_size)){
        buf_size = min_bucket_size << num_levels;       //se dimensione non è potenza di due allora non usiamo tutta la memoria ma solo buf_size = min_bucket_size * 2^num_levels
    }

    alloc->buf_size = buf_size;
    BitMap_init(&alloc->bitmap,bits,bitmap_buf);

    printf("\nINIZIALIZZAZIONE BUDDY ALLOCATOR\n");
    printf("\nMemoria gestita dal Buddy Allocator: %d bytes",alloc->buf_size);
    printf("\nBit usati per la Bitmap : %d\n",bits);
    printf("\nMemoria per la BitMap: %d bytes\nMemoria avanzata per la BitMap: %d bytes\n",bitmap_size,(bitmap_buf_size-bitmap_size));
    printf("\nNumero di livelli del buddy e dimensione del min bucket : %d e %d\n",alloc->num_levels,alloc->min_bucket_size);

}

void* BuddyAllocator_malloc(BuddyAllocator* alloc,int size){
    if (size == 0){
        printf("INSERIRE SIZE DIVERSA DA 0\n");
        return NULL;
    }

    if (size+2*sizeof(int) > alloc->buf_size) {
        printf("NON ABBASTANZA MEMORIA PER SODDISFARE LA RICHIESTA DI %d bytes\n",size);
    }

    
    size+=2*sizeof(int);               //bisogna tener conto dei byte usati per l'indice della bitmap

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
            printf (" %d ",i);
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
    int dim_lvl = alloc->min_bucket_size * (1 << (alloc->num_levels - levelIdx(idx_free)));
    char* block = alloc->buf + start(idx_free) * dim_lvl;    //(min_bucket_size << level) calcola la dimensione dei blocchi in un livello
    //come visto a lezione, invece di passare soltanto l'indirizzo passiamo anche l'indice che ci sarà utile nella free
    ((int *)block)[0] = idx_free;
    ((int *)block)[1] = size;
    

    //ora aggiorniamo la bitmap settando a 1 tutti i successori e i predecessori dell'indice
    set_successors_and_predecessors (&alloc->bitmap,idx_free,1,1);     //successori
    set_successors_and_predecessors (&alloc->bitmap,idx_free,1,0);     //predecessori
    printf("\nALLOCATO BLOCCO DI MEMORIA DI DIMENSIONE %d bytes AL LIVELLO %d CON INDICE BITMAP %d\n",size,level,idx_free);
    printf("\nSTART : %p\n\n",block+2*sizeof(int));

    //Bitmap_print(&alloc->bitmap);

    //return (void*)(block+sizeof(int));
    return (void*) (block + sizeof(int));

}

void BuddyAllocator_free(BuddyAllocator* alloc, void* block){
    if (block == NULL){
        printf("\nBLOCCO NON ALLOCATO\n");
        return;
    }
    //possiamo ora ritrovare l'indice avendolo inserito in precedenza prima dell'indirizzo
    int *p = (int*)block;
    int idx_free =p[-1];
    
    printf("\nINDICE DA LIBERARE %d\n",idx_free);
    //bisogna verificare che il blocco sia stato rilasciato dal buddy vedendo se puntatore allineato
    //controlliamo anche il caso in cui si faccia la free su un blocco libero cosi da evitare double free
    if (BitMap_getBit(&alloc->bitmap,idx_free) == 0){
        printf("\nINDICE LIBERO : DOUBLE FREE\n");
        return;
    }

    BitMap_setBit(&alloc->bitmap,idx_free,0);
    //ora settiamo a 0 i discendenti che avevamo settato a 1 quando abbiamo rilasciato i blocco
    set_successors_and_predecessors(&alloc->bitmap,idx_free,0,1);
    release_mem(&alloc->bitmap,idx_free);       //funzione in cui controlliamo se buddy libero cosi da unirli e farlo ricorsivamente fino al livello più alto possibile
    printf("\nBLOCCO LIBERATO\n\n");

    //Bitmap_print(&alloc->bitmap);

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
    if (idx == 0) return;    
    int buddy_idx = buddy(idx);                    //root
    if (!BitMap_getBit(bitmap,buddy_idx)){         //buddy libero?
        printf("\nBUDDY DI %d : %d LIBERO\n",idx,buddy_idx);
        printf("\nDATO CHE BUDDY LIBERO ESEGUIAMO MERGE AL LIVELLO %d\n",levelIdx(idx));
        BitMap_setBit(bitmap,parent(idx),0);
        release_mem(bitmap,parent(idx));
    }else printf("\nBUDDY DI %d : %d NON LIBERO, QUINDI NIENTE OPERAZIONE DI MERGE\n",idx,buddy_idx);

}

void Bitmap_print(BitMap *bit_map){
    int remain_to_print = 0;
    int lvl = -1; 
    int tot = levelIdx(bit_map->num_bits) - 1;  //numero di livelli totale
    for (int i = 0; i < bit_map->num_bits; i++){  
        if (remain_to_print == 0){ //se non rimangono bit da stampare al livello lvl
            if(lvl==tot){ //se siamo arrivati all'ultimo livello stop
              break;
            } 
            printf("\n\033[93mLivello %d: \t\033[0m", ++lvl);     //indice del primo elemento del livello: i
            for (int j = 0; j < (1 << tot) - (1 << lvl); j++){   //stampa degli spazi dopo aver scritto "Livello x:"
              printf(" "); //stampa spazi
            } 
            remain_to_print = 1 << lvl; //al prossimo livello dovremo stampare 2^lvl bit
        }
        if (BitMap_getBit(bit_map, i)==0){ //se il blocco è 0 lo stampiamo verde
          printf("\033[32m%d\033[0m ", BitMap_getBit(bit_map, i));
        }
        else{   //altrimenti lo stampiamo rosso
          printf("\033[31m%d\033[0m ", BitMap_getBit(bit_map, i));
        }
        remain_to_print--;  //1 bit in meno da stampare
    }
    printf("\n");
};