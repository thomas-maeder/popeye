#if !defined(PYBRAFRK_H)
#define PYBRAFRK_H

/* Branch fork - branch decides that when to continue play in branch
 * and when to change to slice representing subsequent play
 */

#include "stipulation/battle_play/attack_play.h"

/* Allocate a new branch fork slice
 * @param type which slice type
 * @param fork identifies proxy slice that leads towards goal from the branch
 * @return newly allocated slice
 */
slice_index alloc_branch_fork(slice_type type, slice_index fork);

/* Substitute links to proxy slices by the proxy's target
 * @param si slice where to resolve proxies
 * @param st points at the structure holding the state of the traversal
 */
void branch_fork_resolve_proxies(slice_index si, stip_structure_traversal *st);

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void branch_fork_detect_starter(slice_index si, stip_structure_traversal *st);

/* Continue a traversal at the start of a branch; this function is typically
 * invoked by an end of branch slice
 * @param branch_entry entry slice into branch
 * @param st address of data structure holding parameters for the operation
 */
void stip_traverse_structure_next_branch(slice_index branch_entry,
                                         stip_structure_traversal *st);

/* Traverse a subtree
 * @param si root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_children_fork(slice_index si,
                                           stip_structure_traversal *st);

/* Callback to stip_spin_off_testers
 * Spin a tester slice off a fork slice
 * @param si identifies the testing pipe slice
 * @param st address of structure representing traversal
 */
void stip_spin_off_testers_fork(slice_index si, stip_structure_traversal *st);

#endif
