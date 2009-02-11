#if !defined(PYLEAFS_H)
#define PYLEAFS_H

#include "boolean.h"
#include "pyslice.h"
#include "pytable.h"
#include "py.h"

/* This module provides functionality dealing with leaf stipulation
 * slices.
 */

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param si slice index
 * @return true iff starter must resign
 */
boolean leaf_s_must_starter_resign(slice_index leaf);

/* Determine whether a leaf slice has just been solved with the just
 * played move by the non-starter 
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean leaf_s_has_non_starter_solved(slice_index leaf);

/* Spin off a set play slice
 * @param si slice index
 * @return set play slice spun off; no_slice if not applicable
 */
slice_index leaf_s_root_make_setplay_slice(slice_index leaf);

/* Find and write post key play
 * @param leaf slice index
 */
void leaf_s_solve_postkey(slice_index leaf);

/* Write the key just played
 * @param leaf slice index
 * @param type type of attack
 */
void leaf_s_root_write_key(slice_index leaf, attack_type type);

/* Solve the post key play
 * @param refutations table containing the refutations (if any)
 * @param leaf slice index
 */
void leaf_s_root_solve_postkey(table refutations, slice_index leaf);

/* Find and write continuations and append them to the top table
 * @param leaf slice index
 */
void leaf_s_solve_continuations(slice_index leaf);

/* Determine whether the starting side has made such a bad move that
 * it is clear without playing further that it is not going to win.
 * E.g. in s# or r#, has it taken the last potential mating piece of
 * the defender?
 * @param leaf slice identifier
 * @return true iff starter has lost
 */
boolean leaf_s_has_starter_apriori_lost(slice_index leaf);

/* Determine whether the starting side has won with its move just
 * played.
 * @param leaf slice identifier
 * @return true iff starter has won
 */
boolean leaf_s_has_starter_won(slice_index leaf);

/* Determine whether the attacker has reached slice si's goal with his
 * move just played.
 * @param si slice identifier
 * @return true iff the starter reached the goal
 */
boolean leaf_s_has_starter_reached_goal(slice_index leaf);

/* Determine whether there is a solution in a leaf.
 * @param leaf slice index of leaf slice
 * @return true iff leaf has >=1 solution
 */
boolean leaf_s_has_solution(slice_index leaf);

/* Determine and write the solution of a leaf slice.
 * @param leaf identifies leaf slice
 * @return true iff >=1 solution was found
 */
boolean leaf_s_solve(slice_index leaf);

/* Determine and write the solution of a leaf slice at root level.
 * @param leaf identifies leaf slice
 */
void leaf_s_root_solve(slice_index leaf);

/* Detect starter field with the starting side if possible. 
 * @param leaf identifies leaf
 * @param is_duplex is this for duplex?
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter leaf_s_detect_starter(slice_index leaf,
                                             boolean is_duplex,
                                             boolean same_side_as_root);

/* Impose the starting side on a leaf. 
 * @param leaf identifies leaf
 * @param s starting side of leaf
 */
void leaf_s_impose_starter(slice_index leaf, Side s);

/* Write a move by the non-starter that has reached a leaf's goal
 * @param leaf slice index of leaf
 */
void leaf_s_write_non_starter_has_solved(slice_index leaf);

#endif
