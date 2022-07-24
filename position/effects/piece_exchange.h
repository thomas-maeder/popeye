#if !defined(POSITION_PIECE_EXCHANGE_H)
#define POSITION_PIECE_EXCHANGE_H

/* This modules deals with exchanges of places of pieces for various reasons
 */

#include "solving/move_effect_journal.h"

/* Add exchanging two pieces to the current move of the current ply
 * @param reason reason for exchanging the two pieces
 * @param from position of primary piece
 * @param to position of partner piece
 */
void move_effect_journal_do_piece_exchange(move_effect_reason_type reason,
                                           square from,
                                           square to);

/* Follow the captured or a moved piece through the "other" effects of a move
 * @param followed_id id of the piece to be followed
 * @param idx index of a piece_exchange effect
 * @param pos position of the piece when effect idx is played
 * @return the position of the piece with effect idx applied
 *         initsquare if the piece is not on the board after effect idx
 */
square position_piece_exchange_follow_piece(PieceIdType followed_id,
                                           move_effect_journal_index_type idx,
                                           square pos);

/* Initalise the module */
void position_piece_exchange_initialise(void);

#endif
