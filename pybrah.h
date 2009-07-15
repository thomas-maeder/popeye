#if !defined(PYBRAH_H)
#define PYBRAH_H

#include "py.h"
#include "pystip.h"
#include "pyslice.h"
#include "pytable.h"

/* This module provides functionality dealing with STBranchHelp
 * stipulation slices.
 */

/* Allocate a STBranchHelp slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies next slice
 * @return index of allocated slice
 */
slice_index alloc_branch_h_slice(stip_length_type length,
                                 stip_length_type min_length,
                                 slice_index next);

/* Wrap the branch(es) where play starts with a root slice.
 * Set st->param to the newly created slice.
 * @param branch identifies the branch slice to be wrapped
 * @param st address of structure holding traversal state
 * @return true
 */
boolean branch_h_make_root_slice(slice_index branch, slice_traversal *st);

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param si slice index
 * @return true iff starter must resign
 */
boolean branch_h_must_starter_resign(slice_index si);

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * Assumes slice_must_starter_resign(si)
 * @param si slice index
 */
void branch_h_write_unsolvability(slice_index si);

/* Determine whether a branch slice.has just been solved with the
 * just played move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean branch_h_has_non_starter_solved(slice_index si);

/* Determine whether the starting side has made such a bad move that
 * it is clear without playing further that it is not going to win.
 * E.g. in s# or r#, has it taken the last potential mating piece of
 * the defender?
 * @param si slice identifier
 * @return true iff starter has lost
 */
boolean branch_h_has_starter_apriori_lost(slice_index si);

/* Determine whether the attacker has won with his move just played
 * @param si slice identifier
 * @return true iff the starter has won
 */
boolean branch_h_has_starter_won(slice_index si);

/* Determine whether the attacker has reached slice si's goal with his
 * move just played.
 * @param si slice identifier
 * @return true iff the starter reached the goal
 */
boolean branch_h_has_starter_reached_goal(slice_index si);

/* Determine whether a side has reached the goal
 * @param just_moved side that has just moved
 * @param si slice index
 * @return true iff just_moved has reached the goal
 */
boolean branch_h_is_goal_reached(Side just_moved, slice_index si);

/* Determine whether a slice has a solution
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean branch_h_has_solution(slice_index si);

/* Write the key just played
 * @param si slice index
 * @param type type of attack
 */
void branch_h_root_write_key(slice_index si, attack_type type);

/* Find and write post key play
 * @param leaf slice index
 */
void branch_h_solve_postkey(slice_index si);

/* Solve a branch slice at non-root level.
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean branch_h_solve(slice_index si);

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @param side_at_move side at move
 * @return true iff >= 1 solution has been found
 */
boolean branch_h_solve_in_n(slice_index si,
                            stip_length_type n,
                            Side side_at_move);

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @param side_at_move side at move
 * @return true iff >= 1 solution has been found
 */
boolean branch_h_has_solution_in_n(slice_index si,
                                   stip_length_type n,
                                   Side side_at_move);

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @param side_at_move side at move
 */
void branch_h_solve_continuations_in_n(table continuations,
                                       slice_index si,
                                       stip_length_type n,
                                       Side side_at_move);

/* Determine and write continuations of a slice
 * @param continuations table where to store continuing moves (i.e. threats)
 * @param si index of branch slice
 */
void branch_h_solve_continuations(table continuations, slice_index si);

/* Determine the starting side in a help branch in n
 * @param si slice index
 * @param n number of half-moves
 */
Side branch_h_starter_in_n(slice_index si, stip_length_type n);

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter branch_h_detect_starter(slice_index si,
                                               boolean same_side_as_root);

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean branch_h_impose_starter(slice_index si, slice_traversal *st);

/* Allocate a STHelpRoot slice.
 * @param si STBranchHelp slice to "wrap"
 * @return index of allocated slice
 */
slice_index alloc_help_root_slice(slice_index si);

/* Shorten a help branch by a half-move. If the branch represents a
 * half-move only, deallocates the branch.
 * @param si identifies the branch
 * @return if the branch slice represents a half-move only, the slice
 *         representing the subsequent play; otherwise si
 */
slice_index help_root_shorten_help_play(slice_index si);

/* Spin off a set play slice at root level
 * @param si slice index
 * @return set play slice spun off; no_slice if not applicable
 */
slice_index help_root_make_setplay_slice(slice_index si);

/* Solve a branch slice at root level.
 * @param si slice index
 * @return no_slice if set play not applicable
 *         new root slice index (may be equal to old one) otherwise
 * @return true iff >=1 solution was found
 */
boolean help_root_solve(slice_index si);

/* Solve a branch in exactly n moves at root level
 * @param si slice index
 * @param n exact number of moves
 * @return true iff >=1 solution was found
 */
boolean help_root_solve_in_n(slice_index si, stip_length_type n);

#endif
