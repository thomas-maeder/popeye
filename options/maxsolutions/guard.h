#if !defined(OPTIONS_MAXSOLUTIONS_GUARD_H)
#define OPTIONS_MAXSOLUTIONS_GUARD_H

#include "solving/solve.h"

/* This module provides functionality dealing with
 * STMaxSolutionsGuard stipulation slice type.
 * Slices of this type make sure that solving stops after the maximum
 * number of solutions have been found
 */

/* Allocate a STMaxSolutionsCounter slice.
 * @return allocated slice
 */
slice_index alloc_maxsolutions_counter_slice(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type maxsolutions_counter_solve(slice_index si, stip_length_type n);

/* Allocate a STMaxSolutionsGuard slice.
 * @return allocated slice
 */
slice_index alloc_maxsolutions_guard_slice(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type maxsolutions_guard_solve(slice_index si, stip_length_type n);

#endif
