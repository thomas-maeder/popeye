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

#endif
