#if !defined(STIPULATION_BATTLE_PLAY_FORK_H)
#define STIPULATION_BATTLE_PLAY_FORK_H

#include "pystip.h"

/* This module provides functionality dealing with battle play
 * forks
 */


/* Traversal of the moves beyond a series fork slice 
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_battle_fork(slice_index si, stip_move_traversal *st);

#endif
