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

/* Initalise the module */
void position_board_transformation_initialise(void);

#endif
