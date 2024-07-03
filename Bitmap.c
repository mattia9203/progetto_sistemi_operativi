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
void BitMap_setBit(BitMap* bitmap, int num_bit, int value){
    int num_byte = num_bit/8;
    assert(num_byte < bitmap->buffer_size);
    int bit = num_byte&0x03;
    if(value){
        bitmap->buffer[num_byte] |= (1<<bit);    //setta bit a 1
    }
    else{
        bitmap->buffer[num_byte] &= (1<<bit);    //setta bit a 0
    }
}
//funzione che ritorna 1 se bit 1 o 0 altrimenti
int BitMap_getBit(BitMap* bitmap, int num_bit){
    int num_byte = num_bit/8;
    assert(num_byte < bitmap->buffer_size);
    int bit = num_byte&0x03;
    return (bitmap->buffer[num_byte] & (1<<bit)) != 0;
}