#ifndef KICKTABLES_H
#define KICKTABLES_H

typedef enum KickTable {
    NONE,
    SRS,
    SRSp,
} KickTable;

extern const unsigned int SRS_KickTable[7];
extern const unsigned int SRS_KickTable180[9];
extern const unsigned int SRS_KickTable_I[7];
extern const unsigned int SRS_KickTable180_I[4];

typedef enum KickTable_Count {
    SRS_KickTable_Count = 8,
    SRS_KickTable180_Count = 10,
    SRS_KickTable_I_Count = 8,
    SRS_KickTable180_I_Count = 4,
} KickTable_Count;

unsigned int KickTableExtract(const unsigned int* table, unsigned int kickIndex);

#endif