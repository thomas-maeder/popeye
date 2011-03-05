#if !defined(STIPULATION_BATTLE_PLAY_FORK_H)
#define STIPULATION_BATTLE_PLAY_FORK_H

#include "pystip.h"

/* This module provides functionality dealing with battle play
 * forks
 */


/* Traverse a subtree
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_battle_fork(slice_index branch,
                                         stip_structure_traversal *st);

/* Traversal of the moves beyond a battle fork slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_battle_fork(slice_index si, stip_moves_traversal *st);

#endif
