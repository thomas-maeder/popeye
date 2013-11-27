#if !defined(PIECES_WALKS_BOB_H)
#define PIECES_WALKS_BOB_H

/* This module implements the Bob */

#include "position/board.h"
#include "solving/observation.h"

/* Generate moves for an Bob
 */
void bob_generate_moves(void);

boolean bob_check(validator_id evaluate);

#endif
