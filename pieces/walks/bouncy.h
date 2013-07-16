#if !defined(PIECES_WALKS_BOUNCY_H)
#define PIECES_WALKS_BOUNCY_H

/* This module implements bouncy pieces */

#include "position/board.h"
#include "utilities/boolean.h"

void clearedgestraversed(void);
boolean traversed(square edgesq);
void settraversed(square edgesq);

/* Generate moves for a bouncy night
 * @param sq_departure common departure square of the generated moves
 */
void bouncy_knight_generate_moves(square sq_departure);

/* Generate moves for a bouncy nightrider
 * @param sq_departure common departure square of the generated moves
 */
void bouncy_nightrider_generate_moves(square sq_departure);

#endif
