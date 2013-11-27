#if !defined(PIECES_WALKS_KANGAROO_H)
#define PIECES_WALKS_KANGAROO_H

/* This module implements the Kangaroo */

#include "position/board.h"
#include "solving/observation.h"

/* Generate moves for an Kangaroo
 */
void kangaroo_generate_moves(void);

boolean kangaroo_check(validator_id evaluate);

/* Generate moves for an Kangaroo Lion
 */
void kangaroo_lion_generate_moves(void);

boolean kangaroolion_check(validator_id evaluate);

#endif
