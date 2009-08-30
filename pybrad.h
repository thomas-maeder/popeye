#if !defined(PYBRAD_H)
#define PYBRAD_H

#include "py.h"
#include "pystip.h"
#include "pyslice.h"
#include "pydirect.h"
#include "pytable.h"

/* This module provides functionality dealing with the attacking side
 * in STBranchDirect stipulation slices.
 */

/* Allocate a branch that represents direct play
 * @param level is this a top-level branch or one nested into another
 *              branch?
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies next slice
 * @return index of adapter slice
 */
slice_index alloc_direct_branch(branch_level level,
                                stip_length_type length,
                                stip_length_type min_length,
                                slice_index next);

/* Allocate a nested (i.e. non toplevel) direct branch
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param towards_goal identifies slice leading towards goal
 * @return identifier for entry slice of allocated branch
 */
slice_index alloc_nested_direct_branch(stip_length_type length,
                                       stip_length_type min_length,
                                       slice_index towards_goal);

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @param n maximum number of moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean branch_d_are_threats_refuted_in_n(table threats,
                                          stip_length_type len_threat,
                                          slice_index si,
                                          stip_length_type n,
                                          int curr_max_nr_nontrivial);

/* Determine whether a branch slice has a solution
 * @param si slice index
 * @param n maximal number of moves
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type branch_d_has_solution_in_n(slice_index si,
                                             stip_length_type n,
                                             int curr_max_nr_nontrivial);

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type branch_d_has_solution(slice_index si);

/* Determine and write the continuations in the current position
 * (i.e. attacker's moves winning after a defender's move that refuted
 * the threat).
 * @param continuations table where to store continuing moves (i.e. threats)
 * @param si slice index
 * @param n maximum maximal number of moves
 */
void branch_d_solve_continuations_in_n(table continuations,
                                       slice_index si,
                                       stip_length_type n);

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * @param si slice index
 */
void branch_d_write_unsolvability(slice_index si);

/* Determine whether the attacker has won with his move just played
 * independently of the non-starter's possible further play during the
 * current slice.
 * @param si slice identifier
 * @return whether the starter has won
 */
has_starter_won_result_type branch_d_has_starter_won(slice_index si);

/* Determine whether a side has reached the goal
 * @param just_moved side that has just moved
 * @param si slice index
 * @return true iff just_moved has reached the goal
 */
boolean branch_d_is_goal_reached(Side just_moved, slice_index si);

/* Determine whether a slice.has just been solved with the just played
 * move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean branch_d_has_non_starter_solved(slice_index si);

/* Determine whether the attacker has reached slice si's goal with his
 * move just played.
 * @param si slice identifier
 * @return true iff the starter reached the goal
 */
boolean branch_d_has_starter_reached_goal(slice_index si);

/* Determine and write the continuations in the current position
 * (i.e. attacker's moves winning after a defender's move that refuted
 * the threat).
 * @param continuations table where to store continuing moves (i.e. threats)
 * @param si slice index
 */
void branch_d_solve_continuations(table continuations, slice_index si);

/* Write the key just played
 * @param si slice index
 * @param type type of attack
 */
void branch_d_root_write_key(slice_index si, attack_type type);

/* Solve a branch slice at non-root level.
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean branch_d_solve(slice_index si);

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter branch_d_detect_starter(slice_index si,
                                               boolean same_side_as_root);

/* Spin off a set play slice at root level
 * @param si slice index
 * @return set play slice spun off; no_slice if not applicable
 */
slice_index direct_root_make_setplay_slice(slice_index si);

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @return index of first postkey slice; no_slice if postkey play not
 *         applicable
 */
slice_index direct_root_reduce_to_postkey_play(slice_index si);

/* Solve a branch slice at root level.
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean direct_root_solve(slice_index si);

#endif
