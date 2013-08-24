#if !defined(PIECES_WALKS_ORPHAN_H)
#define PIECES_WALKS_ORPHAN_H

/* This module implements the Orphan */

#include "pieces/pieces.h"
#include "position/board.h"
#include "position/position.h"
#include "utilities/boolean.h"
#include "solving/observation.h"

extern PieNam orphanpieces[PieceCount];

/* Generate moves for an Orphan
 */
void orphan_generate_moves(void);

boolean orphan_check(evalfunction_t *evaluate);

#endif
