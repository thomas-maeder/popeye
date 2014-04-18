#if !defined(PIECES_WALKS_FRIEND_H)
#define PIECES_WALKS_FRIEND_H

/* This module implements the Friend */

#include "pieces/pieces.h"
#include "position/board.h"
#include "position/position.h"
#include "utilities/boolean.h"
#include "solving/observation.h"

void locate_observees(piece_walk_type walk, square pos_observees[]);
void isolate_observee(piece_walk_type walk, square const pos_observees[], unsigned int isolated_observee);
void restore_observees(piece_walk_type walk, square const pos_observees[]);

/* Generate moves for a Friend
 */
void friend_generate_moves(void);

boolean friend_check(validator_id evaluate);

#endif
