#if !defined(PYBRAFRK_H)
#define PYBRAFRK_H

/* Branch fork - decides when to continue play in branch and when to
 * change to slice representing subsequent play
 */

#include "pyslice.h"

/* Allocate a STBranchFork slice.
 * @param next identifies next slice
 * @param towards_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_branch_fork_slice(slice_index next,
                                    slice_index towards_goal);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n number of half moves until end state has to be reached
 * @param side_at_move side at the move
 * @return true iff >=1 solution was found
 */
boolean branch_fork_help_solve_in_n(slice_index si,
                                    stip_length_type n,
                                    Side side_at_move);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @param side_at_move side at move
 * @return true iff >= 1 solution has been found
 */
boolean branch_fork_help_has_solution_in_n(slice_index si,
                                           stip_length_type n,
                                           Side side_at_move);

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @param side_at_move side at move
 */
void branch_fork_help_solve_continuations_in_n(table continuations,
                                               slice_index si,
                                               stip_length_type n,
                                               Side side_at_move);

/* Impose the starting side on a slice.
 * @param si identifies slice
 * @param side starting side of leaf
 */
void branch_fork_impose_starter(slice_index si, Side side);

/* Determine the starter in a help stipulation in n half-moves
 * @param si identifies slice
 * @param n number of half-moves
 * @param return starting side
 */
Side branch_fork_help_starter_in_n(slice_index si, stip_length_type n);

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param si slice index
 * @return true iff starter must resign
 */
boolean branch_fork_must_starter_resign(slice_index si);

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * @param si slice index
 */
void branch_fork_write_unsolvability(slice_index si);

/* Determine whether a branch slice.has just been solved with the
 * just played move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean branch_fork_has_non_starter_solved(slice_index si);

/* Determine whether the starting side has made such a bad move that
 * it is clear without playing further that it is not going to win.
 * E.g. in s# or r#, has it taken the last potential mating piece of
 * the defender?
 * @param si slice identifier
 * @return true iff starter has lost
 */
boolean branch_fork_has_starter_apriori_lost(slice_index si);

/* Determine whether the attacker has won with his move just played
 * @param si slice identifier
 * @return true iff the starter has won
 */
boolean branch_fork_has_starter_won(slice_index si);

/* Determine whether the attacker has reached slice si's goal with his
 * move just played.
 * @param si slice identifier
 * @return true iff the starter reached the goal
 */
boolean branch_fork_has_starter_reached_goal(slice_index si);

/* Determine whether a side has reached the goal
 * @param just_moved side that has just moved
 * @param si slice index
 * @return true iff just_moved has reached the goal
 */
boolean branch_fork_is_goal_reached(Side just_moved, slice_index si);

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter branch_fork_detect_starter(slice_index si,
                                                  boolean same_side_as_root);

/* Find and write post key play
 * @param si slice index
 */
void branch_fork_solve_postkey(slice_index si);

/* Find the slice representing the play after a branch
 * @param branch identifies the branch
 * @return identifier for branch representing the play after the branch
 */
slice_index branch_find_slice_behind_fork(slice_index branch);

/* Deallocate a branch
 * @param branch identifies branch
 * @return index of slice representing the play after the branch
 */
slice_index branch_deallocate_to_fork(slice_index branch);

#endif
