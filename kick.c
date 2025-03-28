#include "kick.h"
#include "kicktables.h"
#include "minos.h"
#include "field.h"

inline _Bool Bump3Test(Field* field, Minos mino, unsigned int rot, unsigned int x, unsigned int y) {
    return (!(FieldGetBits3(field, x, y) & TETROMINOS[mino][rot]));
}

inline _Bool Bump4Test(Field* field, Minos mino, unsigned int rot, unsigned int x, unsigned int y) {
    return (!(FieldGetBits4(field, x, y) & TETROMINOS[mino][rot]));
}

_Bool KickRun(Field* field, KickTable kick, Minos mino, unsigned int* currot, unsigned int rot, unsigned int* x, unsigned int* y) {
    if (mino == O) {
        return 1;
    }
    _Bool isI = mino == I;

    _Bool is180 = 0;
    int nextrot = *currot;
    switch (rot) {
        case 0: nextrot = (*currot + 3) & 3; break;
        case 1: nextrot = (*currot + 1) & 3; break;
        case 2: 
            nextrot = (*currot + 2) & 3;
            is180 = 1;
            break;
        default: return 0;
    }

    if (isI) {
        if (Bump4Test(field, mino, nextrot, *x, *y)) {
            *currot = nextrot;
            return 1;
        }
    } else {
        if (Bump3Test(field, mino, nextrot, *x, *y)) {
            *currot = nextrot;
            return 1;
        }
    }

    if (kick == None) {
        return 0;
    }

    int kickIndex, ty, tx;
    const unsigned int* kickTable;

    _Bool isPlus = kick == SRSp;

    if (is180) {
        switch (kick) {
            case SRS | SRSp:
                if (mino == I) {
                    kickTable = SRS_KickTable180_I;
                    kickIndex = SRS_KickTable180_Count;
                } else {
                    kickTable = SRS_KickTable180;
                    kickIndex = SRS_KickTable180_I_Count;
                }
            default:
                return 0;
        }
    } else {
        switch (kick) {
            case SRS | SRSp:
                if (mino == I) {
                    kickTable = SRS_KickTable_I;
                    kickIndex = SRS_KickTable_Count;
                } else {
                    kickTable = SRS_KickTable;
                    kickIndex = SRS_KickTable_I_Count;
                }
            default:
                return 0;
        }
    }

    if (is180) {
        for (int i = 0; i < kickIndex; i++) {
            ty = KickTableExtract(kickTable, nextrot * kickIndex + i * 2) + *x;
            tx = KickTableExtract(kickTable, nextrot * kickIndex + i * 2 + 1) + *y;

            if (!(FieldGetBits4(field, ty, tx) & TETROMINOS[mino][nextrot])) {
                *currot = nextrot;
                *x = ty;
                *y = tx;
                return 1;
            }
        }
    } else {
        for (int i = 0; i < 4; i++) {
            ty = KickTableExtract(kickTable, nextrot * kickIndex + i * 2 + (rot * 32)) + *x;
            tx = KickTableExtract(kickTable, nextrot * kickIndex + i * 2 + 1 + (rot * 32)) + *y;

            if (!(FieldGetBits3(field, ty, tx) & TETROMINOS[mino][nextrot])) {
                *currot = nextrot;
                *x = ty;
                *y = tx;
                return 1;
            }
        }
    }
    return 0;
}
