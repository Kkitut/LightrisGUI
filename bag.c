#include "bag.h"
#include "minos.h"

inline unsigned char getBag(unsigned int bag, int index) {
    return (bag >> (index * 3)) & 0x7;
}
unsigned char getBagRotated(const unsigned int* bag, int index, int rotate) {
    return getBag(bag[(index + rotate) / BAG_SIZE], (index + rotate) % BAG_SIZE);
}

int generateRandomBag() {
    int bagData = 0;
    int blocks[BAG_SIZE] = {I, J, L, S, T, Z, O};

    for (int i = BAG_SIZE - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = blocks[i];
        blocks[i] = blocks[j];
        blocks[j] = temp;
    }

    for (int i = 0; i < BAG_SIZE; i++) {
        bagData |= (blocks[i] << (i * 3));
    }

    return bagData;
}