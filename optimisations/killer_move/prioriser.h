#if !defined(OPTIMISATION_KILLER_MOVE_PRIORISER_H)
#define OPTIMISATION_KILLER_MOVE_PRIORISER_H

#include "solving/solve.h"

/* Allocate a STKillerMovePrioriser slice.
 * @return index of allocated slice
 */
slice_index alloc_killer_move_prioriser_slice(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type killer_move_prioriser_solve(slice_index si, stip_length_type n);

#endif
