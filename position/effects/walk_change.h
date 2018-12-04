#if !defined(POSITION_WALK_CHANGE_H)
#define POSITION_WALK_CHANGE_H

/* This modules deals with changes of pieces' walks for various reasons
 */

#include "solving/move_effect_journal.h"

/* Add changing the walk of a piece to the current move of the current ply
 * @param reason reason for changing the piece's nature
 * @param on position of the piece to be changed
 * @param to new nature of piece
 */
void move_effect_journal_do_walk_change(move_effect_reason_type reason,
                                        square on,
                                        piece_walk_type to);

/* Initalise the module */
void position_walk_change_initialise(void);

#endif
