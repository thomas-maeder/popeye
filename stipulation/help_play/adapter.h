#if !defined(STIPULATION_HELP_PLAY_ADAPTER_H)
#define STIPULATION_HELP_PLAY_ADAPTER_H

#include "py.h"
#include "pyslice.h"

/* This module provides functionality dealing with STHelpAdapter
 * stipulation slices. STHelpAdapter slices switch from general play to help
 * play.
 */

/* Allocate a STHelpAdapter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_help_adapter_slice(stip_length_type length,
                                     stip_length_type min_length);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type help_adapter_solve(slice_index si);

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type help_adapter_has_solution(slice_index si);

#endif
