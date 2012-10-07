#if !defined(SOLVING_PLAY_SELFCHECK_GUARD_H)
#define SOLVING_PLAY_SELFCHECK_GUARD_H

/* STSelfCheckGuard slice - stops solutions with moves that expose the
 * own king
 */

#include "solving/solve.h"

/* Instrument a stipulation with slices dealing with selfcheck detection
 * @param si root of branch to be instrumented
 */
void stip_insert_selfcheck_guards(slice_index si);

/* Allocate a STSelfCheckGuard slice
 * @return allocated slice
 */
slice_index alloc_selfcheck_guard_slice(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type selfcheck_guard_solve(slice_index si, stip_length_type n);

#endif
