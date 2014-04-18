#if !defined(STIPULATION_BATTLE_PLAY_ATTACK_ADAPTER_H)
#define STIPULATION_BATTLE_PLAY_ATTACK_ADAPTER_H

#include "stipulation/structure_traversal.h"
#include "stipulation/stipulation.h"

/* STAttackAdapter slices switch from generic solving to attack solving.
 */

/* Allocate a STAttackAdapter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_attack_adapter_slice(stip_length_type length,
                                       stip_length_type min_length);

/* Wrap the slices representing the initial moves of the solution with
 * slices of appropriately equipped slice types
 * @param si identifies slice where to start
 * @param st address of structure holding the traversal state
 */
void attack_adapter_make_root(slice_index si, stip_structure_traversal *st);

/* Attempt to add set play to an attack stipulation (battle play, not
 * postkey only)
 * @param si identifies the root from which to apply set play
 * @param st address of structure representing traversal
 */
void attack_adapter_apply_setplay(slice_index si, stip_structure_traversal *st);

#endif
