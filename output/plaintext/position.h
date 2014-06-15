#if !defined(OUTPUT_PLAINTEXT_POSITION_H)
#define OUTPUT_PLAINTEXT_POSITION_H

#include "position/position.h"

#include <stdio.h>

void WriteBoard(FILE *file, position const *pos);

void WritePositionRegular(FILE *file);
void WritePositionProofGame(FILE *file);
void WritePositionAtoB(FILE *file, Side starter);

#endif
