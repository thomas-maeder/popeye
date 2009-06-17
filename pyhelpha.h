#if !defined(PYHELPHA_H)
#define PYHELPHA_H

/* Hashed implementation of help play functions
 */

#include "pyhelp.h"
#include "pyslice.h"

/* Allocate a STHelpHashed slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies next slice
 * @param towards_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_help_hashed_slice(stip_length_type length,
                                    stip_length_type min_length,
                                    slice_index next,
                                    slice_index towards_goal);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n number of half moves until end state has to be reached
 * @param side_at_move side at the move
 * @return true iff >=1 solution was found
 */
boolean help_hashed_solve_in_n(slice_index si,
                               stip_length_type n,
                               Side side_at_move);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @param side_at_move side at move
 * @return true iff >= 1 solution has been found
 */
boolean help_hashed_has_solution_in_n(slice_index si,
                                      stip_length_type n,
                                      Side side_at_move);

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @param side_at_move side at move
 */
void help_hashed_solve_continuations_in_n(table continuations,
                                          slice_index si,
                                          stip_length_type n,
                                          Side side_at_move);

/* Impose the starting side on a slice.
 * @param si identifies slice
 * @param side starting side of leaf
 */
void help_hashed_impose_starter(slice_index si, Side side);

/* Determine the starter in a help stipulation in n half-moves
 * @param si identifies slice
 * @param n number of half-moves
 * @param return starting side
 */
Side help_hashed_starter_in_n(slice_index si, stip_length_type n);

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param si slice index
 * @return true iff starter must resign
 */
boolean help_hashed_must_starter_resign(slice_index si);

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * @param si slice index
 */
void help_hashed_write_unsolvability(slice_index si);

/* Determine whether a branch slice.has just been solved with the
 * just played move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean help_hashed_has_non_starter_solved(slice_index si);

/* Determine whether the starting side has made such a bad move that
 * it is clear without playing further that it is not going to win.
 * E.g. in s# or r#, has it taken the last potential mating piece of
 * the defender?
 * @param si slice identifier
 * @return true iff starter has lost
 */
boolean help_hashed_has_starter_apriori_lost(slice_index si);

/* Determine whether the attacker has won with his move just played
 * @param si slice identifier
 * @return true iff the starter has won
 */
boolean help_hashed_has_starter_won(slice_index si);

/* Determine whether the attacker has reached slice si's goal with his
 * move just played.
 * @param si slice identifier
 * @return true iff the starter reached the goal
 */
boolean help_hashed_has_starter_reached_goal(slice_index si);

/* Determine whether a side has reached the goal
 * @param just_moved side that has just moved
 * @param si slice index
 * @return true iff just_moved has reached the goal
 */
boolean help_hashed_is_goal_reached(Side just_moved, slice_index si);

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter help_hashed_detect_starter(slice_index si,
                                                  boolean same_side_as_root);

/* Find and write post key play
 * @param si slice index
 */
void help_hashed_solve_postkey(slice_index si);

#endif
