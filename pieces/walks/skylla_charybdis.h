#if !defined(PIECES_WALKS_SKYALLA_CHARYBDIS_H)
#define PIECES_WALKS_SKYALLA_CHARYBDIS_H

/* This module implements Skylla and Charybdis */

#include "position/board.h"
#include "position/position.h"
#include "pieces/walks/vectors.h"

/* Generate moves for a Skylla
 * @param sq_departure common departure square of the generated moves
 */
void skylla_generate_moves(square sq_departure);

/* Generate moves for a Charybdis
 * @param sq_departure common departure square of the generated moves
 */
void charybdis_generate_moves(square sq_departure);

#endif
