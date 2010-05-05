#if !defined(STIPULATION_SERIES_PLAY_SHORTCUT_H)
#define STIPULATION_SERIES_PLAY_SHORTCUT_H

#include "py.h"
#include "pystip.h"

/* This module provides functionality dealing with STSeriesShortcut
 * stipulation slice type.
 * Slices of this type make sure that attempts fo find short solutions
 * are routed through the correct path.
 */

/* Allocate a STSeriesShortcut slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param short_sols identifies slice to delegate to when looking for
 *                   short solutions
 * @return allocated slice
 */
slice_index alloc_series_shortcut(stip_length_type length,
                                  stip_length_type min_length,
                                  slice_index short_sols);

/* Substitute links to proxy slices by the proxy's target
 * @param si root of sub-tree where to resolve proxies
 * @param st address of structure representing the traversal
 */
void series_shortcut_resolve_proxies(slice_index si,
                                     stip_structure_traversal *st);

/* Determine and write the solution(s) in a series stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type series_shortcut_solve_in_n(slice_index si,
                                            stip_length_type n);

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type series_shortcut_has_solution_in_n(slice_index si,
                                                   stip_length_type n);

#endif
