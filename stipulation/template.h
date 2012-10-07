#if !defined(TEMPLATE_H)
#define TEMPLATE_H

#include "solving/solve.h"

/* This module serves as template for pipe slices */

/* Allocate a STTemplate slice.
 * @return index of allocated slice
 */
slice_index alloc_template_slice(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type template_solve(slice_index si, stip_length_type n);

#endif
