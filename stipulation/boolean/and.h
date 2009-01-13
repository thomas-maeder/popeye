#if !defined(PYRECIPR_H)
#define PYRECIPR_H

#include "boolean.h"
#include "py.h"
#include "pystip.h"

/* This module provides functionality dealing with reciprocal
 * (i.e. logical AND) stipulation slices.
 */

/* Allocate a reciprocal slice.
 * @param op1 1st operand
 * @param op2 2nd operand
 * @return index of allocated slice
 */
slice_index alloc_reciprocal_slice(slice_index op1, slice_index op2);

/* Detect a priori unsolvability of a slice (e.g. because of forced
 * reflex mates)
 * @param si slice index
 * @return true iff slice is a priori unsolvable
 */
boolean reci_is_apriori_unsolvable(slice_index si);

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

/* Determine whether the starting side has lost with its move just
 * played independently of his possible further play during the
 * current slice.
 * @param si slice identifier
 * @return true iff starter has lost
 */
boolean reci_has_starter_lost(slice_index si);

/* Determine whether the attacker has won with his move just played
 * independently of the non-starter's possible further play during the
 * current slice.
 * @param si slice identifier
 * @return true iff the starter has won
 */
boolean reci_has_starter_won(slice_index si);

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * Assumes slice_is_apriori_unsolvable(si)
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

/* Find and write set play
 * @param si slice index
 */
boolean reci_root_solve_setplay(slice_index si);

/* Write the key just played, then solve the post key play (threats,
 * variations), starting at the end of a reciprocal slice.
 * @param si slice index
 * @param type type of attack
 */
void reci_root_write_key_solve_postkey(slice_index si, attack_type type);

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
 */
void reci_detect_starter(slice_index si, boolean is_duplex);

/* Impose the starting side on a slice.
 * @param si identifies slice
 * @param s starting side of slice
 */
void reci_impose_starter(slice_index si, Side s);

#endif
