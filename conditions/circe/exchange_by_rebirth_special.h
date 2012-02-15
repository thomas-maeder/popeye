#if !defined(STIPULATION_CIRCE_EXCHANGE_B_SPECIAL_H)
#define STIPULATION_CIRCE_EXCHANGE_B_SPECIAL_H

#include "pyslice.h"

/* This module provides slice type STCirceExchangeSpecial - detects
 * place exchanges by a reborn captured piece
 */

/* Allocate a STCirceExchangeSpecial slice.
 * @return index of allocated slice
 */
slice_index alloc_circe_exchange_special_slice(void);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type circe_exchange_special_solve(slice_index si);

#endif
