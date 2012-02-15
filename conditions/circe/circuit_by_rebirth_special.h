#if !defined(STIPULATION_CIRCE_CIRCUIT_B_SPECIAL_H)
#define STIPULATION_CIRCE_CIRCUIT_B_SPECIAL_H

#include "pyslice.h"

/* This module provides slice type STCirceCircuitSpecial - detects
 * circuits by a reborn captured piece
 */

/* Allocate a STCirceCircuitSpecial slice.
 * @return index of allocated slice
 */
slice_index alloc_circe_circuit_special_slice(void);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type circe_circuit_special_solve(slice_index si);

#endif
