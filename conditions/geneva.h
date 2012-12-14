#if !defined(CONDITIONS_GENEVA_H)
#define CONDITIONS_GENEVA_H

#include "py.h"

/* This module implements the condition Geneva Chess */

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type geneva_remove_illegal_captures_solve(slice_index si,
                                                      stip_length_type n);

/* Instrument the solvers with Patrol Chess
 * @param si identifies the root slice of the stipulation
 */
void stip_insert_geneva(slice_index si);

#endif
