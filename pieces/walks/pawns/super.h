#if !defined(PIECES_PAWNS_SUPER_H)
#define PIECES_PAWNS_SUPER_H

#include "solving/observation.h"

/* Generate moves for a Super Pawn
 */
void super_pawn_generate_moves(void);

boolean superpawn_check(evalfunction_t *evaluate);

/* Generate moves for a Super-Berolina Pawn
 */
void super_berolina_pawn_generate_moves(void);

boolean superberolinapawn_check(evalfunction_t *evaluate);

#endif
