#ifndef MINOS_H
#define MINOS_H

typedef enum Minos {
    None,
    I,
    J,
    L,
    S,
    T,
    Z,
    O
} Minos;

extern const unsigned int TETROMINOS[6][4];

#endif