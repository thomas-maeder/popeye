#if !defined(PIECES_WALKS_ORPHAN_H)
#define PIECES_WALKS_ORPHAN_H

/* This module implements the Orphan */

#include "pieces/pieces.h"
#include "position/board.h"
#include "position/position.h"
#include "utilities/boolean.h"
#include "solving/observation.h"

extern piece_walk_type orphanpieces[nr_piece_walks];

/* Generate moves for an Orphan
 */
void orphan_generate_moves(void);

boolean orphan_check(validator_id evaluate);

#endif
