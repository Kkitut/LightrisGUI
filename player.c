#include "player.h"
#include "minos.h"

inline unsigned int PlayerGetHand(unsigned int status) {
    return (status >> 3) & PLAYER_HAND_MASK;
}
inline unsigned int PlayerGetHold(unsigned int status) {
    return (status >> 6) & PLAYER_HOLD_MASK;
}
inline unsigned int PlayerGetSpin(unsigned int status) {
    return (status >> 9) & PLAYER_SPIN_MASK;
}

inline void PlayerSetHand(unsigned int* status, unsigned int hand) {
    *status = (*status & ~(PLAYER_HAND_MASK << PLAYER_HAND_SHIFT)) | ((hand & PLAYER_HAND_MASK) << PLAYER_HAND_SHIFT);
}
inline void PlayerSetHold(unsigned int* status, unsigned int hold) {
    *status = (*status & ~(PLAYER_HOLD_MASK << PLAYER_HOLD_SHIFT)) | ((hold & PLAYER_HOLD_MASK) << PLAYER_HOLD_SHIFT);
}
inline void PlayerSetSpin(unsigned int* status, unsigned int spin) {
    *status = (*status & ~(PLAYER_SPIN_MASK << PLAYER_SPIN_SHIFT)) | ((spin & PLAYER_SPIN_MASK) << PLAYER_SPIN_SHIFT);
}

static void PlayerResetPos(Player* player) {
    player->x = player->field->sizeX / 2 - 1;
    player->y = player->field->sizeY + 1;
}

static _Bool PlayerIsCanMoveAt(Player* player,int deg) {
    int xp = player->x;
    int yp = player->y;
    switch (deg) {
        case 0:
            yp--;
            break;
        case 1:
            xp--;
            break;
        case 2:
            xp++;
            break;
    }
    int hand = PlayerGetHand(player->status);
    int spin = PlayerGetSpin(player->status);
    if (hand) {
        if (!(FieldGetBits2(player->field, xp, yp) & 0b1111)) {
            return 1;
        }
    } else if (hand) {
        if (!(FieldGetBits3(player->field, xp, yp) & TETROMINOS[hand][spin])) {
            return 1;
        }
    } else {
        if (!(FieldGetBits4(player->field, xp, yp) & TETROMINOS[0][spin])) {
            return 1;
        }
    }
    return 0;
}

static void PlayerDrop(Player* player, int clock) {
    player->lineClear = 0;
    int downy = player->y;
    gravityReset();

    for (;;) {
        if (PlayerGetHand(player->status) == 6) {
            if ((FieldGetBits2(player->field, player->x, downy - 1) & 0b1111)) break;
        } else if (PlayerGetHand(player->status)) {
            if ((FieldGetBits3(player->field, player->x, downy - 1) & TETROMINOS[PlayerGetHand(player->status)][PlayerGetSpin(player->status)])) break;
        } else {
            if ((FieldGetBits4(player->field, player->x, downy - 1) & TETROMINOS[0][PlayerGetSpin(player->status)])) break;
        }
        downy--;
    }

    int data = 0;
    FieldSetColor(&data, PlayerGetHand(player->status) + 1);

    if (PlayerGetHand(player->status) == 6) {
        for (int i = 0; i < 2; i++)
            for (int j = 0; j < 2; j++)
                FieldGetBits(player->field, player->x + j, downy + i) ==
                player->field->cells[downy + i][player->x + j] = data;
    } else if (PlayerGetHand(player->status)) {
        int minoindex = 2;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (TETROMINOS[PlayerGetHand(player->status)][player->currot] & (1 << (minoindex * 3 + j))) {
                    player->field->cells[downy + i - 1][player->x + j - 1] = data;
                }
            }
            minoindex--;
        }
    } else {
        int minoindex = 3;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (TETROMINOS[0][player->currot] & (1 << (minoindex * 4 + j))) {
                    player->field->cells[downy + i - 2][player->x + j - 1] = data;
                }
            }
            minoindex--;
        }
    }

    for (int i = 0; i < FIELD_SIZE_Y; i++) {
        _Bool full = 1;
        for (int j = 0; j < FIELD_SIZE_X; j++) {
            if (player->field->cells[i][j] == 0) {
                full = 0;
                break;
            }
        }
        if (full) {
            for (int k = i; k < FIELD_SIZE_Y - 1; k++) {
                memcpy(player->field->cells[k], player->field->cells[k + 1], sizeof(player->field->cells[k]));
            }
            memset(player->field->cells[FIELD_SIZE_Y - 1], 0, sizeof(player->field->cells[0]));
            i--;
            player->lineClear++;
        }
    }

    if (player->lineClear)
        player->comboCount++;
    else
        player->comboCount = 0;

    player->bagrotate++;
    if (player->bagrotate > 7) {
        player->bagrotate = 1;
        player->bag[0] = player->bag[1];
        player->bag[1] = generateRandomBag();
    }

    player->isHolding = 0;
    PlayerSetHand(&player->status, getBagRotated(player->bag, 0, player->bagrotate));
    resetPos(player);
    player->currot = 0;
}
