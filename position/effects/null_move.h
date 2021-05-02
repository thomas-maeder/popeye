#if !defined(POSITION_NULL_MOVE_H)
#define POSITION_NULL_MOVE_H

/* This modules deals with null moves for various reasons
 */

#include "solving/move_effect_journal.h"

/* Add a null effect to the current move of the current ply
 * @param reason for null effect
 */
void move_effect_journal_do_null_effect(move_effect_reason_type reason);

/* Add the effects of a null move to the current move of the current ply
 * @param reason for null move
 */
void move_effect_journal_do_null_move(move_effect_reason_type reason);

#endif
