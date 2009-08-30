#if !defined(PYQUODLI_H)
#define PYQUODLI_H

#include "py.h"
#include "pyslice.h"
#include "boolean.h"

/* This module provides functionality dealing with quodlibet
 * (i.e. logical OR) stipulation slices.
 */

/* Construct a quodlibet slice over an already allocated slice object
 * @param si index of slice object where to construct quodlibet slice
 * @param op1 1st operand
 * @param op2 2nd operand
 */
void make_quodlibet_slice(slice_index si, slice_index op1, slice_index op2);

/* Allocate a quodlibet slice.
 * @param op1 1st operand
 * @param op2 2nd operand
 * @return index of allocated slice
 */
slice_index alloc_quodlibet_slice(slice_index op1, slice_index op2);

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * @param si slice index
 */
void quodlibet_write_unsolvability(slice_index si);

/* Determine and write continuations of a quodlibet slice
 * @param continuations table where to store continuing moves
 *                      (e.g. threats)
 * @param si index of quodlibet slice
 */
void quodlibet_solve_continuations(table continuations, slice_index si);

/* Write the key just played
 * @param si slice index
 * @param type type of attack
 */
void quodlibet_root_write_key(slice_index si, attack_type type);

/* Find and write post key play
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean quodlibet_solve_postkey(slice_index si);

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param si slice index
 * @param n number of moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean quodlibet_are_threats_refuted(table threats, slice_index si);

/* Determine whether a quodlibet slice jas a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type quodlibet_has_solution(slice_index si);

/* Determine whether a quodlibet slice.has just been solved with the
 * just played move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean quodlibet_has_non_starter_solved(slice_index si);

/* Determine whether the attacker has won with his move just played
 * independently of the non-starter's possible further play during the
 * current slice.
 * @param si slice identifier
 * @return whether the starter has won
 */
has_starter_won_result_type quodlibet_has_starter_won(slice_index si);

/* Determine whether the attacker has reached slice si's goal with his
 * move just played.
 * @param si slice identifier
 * @return true iff the starter reached the goal
 */
boolean quodlibet_has_starter_reached_goal(slice_index si);

/* Determine whether the defender wins after a move by the attacker
 * @param si slice index
 * @return true iff defender wins
 */
boolean quodlibet_does_defender_win(slice_index si);

/* Solve a quodlibet slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean quodlibet_solve(slice_index si);

/* Solve a slice in exactly n moves at root level
 * @param si slice index
 * @param n exact number of moves
 */
void quodlibet_root_solve_in_n(slice_index si, stip_length_type n);

/* Solve a quodlibet slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean quodlibet_root_solve(slice_index si);

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter quodlibet_detect_starter(slice_index si,
                                                boolean same_side_as_root);

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean quodlibet_impose_starter(slice_index si, slice_traversal *st);

#endif
