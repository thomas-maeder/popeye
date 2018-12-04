#if !defined(POSITION_SIDE_CHANGE_H)
#define POSITION_SIDE_CHANGE_H

/* This modules deals with changes of pieces' flags for various reasons
 */

#include "solving/move_effect_journal.h"

/* Add changing the side of a piece to the current move of the current ply
 * @param reason reason for changing the piece's nature
 * @param on position of the piece to be changed
 */
void move_effect_journal_do_side_change(move_effect_reason_type reason,
                                        square on);

/* Initalise the module */
void position_side_change_initialise(void);

#endif
