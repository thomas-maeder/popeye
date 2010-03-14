#if !defined(STIPULATION_SERIES_PLAY_PARRY_FORK_H)
#define STIPULATION_SERIES_PLAY_PARRY_FORK_H

#include "py.h"
#include "pystip.h"
#include "pyslice.h"

/* This module provides functionality dealing with STParryFork
 * stipulation slice
 */

/* Convert a series branch to a parry series branch
 * @param si identifies first slice of the series branch
 * @param parrying identifies slice responsible for parrying
 * @return identifier of slice representing the play after the
 *         parrying logic
 */
slice_index convert_to_parry_series_branch(slice_index si, slice_index parrying);

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean parry_fork_solve_in_n(slice_index si, stip_length_type n);

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean parry_fork_has_solution_in_n(slice_index si, stip_length_type n);

/* Substitute links to proxy slices by the proxy's target
 * @param si root of sub-tree where to resolve proxies
 * @param st address of structure representing the traversal
 */
void parry_fork_resolve_proxies(slice_index si, slice_traversal *st);

#endif
