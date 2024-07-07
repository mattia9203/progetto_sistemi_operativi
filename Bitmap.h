#include <stdint.h>

typedef struct {
    char* buffer;
    int buffer_size;
    int num_bits;     //numero bit della bitmap
} BitMap;

int BitMap_getBytes(int bits);     //numero bytes necessari a memorizzare bits

void BitMap_init(BitMap* bit_map, int num_bits, char* buffer);  //funzione che costruisce e inizializza la bitmap

void BitMap_setBit(BitMap* bit_map, int bit_num, int status);   //set il #bit_num bit della bitmap(ovviamente 0 o 1)

int BitMap_getBit(const BitMap* bit_map, int bit_num);    //analizza il bit bit_num