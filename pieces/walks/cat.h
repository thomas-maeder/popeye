#if !defined(PIECES_WALKS_CAT_H)
#define PIECES_WALKS_CAT_H

/* This module implements the Cat */

#include "position/board.h"
#include "solving/observation.h"

/* Generate moves for a cat
 */
void cat_generate_moves(void);

boolean cat_check(validator_id evaluate);

#endif
