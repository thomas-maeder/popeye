#if !defined(CONDITIONS_ANTICIRCE_CIRCUIT_SPECIAL_H)
#define CONDITIONS_ANTICIRCE_CIRCUIT_SPECIAL_H

#include "pyslice.h"

/* This module provides slice type STAnticirceCircuitSpecial - detects circuits
 * that end on the rebirth of the piece fulfilling the circuit
 */

/* Allocate a STAnticirceCircuitSpecial slice.
 * @return index of allocated slice
 */
slice_index alloc_anticirce_circuit_special_slice(void);

/* Determine whether a slice has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type anticirce_circuit_special_has_solution(slice_index si);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type anticirce_circuit_special_solve(slice_index si);

#endif
