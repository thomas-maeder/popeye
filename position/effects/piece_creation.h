#if !defined(POSITION_PIECE_CREATION_H)
#define POSITION_PIECE_CREATION_H

/* This modules deals with re-additions of pieces for various reasons
 */

#include "solving/move_effect_journal.h"

/* Add a newly created piece to the current move of the current ply
 * @param reason reason for creating the piece
 * @param on where to insert the piece
 * @param created nature of created piece
 * @param createdspec specs of created piece
 * @param for which side is the (potentially neutral) piece created
 * @return true iff we have successfully created the piece
 */
boolean move_effect_journal_do_piece_creation(move_effect_reason_type reason,
                                              square on,
                                              piece_walk_type created,
                                              Flags createdspec,
                                              Side for_side);

/* Follow the captured or a moved piece through the "other" effects of a move
 * @param followed_id id of the piece to be followed
 * @param idx index of a piece_creation effect
 * @param pos position of the piece when effect idx is played
 * @return the position of the piece with effect idx applied
 *         initsquare if the piece is not on the board after effect idx
 */
square position_piece_creation_follow_piece(PieceIdType followed_id,
                                           move_effect_journal_index_type idx,
                                           square pos);

/* Initalise the module */
void position_piece_creation_initialise(void);

#endif
