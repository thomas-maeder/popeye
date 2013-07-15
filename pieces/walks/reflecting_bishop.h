#if !defined(PIECES_WALKS_REFLECTING_BISHOP_H)
#define PIECES_WALKS_REFLECTING_BISHOP_H

/* This module implements the reflecting bishops */

#include "position/board.h"

/* Generate moves for an reflecting bishop
 * @param sq_departure common departure square of the generated moves
 */
void reflecting_bishop_generate_moves(square sq_departure);

/* Generate moves for an reflecting bishop
 * @param sq_departure common departure square of the generated moves
 */
void archbishop_generate_moves(square sq_departure);

#endif
