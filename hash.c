#include "hash.h"
#include <stdio.h>


static hash_t digest(uint8_t* ssn, uint32_t len) {
    uint32_t hash = 5381;
    for(int i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) + ssn[i];
    }
    return (hash_t) (hash % 256);
}

hash_t hash_ssn(uint8_t* ssn) {
    return digest(ssn, 12);
}
