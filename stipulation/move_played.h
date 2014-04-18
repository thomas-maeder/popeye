#if !defined(STIPULATION_MOVE_PLAYED_H)
#define STIPULATION_MOVE_PLAYED_H

#include "stipulation/structure_traversal.h"

/* This module provides functionality that concludes the execution of a move.
 */

/* Allocate a STAttackPlayed slice.
 * @return index of allocated slice
 */
slice_index alloc_attack_played_slice(void);

/* Allocate a STDefensePlayed slice.
 * @return index of allocated slice
 */
slice_index alloc_defense_played_slice(void);

/* Allocate a STHelpMovePlayed slice.
 * @return index of allocated slice
 */
slice_index alloc_help_move_played_slice(void);

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void move_played_detect_starter(slice_index si, stip_structure_traversal *st);

#endif
