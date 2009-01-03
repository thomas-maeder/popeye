#if !defined(PYLEAF_H)
#define PYLEAF_H

#include "boolean.h"
#include "pystip.h"
#include "pycompos.h"
#include "py.h"
#include "pyhash.h"

/* This module provides functionality dealing with leaf stipulation
 * slices.
 */

/* Determine whether a side has reached the goal of a leaf slice.
 * @param just_moved side that has just moved
 * @param leaf slice index of leaf slice
 * @return true iff just_moved has reached leaf's goal
 */
boolean leaf_is_goal_reached(Side just_moved, slice_index leaf);

/* Detect a priori unsolvability of a leaf (e.g. because of forced
 * reflex mates)
 * @param n number of moves to go
 * @param leaf leaf's slice index
 * @return true iff leaf is a priori unsolvable
 */
boolean leaf_is_unsolvable(slice_index leaf);

/* Determine whether the defender has directly lost with his move just
 * played. 
 * Assumes that there is no short win for the defending side.
 * @param si slice identifier
 * @return true iff the defending side has directly lost
 */
boolean d_leaf_has_defender_lost(slice_index leaf);

/* Determine whether a leaf slice.has just been solved with the just
 * played move by the starter 
 * @param leaf slice index
 * @return true iff the leaf's starter has just solved leaf
 */
boolean leaf_is_solved(slice_index leaf);

/* Write a priori unsolvability (if any) of a leaf (e.g. forced reflex
 * mates).
 * Assumes leaf_is_unsolvable()
 * @param leaf leaf's slice index
 */
void leaf_write_unsolvability(slice_index leaf);

/* Find and write set play
 * @param leaf slice index
 * @return true iff >= 1 set play was found
 */
boolean leaf_root_solve_setplay(slice_index leaf);

/* Find and write set play provided every set move leads to end
 * @param leaf slice index
 * @return true iff every defender's move leads to end
 */
boolean leaf_root_solve_complete_set(slice_index leaf);

/* Find and write variations (i.e. nothing resp. defender's final
 * moves). 
 * @param leaf slice index
 */
void d_leaf_solve_variations(slice_index leaf);

/* Write the key just played, solve the post key play (threats,
 * variations)
 * @param leaf slice index
 * @param type type of attack
 */
void d_leaf_root_write_key_solve_postkey(slice_index leaf, attack_type type);

/* Find and write continuations (i.e. mating moves or final move pairs).
 * @param continuations table where to append continuations found and
 *                      written
 * @param leaf slice index
 */
void d_leaf_solve_continuations(int continuations, slice_index leaf);

/* Determine whether the attacking side has immediately lost with its
 * move just played.
 * @param leaf slice identifier
 * @return true iff attacker has lost
 */
boolean d_leaf_has_attacker_lost(slice_index leaf);

/* Determine whether the attacking side has immediately won with its
 * move just played.
 * @param leaf slice identifier
 * @return true iff attacker has lost
 */
boolean d_leaf_has_attacker_won(slice_index leaf);

/* Determine whether there is a solution in a leaf.
 * @param leaf slice index of leaf slice
 * @return true iff leaf has >=1 solution
 */
boolean leaf_has_solution(slice_index leaf);

/* Determine and write the solution of a leaf slice.
 * @param leaf identifies leaf slice
 * @return true iff >=1 solution was found
 */
boolean leaf_solve(slice_index leaf);

/* Determine whether there is >= 1 solution for the leaf
 * @param leaf slice index of leaf slice
 * @return true iff side_at_move has >=1 solution
 */
boolean leaf_is_solvable(slice_index leaf);

/* Detect starter field with the starting side if possible. 
 * @param leaf identifies leaf
 * @param is_duplex is this for duplex?
 */
void leaf_detect_starter(slice_index leaf, boolean is_duplex);

/* Impose the starting side on a leaf. 
 * @param leaf identifies leaf
 * @param s starting side of leaf
 */
void leaf_impose_starter(slice_index leaf, Side s);

#endif
