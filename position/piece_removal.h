#if !defined(POSITION_PIECE_REMOVAL_H)
#define POSITION_PIECE_REMOVAL_H

/* This modules deals with re-additions of pieces for various reasons
 */

#include "solving/move_effect_journal.h"

/* Add removing a piece to the current move of the current ply
 * @param reason reason for removing the piece
 * @param from current position of the piece
 * @note use move_effect_journal_do_capture_move(), not
 * move_effect_journal_do_piece_removal() for regular captures
 */
void move_effect_journal_do_piece_removal(move_effect_reason_type reason,
                                          square from);

/* Initalise the module */
void position_piece_removal_initialise(void);

#endif
