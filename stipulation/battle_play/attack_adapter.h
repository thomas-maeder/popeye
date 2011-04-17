#if !defined(STIPULATION_BATTLE_PLAY_ATTACK_ADAPTER_H)
#define STIPULATION_BATTLE_PLAY_ATTACK_ADAPTER_H

#include "stipulation/battle_play/attack_play.h"
#include "stipulation/help_play/play.h"

/* STAttackAdapter slices switch from generic solving to attack solving.
 */

/* Allocate a STAttackAdapter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_attack_adapter_slice(stip_length_type length,
                                       stip_length_type min_length);

/* Traversal of the moves of some adapter slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_battle_adapter_slice(slice_index si,
                                              stip_moves_traversal *st);

/* Determine whether a slice has a solution - adapter for direct slices
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type attack_adapter_has_solution(slice_index si);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type attack_adapter_solve(slice_index si);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type attack_adapter_help(slice_index si, stip_length_type n);

#endif
