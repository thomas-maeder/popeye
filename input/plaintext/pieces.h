#if !defined(INPUT_PLAINTEXT_PIECES_H)
#define INPUT_PLAINTEXT_PIECES_H

#include "position/position.h"
#include "utilities/boolean.h"

int GetPieNamIndex(char a, char b);

char *ParseSingleWalk(char *tok, piece_walk_type *result);

typedef enum
{
  piece_addition_initial,
  piece_addition_twinning
} piece_addition_type;

char *ParsePieces(char *tok, piece_addition_type type);

Flags ParseColour(char *tok, boolean colour_is_mandatory);
char *ParsePieceFlags(Flags *flags);
char *ParsePieceWalk(char *tok, piece_walk_type *name);

#endif
