#if !defined(STIPULATION_BATTLE_PLAY_READY_FOR_ATTACK_H)
#define STIPULATION_BATTLE_PLAY_READY_FOR_ATTACK_H

#include "boolean.h"
#include "pystip.h"
#include "pyslice.h"

/* This module provides functionality dealing with the attacking side
 * in STReadyForAttack stipulation slices.
 */

/* Allocate a STReadyForAttack slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_ready_for_attack_slice(stip_length_type length,
                                         stip_length_type min_length);

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void ready_for_attack_reduce_to_postkey_play(slice_index si,
                                             stip_structure_traversal *st);

#endif
