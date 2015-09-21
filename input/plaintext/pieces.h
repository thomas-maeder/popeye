#if !defined(INPUT_PLAINTEXT_PIECES_H)
#define INPUT_PLAINTEXT_PIECES_H

#include "position/position.h"
#include "utilities/boolean.h"

int GetPieNamIndex(char a, char b);

char *ParseSingleWalk(char *tok, piece_walk_type *result);

char *ParsePieces(char *tok);

Flags ParseColour(char *tok, boolean colour_is_mandatory);
char *ParsePieceFlags(Flags *flags);
char *ParsePieceWalk(char *tok, piece_walk_type *name);

#endif
