#if !defined(OUTPUT_PLAINTEXT_PIECES_H)
#define OUTPUT_PLAINTEXT_PIECES_H

#include "position/position.h"
#include "pieces/pieces.h"
#include "utilities/boolean.h"

#include <stdio.h>

boolean WriteSpec(FILE *file, Flags pspec, piece_walk_type p, boolean printcolours);
void WritePiece(FILE *file, piece_walk_type p);
void WriteSquare(FILE *file, square s);

void AppendSquare(char *List, square s);

#endif
