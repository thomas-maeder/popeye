#if !defined(SOLVING_BATTLE_PLAY_MIN_LENGTH_H)
#define SOLVING_BATTLE_PLAY_MIN_LENGTH_H

/* make sure that the minimum length of a branch is respected
 */

#include "solving/solve.h"

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type min_length_guard_solve(slice_index si, stip_length_type n);

/* Instrument the stipulation with minimum length functionality
 * @param si identifies the root slice of the stipulation
 */
void stip_insert_min_length(slice_index si);

#endif
