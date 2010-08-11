#if !defined(STIPULATION_BATTLE_PLAY_DEFENSE_DEALT_WITH_H)
#define STIPULATION_BATTLE_PLAY_DEFENSE_DEALT_WITH_H

#include "boolean.h"
#include "pystip.h"
#include "pyslice.h"

/* This module provides functionality dealing with the attacking side
 * in STDefenseDealtWith stipulation slices.
 */

/* Allocate a STDefenseDealtWith slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_defense_dealt_with_slice(stip_length_type length,
                                           stip_length_type min_length);

/* Traversal of the moves beyond a attack end slice 
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_defense_dealt_with(slice_index si,
                                            stip_moves_traversal *st);

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void defense_dealt_with_reduce_to_postkey_play(slice_index si,
                                               stip_structure_traversal *st);

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
defense_dealt_with_has_solution_in_n(slice_index si,
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
defense_dealt_with_solve_in_n(slice_index si,
                              stip_length_type n,
                              stip_length_type n_max_unsolvable);

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void defense_dealt_with_detect_starter(slice_index si,
                                       stip_structure_traversal *st);

/* Spin off set play
 * @param si slice index
 * @param st state of traversal
 */
void defense_dealt_with_apply_setplay(slice_index si,
                                      stip_structure_traversal *st);

#endif
