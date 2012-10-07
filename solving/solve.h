#if !defined(SOLVING_BATTLE_PLAY_ATTACK_PLAY_H)
#define SOLVING_BATTLE_PLAY_ATTACK_PLAY_H

/* Interface for dynamically dispatching solve operations to slices
 * depending on the slice type
 */

#include "py.h"

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type solve(slice_index si, stip_length_type n);

#endif
