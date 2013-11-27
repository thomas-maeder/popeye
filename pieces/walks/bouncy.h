#if !defined(PIECES_WALKS_BOUNCY_H)
#define PIECES_WALKS_BOUNCY_H

/* This module implements bouncy pieces */

#include "position/board.h"
#include "utilities/boolean.h"
#include "solving/observation.h"

void clearedgestraversed(void);
boolean traversed(square edgesq);
void settraversed(square edgesq);

/* Generate moves for a bouncy night
 */
void bouncy_knight_generate_moves(void);

boolean bouncy_knight_check(validator_id evaluate);

/* Generate moves for a bouncy nightrider
 */
void bouncy_nightrider_generate_moves(void);

boolean bouncy_nightrider_check(validator_id evaluate);

#endif
