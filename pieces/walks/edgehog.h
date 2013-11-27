#if !defined(PIECES_WALKS_EDGEHOG_H)
#define PIECES_WALKS_EDGEHOG_H

/* This module implements the Edgehog */

#include "position/board.h"
#include "solving/observation.h"

/* Generate moves for an Edgehog
 */
void edgehog_generate_moves(void);

boolean edgehog_check(validator_id evaluate);

#endif
