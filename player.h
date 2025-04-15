#ifndef PLAYER_H
#define PLAYER_H

#include "field.h"

typedef struct Player {
    Field* field;
    int fieldX, fieldY;
    unsigned int x, y;
    unsigned int attackCount;
    unsigned int lineCount;
    unsigned int b2bCount;
    unsigned int comboCount;
    unsigned int lineClear;
    unsigned int status
} Player;

#define PLAYER_HAND_MASK        0x7
#define PLAYER_HOLD_MASK        0x7
#define PLAYER_SPIN_MASK        0xF

#define PLAYER_HAND_SHIFT       0
#define PLAYER_HOLD_SHIFT       3
#define PLAYER_SPIN_SHIFT       9

inline unsigned int PlayerGetHand(unsigned int status);
inline unsigned int PlayerGetHold(unsigned int status);
inline unsigned int PlayerGetSpin(unsigned int status);

inline void PlayerSetHand(unsigned int* status, unsigned int hand);
inline void PlayerSetHold(unsigned int* status, unsigned int hold);
inline void PlayerSetSpin(unsigned int* status, unsigned int spin);

static _Bool PlayerIsCanMoveAt(Player* player, int deg);

#endif