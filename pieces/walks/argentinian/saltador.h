#if !defined(PIECES_WALKS_ARGENTINIAN_SALTADOR_H)
#define PIECES_WALKS_ARGENTINIAN_SALTADOR_H

/* This module implements the Saltador (argentinian knight) */

#include "position/board.h"
#include "solving/observation.h"

/* Generate moves for a Saltador
 */
void saltador_generate_moves(void);

boolean saltador_check(validator_id evaluate);

#endif
