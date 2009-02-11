#if !defined(PYLEAFF_H)
#define PYLEAFF_H

#include "pyslice.h"

/* This module provides functionality dealing with leaf slices that
 * find forced half-moves reaching the goal.
 */

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param leaf identifies leaf
 * @return true iff starter must resign
 */
boolean leaf_forced_must_starter_resign(slice_index leaf);

/* Determine whether the starting side has made such a bad move that
 * it is clear without playing further that it is not going to win.
 * E.g. in s# or r#, has it taken the last potential mating piece of
 * the defender?
 * @param leaf slice identifier
 * @return true iff starter has lost
 */
boolean leaf_forced_has_starter_apriori_lost(slice_index leaf);

/* Determine whether the starting side has won with its move just
 * played.
 * @param leaf slice identifier
 * @return true iff starter has won
 */
boolean leaf_forced_has_starter_won(slice_index leaf);

/* Determine whether the attacker has reached slice leaf's goal with
 * his move just played.
 * @param leaf slice identifier
 * @return true iff the starter reached the goal
 */
boolean leaf_forced_has_starter_reached_goal(slice_index leaf);

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

/* Solve at non-root level
 * @param leaf slice index
 */
void leaf_forced_solve(slice_index leaf);

/* Determine and write the solution of a leaf forced slice at root level
 * @param leaf identifies leaf slice
 * @return true iff >=1 solution was found
 */
void leaf_forced_root_solve(slice_index leaf);

/* Detect starter field with the starting side if possible. 
 * @param leaf identifies leaf
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter leaf_forced_detect_starter(slice_index leaf,
                                                  boolean same_side_as_root);

#endif
