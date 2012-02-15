#if !defined(CONDITIONS_ANTICIRCE_EXCHANGE_SPECIAL_H)
#define CONDITIONS_ANTICIRCE_EXCHANGE_SPECIAL_H

#include "pyslice.h"

/* This module provides slice type STAnticirceExchangeSpecial - detects
 * exchanges that end on the rebirth of the piece fulfilling the exchange
 */

/* Allocate a STAnticirceExchangeSpecial slice.
 * @return index of allocated slice
 */
slice_index alloc_anticirce_exchange_special_slice(void);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type anticirce_exchange_special_solve(slice_index si);

#endif
