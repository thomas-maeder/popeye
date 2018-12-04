#if !defined(POSITION_BOARD_TRANSFORMATION_H)
#define POSITION_BOARD_TRANSFORMATION_H

/* This modules deals with board transformations for various reasons
 */

#include "solving/move_effect_journal.h"

/* Add transforming the board to the current move of the current ply
 * @param reason reason for moving the king square
 * @param transformation how to transform the board
 */
void move_effect_journal_do_board_transformation(move_effect_reason_type reason,
                                                 SquareTransformation transformation);

/* Follow the captured or a moved piece through the "other" effects of a move
 * @param followed_id id of the piece to be followed
 * @param idx index of a board_transformation effect
 * @param pos position of the piece when effect idx is played
 * @return the position of the piece with effect idx applied
 *         initsquare if the piece is not on the board after effect idx
 */
square position_board_transformation_follow_piece(PieceIdType followed_id,
                                                  move_effect_journal_index_type idx,
                                                  square pos);

/* Initalise the module */
void position_board_transformation_initialise(void);

#endif
