#if !defined(PIECES_PAWNS_REVERSE_H)
#define PIECES_PAWNS_REVERSE_H

#include "solving/observation.h"

/* Generate moves for a reverse pawn
 */
void reverse_pawn_generate_moves(void);

boolean reversepawn_check(evalfunction_t *evaluate);

#endif
