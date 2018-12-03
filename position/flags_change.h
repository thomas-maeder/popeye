#if !defined(POSITION_FLAGS_CHANGE_H)
#define POSITION_FLAGS_CHANGE_H

/* This modules deals with changes of pieces' flags for various reasons
 */

#include "solving/move_effect_journal.h"

/* Add changing the flags of a piece to the current move of the current ply
 * @param reason reason for moving the king square
 * @param on position of pieces whose flags to piece_change
 * @param to changed flags
 */
void move_effect_journal_do_flags_change(move_effect_reason_type reason,
                                         square on,
                                         Flags to);

/* Initalise the module */
void position_flags_change_initialise(void);

#endif
