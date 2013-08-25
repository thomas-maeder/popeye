#if !defined(INPUT_PIECES_H)
#define INPUT_PIECES_H

#include "position/position.h"
#include "utilities/boolean.h"

int GetPieNamIndex(char a, char b);

square SquareNum(char a, char b);

typedef enum
{
  piece_addition_initial,
  piece_addition_twinning
} piece_addition_type;

char *ParsePieces(piece_addition_type type);

Flags ParseColor(char *tok, boolean color_is_mandatory);
char *ParsePieceFlags(Flags *flags);
char *ParsePieceName(char *tok, PieNam *name);

#endif
