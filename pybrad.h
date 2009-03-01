#if !defined(PYBRAD_H)
#define PYBRAD_H

#include "py.h"
#include "pystip.h"
#include "pyslice.h"
#include "pytable.h"

/* This module provides functionality dealing with the attacking side
 * in STBranchDirect stipulation slices.
 */

/* Allocate a STBranchDirect slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies next slice
 * @return index of allocated slice
 */
slice_index alloc_branch_d_slice(stip_length_type length,
                                 stip_length_type min_length,
                                 slice_index next);

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * Assumes slice_must_starter_resign(si)
 * @param si slice index
 */
void branch_d_write_unsolvability(slice_index si);

/* Determine whether a side has reached the goal
 * @param just_moved side that has just moved
 * @param si slice index
 * @return true iff just_moved has reached the goal
 */
boolean branch_d_is_goal_reached(Side just_moved, slice_index si);

/* Determine whether a branch slice has a solution
 * @param si slice index
 * @param n maximal number of moves
 * @return true iff slice si has a solution
 */
boolean branch_d_has_solution_in_n(slice_index si, stip_length_type n);

/* Determine whether a slice has a solution
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean branch_d_has_solution(slice_index si);

/* Spin off a set play slice at root level
 * @param si slice index
 * @return set play slice spun off; no_slice if not applicable
 */
slice_index branch_d_root_make_setplay_slice(slice_index si);

/* Write the key just played
 * @param si slice index
 * @param type type of attack
 */
void branch_d_root_write_key(slice_index si, attack_type type);

/* Determine and write the continuations in the current position
 * (i.e. attacker's moves winning after a defender's move that refuted
 * the threat).
 * @param continuations table where to store continuing moves (i.e. threats)
 * @param si slice index
 * @param n maximal number of moves
 */
void branch_d_solve_continuations_in_n(table continuations,
                                       slice_index si,
                                       stip_length_type n);

/* Determine and write the continuations in the current position
 * (i.e. attacker's moves winning after a defender's move that refuted
 * the threat).
 * @param continuations table where to store continuing moves (i.e. threats)
 * @param si slice index
 */
void branch_d_solve_continuations(table continuations, slice_index si);

/* Solve a branch slice at non-root level.
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean branch_d_solve(slice_index si);

/* Solve a branch slice at root level.
 * @param si slice index
 */
void branch_d_root_solve(slice_index si);

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter branch_d_detect_starter(slice_index si,
                                               boolean same_side_as_root);

/* Impose the starting side on a slice.
 * @param si identifies branch
 * @param s starting side of slice
 */
void branch_d_impose_starter(slice_index si, Side s);

#endif
