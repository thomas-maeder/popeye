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
 * @param short_sols identifies slice to delegate to when looking for
 *                   short solutions
 * @return allocated slice
 */
slice_index alloc_series_shortcut(stip_length_type length,
                                  slice_index short_sols);

/* Traverse a subtree
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_series_shortcut(slice_index si,
                                             stip_structure_traversal *st);

/* Traversal of the moves beyond a series shortcut slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_series_shortcut(slice_index si,
                                         stip_moves_traversal *st);

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
