#include "field.h"

inline unsigned int FieldGetColor(unsigned int data) {
    return data & FIELD_COLOR_MASK;
}
inline unsigned int FieldGetRot(unsigned int data) {
    return (data >> FIELD_DIR_SHIFT) & FIELD_DIR_MASK;
}
inline unsigned int FieldGetShape(unsigned int data) {
    return (data >> FIELD_SHAPE_SHIFT) & FIELD_SHAPE_MASK;
}
inline unsigned int FieldGetType(unsigned int data) {
    return (data >> FIELD_TYPE_SHIFT) & FIELD_TYPE_MASK;
}

inline void FieldSetColor(unsigned int* data, unsigned int color) {
    *data = (*data & ~FIELD_COLOR_MASK) | (color & FIELD_COLOR_MASK);
}
inline void FieldSetRot(unsigned int* data, unsigned int dir) {
    *data = (*data & ~(FIELD_DIR_MASK << FIELD_DIR_SHIFT)) | ((dir & FIELD_DIR_MASK) << FIELD_DIR_SHIFT);
}
inline void FieldSetShape(unsigned int* data, unsigned int shape) {
    *data = (*data & ~(FIELD_SHAPE_MASK << FIELD_SHAPE_SHIFT)) | ((shape & FIELD_SHAPE_MASK) << FIELD_SHAPE_SHIFT);
}
inline void FieldSetType(unsigned int* data, unsigned int type) {
    *data = (*data & ~(FIELD_TYPE_MASK << FIELD_TYPE_SHIFT)) | ((type & FIELD_TYPE_MASK) << FIELD_TYPE_SHIFT);
}

int FieldGetBits(Field* field, int x, int y) {
    return (y < 0 || y >= field->sizeY || x < 0 || x >= field->sizeX || field->data[y * field->sizeX + x]) ? 1 : 0;
}

int FieldGetBits2(Field* field, int x, int y) {
    return
        (FieldGetBits(field, x, y)         << 0) |
        (FieldGetBits(field, x + 1, y)     << 1) |
        (FieldGetBits(field, x, y + 1)     << 2) |
        (FieldGetBits(field, x + 1, y + 1) << 3);
}

int FieldGetBits3(Field* field, int x, int y) {
    return
        (FieldGetBits(field, x - 1, y + 1) << 0) |
        (FieldGetBits(field, x, y + 1)     << 1) |
        (FieldGetBits(field, x + 1, y + 1) << 2) |
        (FieldGetBits(field, x - 1, y)     << 3) |
        (FieldGetBits(field, x, y)         << 4) |
        (FieldGetBits(field, x + 1, y)     << 5) |
        (FieldGetBits(field, x - 1, y - 1) << 6) |
        (FieldGetBits(field, x, y - 1)     << 7) |
        (FieldGetBits(field, + 1, y - 1) << 8);
}

int FieldGetBits4(Field* field, int x, int y) {
    return
        (FieldGetBits(field, x - 1, y + 1) << 0)  |
        (FieldGetBits(field, x, y + 1)     << 1)  |
        (FieldGetBits(field, x + 1, y + 1) << 2)  |
        (FieldGetBits(field, x + 2, y + 1) << 3)  |
        (FieldGetBits(field, x - 1, y)     << 4)  |
        (FieldGetBits(field, x, y)         << 5)  |
        (FieldGetBits(field, x + 1, y)     << 6)  |
        (FieldGetBits(field, x + 2, y)     << 7)  |
        (FieldGetBits(field, x - 1, y - 1) << 8)  |
        (FieldGetBits(field, x, y - 1)     << 9)  |
        (FieldGetBits(field, x + 1, y - 1) << 10) |
        (FieldGetBits(field, x + 2, y - 1) << 11) |
        (FieldGetBits(field, x - 1, y - 2) << 12) |
        (FieldGetBits(field, x, y - 2)     << 13) |
        (FieldGetBits(field, x + 1, y - 2) << 14) |
        (FieldGetBits(field, x + 2, y - 2) << 15);
}