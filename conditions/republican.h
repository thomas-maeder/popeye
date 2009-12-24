#if !defined(REPUBLICAN_H)
#define REPUBLICAN_H

#include "pyproc.h"

/* In Republican Chess Type 1, Republican Chess is suspended once a
 * side has inserted the opposite king.
 */
extern boolean is_republican_suspended;

/* Perform the necessary verification steps for solving a Republican
 * Chess problem
 * @return true iff verification passed
 */
boolean republican_verifie_position(void);

/* Write the Republican Chess diagram caption
 * @param CondLine string containing the condition caption
 * @param lineLength size of array CondLine
 */
void republican_write_diagram_caption(char CondLine[], size_t lineLength);

/* Place the opposite king as part of playing a move
 * @param jt are we playing the move for the first time?
 * @param moving side at the move
 * @param ply_id id of ply in which the move is played
 */
void republican_place_king(joue_type jt, Side moving, ply ply_id);

/* Unplace the opposite king as part of taking back a move
 */
void republican_unplace_king(void);

/* Advance the square where to place the opposite king as part of
 * taking back a move
 */
boolean republican_advance_king_square(void);

/* Save the Republican Chess part of the current move in a play
 * @param ply_id identifies ply of move to be saved
 * @param mov address of structure where to save the move
 */
void republican_current(ply ply_id, coup *move);

/* Compare the Republican Chess parts of two saved moves
 * @param move1 address of 1st saved move
 * @param move2 address of 2nd saved move
 * @return true iff the Republican Chess parts are equal
 */
boolean republican_moves_equal(coup const *move1, coup const *move2);

/* Write how the opposite king is placed as part of a move
 * @param move address of move being written
 */
void write_republican_king_placement(coup const *move);

#endif
