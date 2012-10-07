#if !defined(SOLVING_BATTLE_PLAY_MIN_LENGTH_ATTACK_FILTER_H)
#define SOLVING_BATTLE_PLAY_MIN_LENGTH_ATTACK_FILTER_H

#include "solving/solve.h"

/* Make sure that attacks in less moves than allowed by the minimum length
 * aren't even attempted
 */

/* Allocate a STMinLengthAttackFilter defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_min_length_optimiser_slice(stip_length_type length,
                                             stip_length_type min_length);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
min_length_optimiser_solve(slice_index si, stip_length_type n);

#endif
