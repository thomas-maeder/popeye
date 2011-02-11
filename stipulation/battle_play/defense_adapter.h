#if !defined(STIPULATION_BATTLE_PLAY_DEFENSE_ADAPTER_H)
#define STIPULATION_BATTLE_PLAY_DEFENSE_ADAPTER_H

#include "stipulation/battle_play/defense_play.h"

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

#endif
