#if !defined(PYLEAFD_H)
#define PYLEAFD_H

#include "boolean.h"
#include "pyslice.h"
#include "pydirect.h"
#include "pytable.h"
#include "py.h"

/* This module provides functionality dealing with direct leaf
 * stipulation slices.
 */

/* Determine whether a leaf slice has just been solved with the just
 * played move by the non-starter 
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean leaf_d_has_non_starter_solved(slice_index leaf);

/* Find and write post key play
 * @param leaf slice index
 * @return true iff >=1 solution was found
 */
boolean leaf_d_solve_postkey(slice_index leaf);

/* Write the key just played
 * @param leaf slice index
 * @param type type of attack
 */
void leaf_d_root_write_key(slice_index leaf, attack_type type);

/* Find and write continuations and append them to the top table
 * @param leaf slice index
 */
void leaf_d_solve_continuations(slice_index leaf);

/* Determine whether the starting side has won with its move just
 * played.
 * @param leaf slice identifier
 * @return whether starter has won
 */
has_starter_won_result_type leaf_d_has_starter_won(slice_index leaf);

/* Determine whether the attacker has reached slice si's goal with his
 * move just played.
 * @param si slice identifier
 * @return true iff the starter reached the goal
 */
boolean leaf_d_has_starter_reached_goal(slice_index si);

/* Determine whether the defender wins after a move by the attacker
 * @param leaf identifies leaf
 * @return true iff the defender wins
 */
boolean leaf_d_does_defender_win(slice_index leaf);

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param leaf slice index
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean leaf_d_are_threats_refuted(table threats, slice_index leaf);

/* Determine whether there is a solution in a leaf.
 * @param leaf slice index of leaf slice
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type leaf_d_has_solution(slice_index leaf);

/* Determine and write keys at root level
 * @param leaf leaf's slice index
 * @return true iff >=1 key was found and written
 */
boolean leaf_d_root_solve(slice_index leaf);

/* Solve postkey play at root level.
 * @param leaf slice index
 * @return true iff >=1 solution was found
 */
boolean leaf_d_root_solve_postkey(slice_index leaf);

/* Find refutations after a move of the attacking side at root level.
 * @param t table where to store refutations
 * @param si slice index
 * @return attacker_has_reached_deadend if we are in a situation where
 *            the attacking move is to be considered to have failed, e.g.:
 *            if the defending side is immobile and shouldn't be
 *            if some optimisation tells us so
 *         attacker_has_solved_next_slice if the attacking move has
 *            solved the branch
 *         found_refutations if refutations contains some refutations
 *         found_no_refutation otherwise
 */
quantity_of_refutations_type leaf_d_root_find_refutations(slice_index leaf);

/* Determine and write the solution of a leaf slice.
 * @param leaf identifies leaf slice
 * @return true iff >=1 solution was found
 */
boolean leaf_d_solve(slice_index leaf);

/* Write a priori unsolvability (if any) of a leaf (e.g. forced reflex
 * mates)
 * @param leaf leaf's slice index
 */
void leaf_d_write_unsolvability(slice_index leaf);

/* Detect starter field with the starting side if possible. 
 * @param leaf identifies leaf
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter leaf_d_detect_starter(slice_index leaf,
                                             boolean same_side_as_root);

#endif
