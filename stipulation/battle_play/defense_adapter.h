#if !defined(STIPULATION_BATTLE_PLAY_DEFENSE_ADAPTER_H)
#define STIPULATION_BATTLE_PLAY_DEFENSE_ADAPTER_H

#include "stipulation/battle_play/defense_play.h"
#include "stipulation/series_play/play.h"

/* STDefenseAdapter slices switch from generic solving to defense solving.
 */

/* Allocate a STDefenseAdapter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_defense_adapter_slice(stip_length_type length,
                                        stip_length_type min_length);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type defense_adapter_solve(slice_index si);

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type defense_adapter_has_solution(slice_index si);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type defense_adapter_series(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type defense_adapter_has_series(slice_index si, stip_length_type n);

#endif
