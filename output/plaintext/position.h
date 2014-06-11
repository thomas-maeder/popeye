#if !defined(OUTPUT_PLAINTEXT_POSITION_H)
#define OUTPUT_PLAINTEXT_POSITION_H

#include "position/position.h"

void WriteBoard(position const *pos);

void WritePositionRegular(void);
void WritePositionProofGame(void);
void WritePositionAtoB(Side starter);

#endif
