#if !defined(OPTIMISATIONS_ORTHODOX_MATING_MOVE_FORK_H)
#define OPTIMISATIONS_ORTHODOX_MATING_MOVE_FORK_H

#include "boolean.h"
#include "pystip.h"
#include "pyslice.h"

/* This module provides functionality dealing with the attacking side
 * in STOrthodoxMatingMoveFork stipulation slices.
 */

/* Allocate a STOrthodoxMatingMoveFork slice.
 * @param proxy_to_next identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_orthodox_mating_move_fork_slice(slice_index proxy_to_next);

/* Traversal of the moves beyond an attack fork slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_orthodox_mating_move_fork(slice_index si,
                                                   stip_moves_traversal *st);

/* Determine whether there is a solution in n half moves.
 * @param si slice index
 * @param n maximal number of moves
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
orthodox_mating_move_fork_has_solution_in_n(slice_index si,
                                            stip_length_type n,
                                            stip_length_type n_max_unsolvable);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @note n==n_max_unsolvable means that we are solving refutations
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
orthodox_mating_move_fork_solve_in_n(slice_index si,
                                     stip_length_type n,
                                     stip_length_type n_max_unsolvable);

#endif
