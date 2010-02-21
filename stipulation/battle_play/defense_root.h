#if !defined(STIPULATION_BATTLE_PLAY_DEFENSE_ROOT_H)
#define STIPULATION_BATTLE_PLAY_DEFENSE_ROOT_H

#include "pydirect.h"

/* This module provides functionality dealing with the defending side
 * in STAttackMove stipulation slices.
 */

/* Allocate a STDefenseRoot defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_defense_root_slice(stip_length_type length,
                                     stip_length_type min_length);

/* Solve at root level.
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean defense_root_solve(slice_index si);

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean defense_root_defend(slice_index si);

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 * @return true iff slice has been successfully traversed
 */
boolean defense_root_detect_starter(slice_index si, slice_traversal *st);

/* Spin off a set play slice
 * @param si slice index
 * @param st state of traversal
 * @return true iff this slice has been sucessfully traversed
 */
boolean defense_root_make_setplay_slice(slice_index si,
                                        struct slice_traversal *st);

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 * @return true iff slice has been successfully traversed
 */
boolean defense_root_reduce_to_postkey_play(slice_index si,
                                            struct slice_traversal *st);

#endif
