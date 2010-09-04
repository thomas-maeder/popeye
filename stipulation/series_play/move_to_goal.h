#if !defined(STIPULATION_SERIES_PLAY_MOVE_TO_GOAL_H)
#define STIPULATION_SERIES_PLAY_MOVE_TO_GOAL_H

#include "py.h"
#include "pystip.h"

/* This module provides functionality dealing with STSeriesMoveToGoal
 * stipulation slices.
 */

/* Allocate a STSeriesMoveToGoal slice.
 * @param goal goal to be reached
 * @return index of allocated slice
 */
slice_index alloc_series_move_to_goal_slice(Goal goal);

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void series_move_to_goal_detect_starter(slice_index si,
                                        stip_structure_traversal *st);

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type series_move_to_goal_solve_in_n(slice_index si,
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
stip_length_type series_move_to_goal_has_solution_in_n(slice_index si,
                                                       stip_length_type n);

#endif
