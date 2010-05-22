#if !defined(STIPULATION_BATTLE_PLAY_ATTACK_ROOT_H)
#define STIPULATION_BATTLE_PLAY_ATTACK_ROOT_H

#include "boolean.h"
#include "pystip.h"
#include "pyslice.h"

/* This module provides functionality dealing with the attacking side
 * in STAttackMove stipulation slices.
 */

/* Allocate a STAttackRoot slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_attack_root_slice(stip_length_type length,
                                    stip_length_type min_length);

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void attack_root_reduce_to_postkey_play(slice_index si,
                                        stip_structure_traversal *st);

/* Solve a branch slice at root level.
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean attack_root_root_solve(slice_index si);

#endif
