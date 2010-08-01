#if !defined(STIPULATION_BATTLE_PLAY_COUNTERMATE_ATTACKER_FILTER_H)
#define STIPULATION_BATTLE_PLAY_COUNTERMATE_ATTACKER_FILTER_H

#include "boolean.h"
#include "pystip.h"
#include "pyslice.h"

/* This module provides functionality dealing with the attacking side
 * in STCounterMateAttackerFilter stipulation slices.
 * Slices of this type make sure that the prerequisites for reaching a
 * countermate are fulfilled (i.e. that the side delivering the countermate
 * is mated before the move delivering it).
 */

/* Allocate a STCounterMateAttackerFilter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index
alloc_countermate_attacker_filter_slice(stip_length_type length,
                                        stip_length_type min_length);

/* Determine whether there is a solution in n half moves, by trying
 * n_min, n_min+2 ... n half-moves.
 * @param si slice index
 * @param n maximal number of moves
 * @param n_min minimal number of half moves to try
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
countermate_attacker_filter_has_solution_in_n(slice_index si,
                                              stip_length_type n,
                                              stip_length_type n_min,
                                              stip_length_type n_max_unsolvable);

/* Solve a slice, by trying n_min, n_min+2 ... n half-moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
countermate_attacker_filter_solve_in_n(slice_index si,
                                       stip_length_type n,
                                       stip_length_type n_max_unsolvable);

#endif
