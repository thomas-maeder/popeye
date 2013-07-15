#if !defined(PIECES_WALKS_KANGAROO_H)
#define PIECES_WALKS_KANGAROO_H

/* This module implements the Kangaroo */

#include "position/board.h"

/* Generate moves for an Kangaroo
 * @param sq_departure common departure square of the generated moves
 */
void kangaroo_generate_moves(square sq_departure);

/* Generate moves for an Kangaroo Lion
 * @param sq_departure common departure square of the generated moves
 */
void kangaroo_lion_generate_moves(square sq_departure);

#endif
