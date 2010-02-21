#if !defined(STIPULATION_BATTLE_PLAY_ATTACK_ROOT_H)
#define STIPULATION_BATTLE_PLAY_ATTACK_ROOT_H

#include "boolean.h"
#include "pystip.h"
#include "pyslice.h"
#include "pytable.h"

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

/* Spin off a set play slice
 * @param si slice index
 * @param st state of traversal
 * @return true iff this slice has been sucessfully traversed
 */
boolean attack_root_make_setplay_slice(slice_index si,
                                       struct slice_traversal *st);

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 * @return true iff slice has been successfully traversed
 */
boolean attack_root_reduce_to_postkey_play(slice_index si,
                                           struct slice_traversal *st);

/* Solve a branch slice at root level.
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean attack_root_root_solve(slice_index si);

#endif
