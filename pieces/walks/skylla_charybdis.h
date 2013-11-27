#if !defined(PIECES_WALKS_SKYALLA_CHARYBDIS_H)
#define PIECES_WALKS_SKYALLA_CHARYBDIS_H

/* This module implements Skylla and Charybdis */

#include "position/board.h"
#include "position/position.h"
#include "pieces/walks/vectors.h"
#include "solving/observation.h"

/* Generate moves for a Skylla
 */
void skylla_generate_moves(void);

boolean skylla_check(validator_id evaluate);

/* Generate moves for a Charybdis
 */
void charybdis_generate_moves(void);

boolean charybdis_check(validator_id evaluate);

#endif
