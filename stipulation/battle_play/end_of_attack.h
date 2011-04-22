#if !defined(STIPULATION_BATTLE_PLAY_END_OF_ATTACK_H)
#define STIPULATION_BATTLE_PLAY_END_OF_ATTACK_H

#include "boolean.h"
#include "pystip.h"
#include "pyslice.h"

/* This module provides functionality dealing with the attacking side
 * in STEndOfAttack stipulation slices.
 */

/* Allocate a STEndOfAttack slice.
 * @return index of allocated slice
 */
slice_index alloc_end_of_attack_slice(void);

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void end_of_attack_apply_postkeyplay(slice_index si,
                                     stip_structure_traversal *st);

#endif
