#if !defined(PIECES_WALKS_QUERQUISITE_H)
#define PIECES_WALKS_QUERQUISITE_H

/* This module implements the Querquisite */

#include "position/board.h"
#include "solving/observation.h"

/* Generate moves for an Querquisite
 */
void querquisite_generate_moves(void);

boolean querquisite_check(evalfunction_t *evaluate);

#endif
