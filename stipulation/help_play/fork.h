#if !defined(STIPULATION_HELP_PLAY_FORK_H)
#define STIPULATION_HELP_PLAY_FORK_H

/* Branch fork - branch decides that when to continue play in branch
 * and when to change to slice representing subsequent play
 */

#include "stipulation/help_play/play.h"

/* Allocate a STHelpFork slice.
 * @param to_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_help_fork_slice(slice_index to_goal);

/* Traversal of the moves beyond a help fork slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_help_fork(slice_index si, stip_moves_traversal *st);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type help_fork_help(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type help_fork_can_help(slice_index si, stip_length_type n);

#endif
