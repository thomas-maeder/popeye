#if !defined(PIECES_WALKS_CARDINAL_H)
#define PIECES_WALKS_CARDINAL_H

/* This module implements the Cardinal */

#include "position/board.h"
#include "solving/observation.h"

/* Generate moves for a Cardinal
 */
void cardinal_generate_moves(void);

boolean cardinal_check(validator_id evaluate);

#endif
