#if !defined(CONDITIONS_ANTICIRCE_CAPTURE_FORK_H)
#define CONDITIONS_ANTICIRCE_CAPTURE_FORK_H

/* bypass Circe slices if there is no capture */

#include "stipulation/slice_type.h"
#include "solving/solve.h"

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type anticirce_capture_fork_solve(slice_index si,
                                              stip_length_type n);

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_anticirce_capture_forks(slice_index si);

#endif
