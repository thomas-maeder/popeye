#if !defined(OPTIMISATIONS_KEEPMATING_H)
#define OPTIMISATIONS_KEEPMATING_H

/* Implementation of the "keep mating piece" optimisation:
 * Solving stops once the last piece of the mating side that could
 * deliver mate has been captured.
 */

#include "solving/solve.h"

/* Instrument stipulation with STKeepMatingFilter slices
 * @param si identifies slice where to start
 */
void stip_insert_keepmating_filters(slice_index si);


/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
keepmating_filter_solve(slice_index si, stip_length_type n);

#endif
