#if !defined(PYNOT_H)
#define PYNOT_H

#include "py.h"
#include "pystip.h"
#include "boolean.h"

/* This module provides functionality dealing with STNot stipulation
 * slices.
 */

/* Allocate a not slice.
 * @param op 1st operand
 * @return index of allocated slice
 */
slice_index alloc_not_slice(slice_index op);

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * Assumes slice_is_apriori_unsolvable(si)
 * @param si slice index
 */
void not_write_unsolvability(slice_index si);

/* Determine whether there is >= 1 solution
 * @param si slice index
 * @return true iff there is >=1 solution
 */
boolean not_is_solvable(slice_index si);

/* Detect a priori unsolvability (e.g. because of forced
 * reflex mates)
 * @param si slice index
 * @return true iff slice is a priori unsolvable
 */
boolean not_is_apriori_unsolvable(slice_index si);

/* Determine and write the solution
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean not_solve(slice_index si);

/* Determine whether a slice has a solution
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean not_has_solution(slice_index si);

/* Determine and write continuations of a slice
 * @param continuations table where to store continuing moves
 *                      (e.g. threats)
 * @param si index of slice
 */
void not_solve_continuations(int continuations, slice_index si);

/* Determine whether the starting side has lost with its move just
 * played independently of his possible further play during the
 * current slice.
 * @param si slice identifier
 * @return true iff starter has lost
 */
boolean not_has_starter_lost(slice_index si);

/* Determine whether the attacker has won with his move just played
 * @param si slice identifier
 * @return true iff the starter has won
 */
boolean not_has_starter_won(slice_index si);

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param is_duplex is this for duplex?
 */
void not_detect_starter(slice_index si, boolean is_duplex);

/* Impose the starting side on a slice.
 * @param si identifies slice
 * @param s starting side of slice
 */
void not_impose_starter(slice_index si, Side s);

/* Determine and write the solution of a slice
 * @param slice index
 * @return true iff >=1 move pair was found
 */
boolean not_root_solve(slice_index si);

#endif
