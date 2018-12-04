#if !defined(POSITION_NULL_MOVE_H)
#define POSITION_NULL_MOVE_H

/* This modules deals with null moves for various reasons
 */

#include "solving/move_effect_journal.h"

/* Add a null effect to the current move of the current ply
 */
void move_effect_journal_do_null_effect(void);

/* Add the effects of a null move to the current move of the current ply
 */
void move_effect_journal_do_null_move(void);

#endif
