#if !defined(STIPULATION_HELP_PLAY_FORK_H)
#define STIPULATION_HELP_PLAY_FORK_H

/* Branch fork - branch decides that when to continue play in branch
 * and when to change to slice representing subsequent play
 */

#include "pyslice.h"

/* Allocate a STHelpFork slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param to_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_help_fork_slice(stip_length_type length,
                                  stip_length_type min_length,
                                  slice_index to_goal);

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void help_fork_insert_root(slice_index si, stip_structure_traversal *st);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type help_fork_solve_in_n(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type help_fork_has_solution_in_n(slice_index si,
                                             stip_length_type n);

#endif
