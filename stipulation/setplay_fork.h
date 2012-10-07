#if !defined(STIPULATION_SETPLAY_FORK_H)
#define STIPULATION_SETPLAY_FORK_H

/* Functionality related to STSetplayFork slices
 */

#include "stipulation/pipe.h"
#include "solving/solve.h"

/* Allocate a STSetplayFork slice
 * @param set entry branch of set play
 * @return newly allocated slice
 */
slice_index alloc_setplay_fork_slice(slice_index set);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type setplay_fork_solve(slice_index si, stip_length_type n);

#endif
