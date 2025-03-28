#include "player.h"

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