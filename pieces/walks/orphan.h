#if !defined(PIECES_WALKS_ORPHAN_H)
#define PIECES_WALKS_ORPHAN_H

/* This module implements the Friend */

#include "position/board.h"
#include "position/position.h"
#include "utilities/boolean.h"
#include "pyproc.h"

/* Generate moves for a rider piece
 * @param sq_departure common departure square of the generated moves
 */
void orphan_generate_moves(square sq_departure);

#endif
