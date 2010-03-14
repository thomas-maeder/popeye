#if !defined(STIPULATION_SERIES_PLAY_FORK_H)
#define STIPULATION_SERIES_PLAY_FORK_H

/* This module provides functionality dealing with STSeriesFork
 * stipulation slices.
 */

#include "pyslice.h"
#include "pytable.h"

/* Allocate a STSeriesFork slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param to_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_series_fork_slice(stip_length_type length,
                                    stip_length_type min_length,
                                    slice_index to_goal);

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void series_fork_insert_root(slice_index si, slice_traversal *st);

/* Spin off a set play slice at root level
 * @param si slice index
 * @param st state of traversal
 */
void series_fork_make_setplay_slice(slice_index si, slice_traversal *st);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean series_fork_solve_in_n(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean series_fork_has_solution_in_n(slice_index si, stip_length_type n);

/* Determine and write threats
 * @param threats table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void series_fork_solve_threats_in_n(table threats,
                                    slice_index si,
                                    stip_length_type n);

#endif
