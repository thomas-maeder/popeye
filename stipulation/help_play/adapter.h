#if !defined(STIPULATION_HELP_PLAY_ADAPTER_H)
#define STIPULATION_HELP_PLAY_ADAPTER_H

#include "stipulation/battle_play/attack_play.h"

/* This module provides functionality dealing with STHelpAdapter
 * stipulation slices. STHelpAdapter slices switch from general play to help
 * play.
 */

/* Allocate a STHelpAdapter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_help_adapter_slice(stip_length_type length,
                                     stip_length_type min_length);

/* Wrap the slices representing the initial moves of the solution with
 * slices of appropriately equipped slice types
 * @param si identifies slice where to start
 * @param st address of structure holding the traversal state
 */
void help_adapter_make_root(slice_index si, stip_structure_traversal *st);

/* Wrap the slices representing the nested slices
 * @param adapter identifies attack adapter slice
 * @param st address of structure holding the traversal state
 */
void help_adapter_make_intro(slice_index adapter, stip_structure_traversal *st);

/* Traverse a subtree
 * @param si root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_children_help_adpater(slice_index si,
                                                   stip_structure_traversal *st);

/* Traversal of the moves of some adapter slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_help_adapter(slice_index si,
                                      stip_moves_traversal *st);

/* Attempt to add set play to an attack stipulation (battle play, not
 * postkey only)
 * @param si identifies the root from which to apply set play
 * @param st address of structure representing traversal
 */
void help_adapter_apply_setplay(slice_index si, stip_structure_traversal *st);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type help_adapter_attack(slice_index si, stip_length_type n);

#endif
