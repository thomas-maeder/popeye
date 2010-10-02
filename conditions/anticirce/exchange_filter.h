#if !defined(CONDITIONS_ANTICIRCE_EXCHANGE_FILTER_H)
#define CONDITIONS_ANTICIRCE_EXCHANGE_FILTER_H

#include "pyslice.h"

/* This module provides slice type STAnticirceExchangeFilter - prevent
 * move sequences terminated by a capture from accidentally passing the orthodox
 * test for the exchange goal.
 */

/* Allocate a STAnticirceExchangeFilter slice.
 * @return index of allocated slice
 */
slice_index alloc_anticirce_exchange_filter_slice(void);

/* Determine whether a slice has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type anticirce_exchange_filter_has_solution(slice_index si);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type anticirce_exchange_filter_solve(slice_index si);

#endif
