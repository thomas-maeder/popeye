#if !defined(STIPULATION_BATTLE_PLAY_BRANCH_H)
#define STIPULATION_BATTLE_PLAY_BRANCH_H

#include "pystip.h"

/* This module provides functionality dealing with battle play
 * branches
 */

/* Allocate a branch consisting mainly of an defense move
 * @param next identifies the slice that the defense branch lead sto
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of entry slice to allocated branch
 */
slice_index alloc_defense_branch(slice_index next,
                                 stip_length_type length,
                                 stip_length_type min_length);

/* Allocate a branch that represents battle play
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of entry slice of allocated branch
 */
slice_index alloc_battle_branch(stip_length_type length,
                                stip_length_type min_length);

/* Insert slices into a battle branch.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by battle_branch_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void battle_branch_insert_slices(slice_index si,
                                 slice_index const prototypes[],
                                 unsigned int nr_prototypes);

/* Remove slices from a battle branch
 * @param si identifies starting point of deletion
 * @param types contains the types of slices to be removed in the order that
 *              they occur in types
 * @param nr_types number of elements of array types
 */
void battle_branch_remove_slices(slice_index si,
                                 SliceType const types[],
                                 unsigned int nr_types);

/* Insert slices into a battle branch; the elements of
 * prototypes are *not* deallocated by battle_branch_insert_slices_nested().
 * The inserted slices are copies of the elements of prototypes).
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void battle_branch_insert_slices_nested(slice_index si,
                                        slice_index const prototypes[],
                                        unsigned int nr_prototypes);

/* Instrument a branch leading to a goal to be an attack branch
 * @param si identifies entry slice of branch
 */
void stip_make_goal_attack_branch(slice_index si);

/* Produce slices representing set play.
 * @param si identifies the successor of the slice representing the
 *           move(s) not played in set play
 * @return entry point of the slices representing set play
 *         no_slice if set play is not applicable
 */
slice_index battle_branch_make_setplay(slice_index si);

/* Make the postkey play representation of a non-postkey play representation
 * @param root_proxy identifies root proxy slice
 * @return identifier to adapter slice into postkey representation
 */
slice_index battle_branch_make_postkeyplay(slice_index si);

/* Remove a slice while converting the stipulation to postkey only play
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void trash_for_postkey_play(slice_index si, stip_structure_traversal *st);

/* Attempt to apply the postkey play option to the current stipulation
 * @param root_proxy identifies root proxy slice
 * @return true iff postkey play option is applicable (and has been
 *              applied)
 */
boolean battle_branch_apply_postkeyplay(slice_index root_proxy);

/* Wrap the slices representing the initial moves of the solution with
 * slices of appropriately equipped slice types
 * @param si identifies slice where to start
 * @return identifier of root slice
 */
slice_index battle_branch_make_root(slice_index si);

/* Instrument a branch with STEndOfBranchForced slices (typically for a
 * (semi-)reflex stipulation)
 * @param si root of branch to be instrumented
 * @param forced identifies what branch needs to be guarded from
 */
void battle_branch_insert_end_of_branch_forced(slice_index si,
                                               slice_index forced);

/* Instrument a branch with slices dealing with direct play
 * @param si root of branch to be instrumented
 * @param goal identifies slice leading towards goal
 */
void battle_branch_insert_direct_end_of_branch_goal(slice_index si,
                                                    slice_index goal);

/* Instrument a branch for detecting whether the defense was forced to reach a
 * goal
 * @param si root of branch to be instrumented
 * @param goal identifies slice leading towards goal
 */
void battle_branch_insert_self_end_of_branch_goal(slice_index si,
                                                  slice_index goal);

/* Instrument a battle branch with STConstraint slices (typically for a reflex
 * stipulation)
 * @param si entry slice of branch to be instrumented
 * @param constraint identifies branch that constrains the attacker
 */
void battle_branch_insert_attack_constraint(slice_index si,
                                            slice_index constraint);

/* Instrument a battle branch with STConstraint slices (typically for a reflex
 * stipulation)
 * @param si entry slice of branch to be instrumented
 * @param constraint identifies branch that constrains the attacker
 */
void battle_branch_insert_defense_constraint(slice_index si,
                                             slice_index constraint);

#endif
