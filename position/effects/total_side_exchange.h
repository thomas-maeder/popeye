#if !defined(POSITION_TOTAL_SIDE_EXCHANGE_H)
#define POSITION_TOTAL_SIDE_EXCHANGE_H

/* This modules deals with changes of all pieces' sides for various reasons
 */

#include "solving/move_effect_journal.h"

/* Add changing the side of a piece to the current move of the current ply
 * @param reason reason for changing the piece's nature
 * @param on position of the piece to be changed
 */
void move_effect_journal_do_total_side_exchange(move_effect_reason_type reason);

/* Initalise the module */
void position_total_side_exchange_initialise(void);

#endif
