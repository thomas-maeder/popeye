#if !defined(STIPULATION_BATTLE_PLAY_DEFENSE_ROOT_H)
#define STIPULATION_BATTLE_PLAY_DEFENSE_ROOT_H

#include "boolean.h"
#include "pystip.h"
#include "pyslice.h"


/* This module provides functionality dealing with the attacking side
 * in STDefenseRoot stipulation slices.
 */

/* Allocate a STDefenseRoot slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_defense_root_slice(stip_length_type length,
                                     stip_length_type min_length);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type defense_root_solve(slice_index si);

#endif
