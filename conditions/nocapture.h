#if !defined(CONDITIONS_NOCAPTURE_H)
#define CONDITIONS_NOCAPTURE_H

#include "utilities/boolean.h"
#include "py.h"

/* This module implements the condition Nocapture */

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type nocapture_remove_captures_solve(slice_index si,
                                                 stip_length_type n);

/* Instrument the solvers with Nocapture
 * @param si identifies the root slice of the stipulation
 */
void stip_insert_nocapture(slice_index si);

#endif
