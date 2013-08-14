#if !defined(PIECES_PAWNS_BEROLINA_H)
#define PIECES_PAWNS_BEROLINA_H

#include "solving/observation.h"

/* Generate moves for a Berolina pawn
 */
void berolina_pawn_generate_moves(void);

boolean berolina_pawn_check(evalfunction_t *evaluate);

#endif
