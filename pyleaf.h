#if !defined(PYLEAF_H)
#define PYLEAF_H

#include "boolean.h"
#include "pystip.h"
#include "py.h"

/* This module provides functionality dealing with leaf stipulation
 * slices.
 */

/* Determine whether a side has reached the goal of a leaf slice.
 * @param just_moved side that has just moved
 * @param leaf slice index of leaf slice
 * @return whether just_moved has reached leaf's goal
 */
goal_checker_result_type leaf_is_goal_reached(Side just_moved,
                                              slice_index leaf);

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean leaf_impose_starter(slice_index si, slice_traversal *st);

/* Determine whether the mating side still has a piece that could
 * deliver the mate.
 * @return true iff the mating side has such a piece
 */
boolean is_a_mating_piece_left(Side mating_side);

/* Generate moves for side side_at_move; optimise for moves reaching a
 * specific goal.
 * @param leaf leaf slice whose goal is to be reached by generated
 *             move(s)
 * @param side_at_move side for which to generate moves
 */
void generate_move_reaching_goal(slice_index leaf, Side side_at_move);

#endif
