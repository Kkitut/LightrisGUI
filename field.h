#ifndef FIELD_H
#define FIELD_H

#define FIELD_COLOR_MASK    0x1F
#define FIELD_DIR_MASK      0x3
#define FIELD_SHAPE_MASK    0x7
#define FIELD_TYPE_MASK     0x3

#define FIELD_COLOR_SHIFT   0
#define FIELD_DIR_SHIFT     5
#define FIELD_SHAPE_SHIFT   7
#define FIELD_TYPE_SHIFT    10

typedef struct Field {
    unsigned int* data;
    unsigned int sizeX, sizeY;
    unsigned int logicalY;
} Field;

inline unsigned int FieldGetColor(unsigned int data);
inline unsigned int FieldGetRot(unsigned int data);
inline unsigned int FieldGetShape(unsigned int data);
inline unsigned int FieldGetType(unsigned int data);

inline void FieldSetColor(unsigned int* data, unsigned int color);
inline void FieldSetRot(unsigned int* data, unsigned int dir);
inline void FieldSetShape(unsigned int* data, unsigned int shape);
inline void FieldSetType(unsigned int* data, unsigned int type);

unsigned int FieldGetBits(Field* field, int x, int y);
unsigned int FieldGetBits2(Field* field, int x, int y);
unsigned int FieldGetBits3(Field* field, int x, int y);
unsigned int FieldGetBits4(Field* field, int x, int y);

#endif