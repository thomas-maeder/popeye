#if !defined(POSITION_PIECE_REMOVAL_H)
#define POSITION_PIECE_REMOVAL_H

/* This modules deals with re-additions of pieces for various reasons
 */

#include "solving/move_effect_journal.h"

void undo_piece_removal(move_effect_journal_entry_type const *entry);
void redo_piece_removal(move_effect_journal_entry_type const *entry);

/* Add removing a piece to the current move of the current ply
 * @param reason reason for removing the piece
 * @param from current position of the piece
 * @note use move_effect_journal_do_capture_move(), not
 * move_effect_journal_do_piece_removal() for regular captures
 */
void move_effect_journal_do_piece_removal(move_effect_reason_type reason,
                                          square from);

/* Fill the capture gap at the head of each move by no capture
 */
void move_effect_journal_do_no_piece_removal(void);

/* Follow the captured or a moved piece through the "other" effects of a move
 * @param followed_id id of the piece to be followed
 * @param idx index of a piece_removal effect
 * @param pos position of the piece when effect idx is played
 * @return the position of the piece with effect idx applied
 *         initsquare if the piece is not on the board after effect idx
 */
square position_piece_removal_follow_piece(PieceIdType followed_id,
                                           move_effect_journal_index_type idx,
                                           square pos);

/* Initalise the module */
void position_piece_removal_initialise(void);

#endif
