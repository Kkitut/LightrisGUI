#ifndef KICK_H
#define KICK_H

#include "field.h"
#include "minos.h"
#include "kicktables.h"

inline _Bool Bump3Test(Field* field, Minos mino, unsigned int rot, unsigned int x, unsigned int y);
inline _Bool Bump4Test(Field* field, Minos mino, unsigned int rot, unsigned int x, unsigned int y);
_Bool KickRun(Field* field, KickTable kick, Minos mino, unsigned int* currot, unsigned int rot, unsigned int* x, unsigned int* y);

#endif