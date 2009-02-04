#if !defined(PYRECIPR_H)
#define PYRECIPR_H

#include "boolean.h"
#include "py.h"
#include "pyslice.h"

/* This module provides functionality dealing with reciprocal
 * (i.e. logical AND) stipulation slices.
 */

/* Allocate a reciprocal slice.
 * @param op1 1st operand
 * @param op2 2nd operand
 * @return index of allocated slice
 */
slice_index alloc_reciprocal_slice(slice_index op1, slice_index op2);

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param si slice index
 * @return true iff starter must resign
 */
boolean reci_must_starter_resign(slice_index si);

/* Determine whether there is a solution at the end of a reciprocal
 * slice. 
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean reci_has_solution(slice_index si);

/* Determine whether a reciprocal slice.has just been solved with the
 * just played move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean reci_has_non_starter_solved(slice_index si);

/* Determine whether the starting side has made such a bad move that
 * it is clear without playing further that it is not going to win.
 * E.g. in s# or r#, has it taken the last potential mating piece of
 * the defender?
 * @param si slice identifier
 * @return true iff starter has lost
 */
boolean reci_has_starter_apriori_lost(slice_index si);

/* Determine whether the attacker has won with his move just played
 * independently of the non-starter's possible further play during the
 * current slice.
 * @param si slice identifier
 * @return true iff the starter has won
 */
boolean reci_has_starter_won(slice_index si);

/* Determine whether the attacker has reached slice si's goal with his
 * move just played.
 * @param si slice identifier
 * @return true iff the starter reached the goal
 */
boolean reci_has_starter_reached_goal(slice_index si);

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * Assumes slice_must_starter_resign(si)
 * @param si slice index
 */
void reci_write_unsolvability(slice_index si);

/* Find and write variations from the end of a reciprocal slice.
 * @param si slice index
 */
void reci_solve_variations(slice_index si);

/* Determine and write continuations at end of reciprocal slice
 * @param continuations table where to store continuing moves
 *                      (e.g. threats)
 * @param si index of reciprocal slice
 */
void reci_solve_continuations(int continuations, slice_index si);

/* Spin off a set play slice
 * @param si slice index
 * @return set play slice spun off; no_slice if not applicable
 */
slice_index reci_root_make_setplay_slice(slice_index si);

/* Write the key just played
 * @param si slice index
 * @param type type of attack
 */
void reci_root_write_key(slice_index si, attack_type type);

/* Continue solving after the key just played in the slice to find and
 * write the post key play (threats, variations)
 * @param si slice index
 */
void reci_root_solve_postkey(slice_index si);

/* Solve at root level at the end of a reciprocal slice
 * @param si slice index
 */
void reci_root_solve(slice_index si);

/* Continue solving at the end of a reciprocal slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean reci_solve(slice_index si);

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param is_duplex is this for duplex?
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter reci_detect_starter(slice_index si,
                                           boolean is_duplex,
                                           boolean same_side_as_root);

/* Impose the starting side on a slice.
 * @param si identifies slice
 * @param s starting side of slice
 */
void reci_impose_starter(slice_index si, Side s);

#endif
