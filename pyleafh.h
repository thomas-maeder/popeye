#if !defined(PYLEAFH_H)
#define PYLEAFH_H

#include "boolean.h"
#include "pystip.h"
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
boolean leaf_h_must_starter_resign(slice_index leaf);

/* Determine whether a leaf slice has just been solved with the just
 * played move by the non-starter 
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean leaf_h_has_non_starter_solved(slice_index leaf);

/* Find and write set play
 * @param leaf slice index
 * @return true iff >= 1 set play was found
 */
boolean leaf_h_root_solve_setplay(slice_index leaf);

/* Find and write set play provided every set move leads to end
 * @param leaf slice index
 * @return true iff every defender's move leads to end
 */
boolean leaf_h_root_solve_complete_set(slice_index leaf);

/* Find and write variations (i.e. nothing resp. defender's final
 * moves). 
 * @param leaf slice index
 */
void leaf_h_solve_variations(slice_index leaf);

/* Write the key just played, then solve the post key play
 * @param leaf slice index
 * @param type type of attack
 */
void leaf_h_root_write_key_solve_postkey(slice_index leaf, attack_type type);

/* Find and write continuations (i.e. mating moves or final move pairs).
 * @param continuations table where to append continuations found and
 *                      written
 * @param leaf slice index
 */
void leaf_h_solve_continuations(int continuations, slice_index leaf);

/* Determine whether the starting side has made such a bad move that
 * it is clear without playing further that it is not going to win.
 * E.g. in s# or r#, has it taken the last potential mating piece of
 * the defender?
 * @param leaf slice identifier
 * @return true iff starter has lost
 */
boolean leaf_h_has_starter_apriori_lost(slice_index leaf);

/* Determine whether the starting side has won with its move just
 * played.
 * @param leaf slice identifier
 * @return true iff starter has won
 */
boolean leaf_h_has_starter_won(slice_index leaf);

/* Determine whether the attacker has reached slice si's goal with his
 * move just played.
 * @param si slice identifier
 * @return true iff the starter reached the goal
 */
boolean leaf_h_has_starter_reached_goal(slice_index si);

/* Determine whether there is a solution in a leaf.
 * @param leaf slice index of leaf slice
 * @return true iff leaf has >=1 solution
 */
boolean leaf_h_has_solution(slice_index leaf);

/* Determine and write the solution of a leaf slice.
 * @param leaf identifies leaf slice
 * @return true iff >=1 solution was found
 */
boolean leaf_h_solve(slice_index leaf);

/* Detect starter field with the starting side if possible. 
 * @param leaf identifies leaf
 * @param is_duplex is this for duplex?
 */
void leaf_h_detect_starter(slice_index leaf, boolean is_duplex);

#endif
