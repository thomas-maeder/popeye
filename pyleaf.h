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
 * @return true iff just_moved has reached leaf's goal
 */
boolean leaf_is_goal_reached(Side just_moved, slice_index leaf);

/* Impose the starting side on a leaf. 
 * @param leaf identifies leaf
 * @param s starting side of leaf
 */
void leaf_impose_starter(slice_index leaf, Side s);

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

/* Write a priori unsolvability (if any) of a leaf (e.g. forced reflex
 * mates)
 * @param leaf leaf's slice index
 */
void leaf_write_unsolvability(slice_index leaf);

#endif
