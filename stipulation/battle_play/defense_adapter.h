#if !defined(STIPULATION_BATTLE_PLAY_DEFENSE_ADAPTER_H)
#define STIPULATION_BATTLE_PLAY_DEFENSE_ADAPTER_H

#include "stipulation/structure_traversal.h"

/* STDefenseAdapter slices switch from generic solving to defense solving.
 */

/* Allocate a STDefenseAdapter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_defense_adapter_slice(stip_length_type length,
                                        stip_length_type min_length);

/* Wrap the slices representing the initial moves of the solution with
 * slices of appropriately equipped slice types
 * @param si identifies slice where to start
 * @param st address of structure holding the traversal state
 */
void defense_adapter_make_root(slice_index si, stip_structure_traversal *st);

/* Wrap the slices representing the nested slices
 * @param adapter identifies attack adapter slice
 * @param st address of structure holding the traversal state
 */
void defense_adapter_make_intro(slice_index adapter,
                                stip_structure_traversal *st);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type defense_adapter_solve(slice_index si, stip_length_type n);

#endif
