#if !defined(PYLEAFF_H)
#define PYLEAFF_H

#include "pyslice.h"
#include "pydirect.h"

/* This module provides functionality dealing with leaf slices that
 * find forced half-moves reaching the goal.
 */

/* Write the key
 * @param leaf slice index
 * @param type type of attack
 */
void leaf_forced_root_write_key(slice_index leaf, attack_type type);

/* Determine whether the starting side has won with its move just
 * played.
 * @param leaf slice identifier
 * @return whether starter has won
 */
has_starter_won_result_type leaf_forced_has_starter_won(slice_index leaf);

/* Determine whether the defender wins after a move by the attacker
 * @param leaf identifies leaf
 * @return true iff the defender wins
 */
boolean leaf_forced_does_defender_win(slice_index leaf);

/* Determine whether a leaf slice.has just been solved with the move
 * by the non-starter 
 * @param leaf slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean leaf_forced_has_non_starter_solved(slice_index leaf);

/* Write a move by the non-starter that has reached a leaf's goal
 * @param leaf slice index of leaf
 */
void leaf_forced_write_non_starter_has_solved(slice_index leaf);

/* Determine and write the postkey play after the move that has just
 * been played in the current ply.
 * We have already determined that >=1 move reaching the goal is forced
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean leaf_forced_solve_postkey(slice_index leaf);

/* Solve at non-root level
 * @param leaf slice index
 * @return true iff >=1 solution was found
 */
boolean leaf_forced_solve(slice_index leaf);

/* Determine and write the solution of a leaf forced slice at root level
 * @param leaf identifies leaf slice
 * @return true iff >=1 solution was found
 */
boolean leaf_forced_root_solve(slice_index leaf);

/* Find refutations after a move of the attacking side at root level.
 * @param refutations table where to store refutations
 * @param si slice index
 * @return attacker_has_reached_deadend if we are in a situation where
 *            the attacking move is to be considered to have failed, e.g.:
 *            if the defending side is immobile and shouldn't be
 *            if some optimisation tells us so
 *         attacker_has_solved_next_slice if the attacking move has solved the branch
 *         found_refutations if refutations contains some refutations
 *         found_no_refutation otherwise
 */
unsigned int leaf_forced_root_find_refutations(table refutations,
                                               slice_index leaf);

/* Solve postkey play at root level.
 * @param leaf slice index
 * @return true iff >=1 solution was found
 */
boolean leaf_forced_root_solve_postkey(slice_index leaf);

/* Detect starter field with the starting side if possible. 
 * @param leaf identifies leaf
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter leaf_forced_detect_starter(slice_index leaf,
                                                  boolean same_side_as_root);

#endif
