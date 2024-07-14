#include "Bitmap.h"
#include <assert.h>

//return del numero di bytes necessari per memorizzare la bitmap
int BitMap_getBytes(int bits){
    int plus = (bits%8) != 0 ? 1 : 0;  
    return bits/8 + plus;        //se necessario arrotondiamo per eccesso
}

//costruiamo la bitmap di dimensione buffer_size che metteremo su un buffer 
void BitMap_init(BitMap* bitmap, int num_bits, char* buf){
    bitmap->buffer = buf;
    bitmap->num_bits = num_bits;
    bitmap -> buffer_size = BitMap_getBytes(num_bits);
}

//funzione per settare il num_bit nella bitmap a value(0 o 1)
void BitMap_setBit(BitMap* bitmap, int idx, int value){
    int num_byte = idx>>3;                 //num_byte = idx/8,calcola byte della bitmap che contiene il bit che ci interessa 
    assert(num_byte < bitmap->buffer_size);
    int bit = idx&0x07;                    //trova la posizione del bit all'interno del byte -> num_bit%8
    if(value){
        bitmap->buffer[num_byte] |= (1<<bit);    //setta bit a 1 utilizzando l'OR bit a bit
    }
    else{
        bitmap->buffer[num_byte] &= ~(1<<bit);    //setta bit a 0 utilizzando l'AND bit a bit
    }
}
//funzione che ritorna 1 se bit 1 o 0 altrimenti
int BitMap_getBit(const BitMap* bitmap, int idx){
    int num_byte = idx>>3;                  //calcola byte della bitmap che contiene il bit che ci interessa       
    assert(num_byte < bitmap->buffer_size);
    int bit = idx&0x07;
    return (bitmap->buffer[num_byte] & (1<<bit)) != 0;         //ritorna 1 se bit 1 e 0 altrimenti, attraverso l'AND bit a bit
}