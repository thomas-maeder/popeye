#if !defined(POSITION_PIECE_MOVEMENT_H)
#define POSITION_PIECE_MOVEMENT_H

/* This modules deals with movements of pieces for various reasons
 */

#include "solving/move_effect_journal.h"

/* Add moving a piece to the current move of the current ply
 * @param reason reason for moving the piece
 * @param from current position of the piece
 * @param to where to move the piece
 */
void move_effect_journal_do_piece_movement(move_effect_reason_type reason,
                                           square from,
                                           square to);

/* Initalise the module */
void position_piece_movement_initialise(void);

#endif
