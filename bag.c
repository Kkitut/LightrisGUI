#include "bag.h"
#include "minos.h"

#include <stdlib.h>

inline unsigned char getBag(unsigned int bag, int index) {
    return (bag >> (index * 3)) & 0x7;
}
unsigned char getBagRotated(const unsigned int* bag, int index, int rotate) {
    return getBag(bag[(index + rotate) / 7], (index + rotate) % 7);
}

int generateRandomBag() {
    int bagData = 0;
    int blocks[] = {I, J, L, S, T, Z, O};

    for (int i = 7 - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = blocks[i];
        blocks[i] = blocks[j];
        blocks[j] = temp;
    }

    for (int i = 0; i < 7; i++) {
        bagData |= (blocks[i] << (i * 3));
    }

    return bagData;
}