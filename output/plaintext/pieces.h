#if !defined(OUTPUT_PLAINTEXT_PIECES_H)
#define OUTPUT_PLAINTEXT_PIECES_H

#include "position/position.h"
#include "pieces/pieces.h"
#include "utilities/boolean.h"

boolean WriteSpec(Flags pspec, PieNam p, boolean printcolours);
void WritePiece(PieNam p);
void WriteSquare(square s);

void AppendSquare(char *List, square s);

#endif
