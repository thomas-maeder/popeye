#if !defined(STIPULATION_BATTLE_PLAY_BRANCH_H)
#define STIPULATION_BATTLE_PLAY_BRANCH_H

#include "stipulation/stipulation.h"
#include "stipulation/slice.h"
#include "stipulation/structure_traversal.h"

/* This module provides functionality dealing with battle play
 * branches
 */

/* Allocate a branch consisting mainly of an defense move
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of entry slice to allocated branch
 */
slice_index alloc_defense_branch(stip_length_type length,
                                 stip_length_type min_length);

/* Allocate a branch that represents battle play
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of entry slice of allocated branch
 */
slice_index alloc_battle_branch(stip_length_type length,
                                stip_length_type min_length);

/* Insert slices into a battle branch, starting between defense and attack move
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by battle_branch_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void attack_branch_insert_slices(slice_index si,
                                 slice_index const prototypes[],
                                 unsigned int nr_prototypes);

/* Like attack_branch_insert_slices, but starting at a proxy slice
 * @param base used instead of proxy for determining the current position in the
 *             sequence of slices
 */
void attack_branch_insert_slices_behind_proxy(slice_index si,
                                              slice_index const prototypes[],
                                              unsigned int nr_prototypes,
                                              slice_index base);

/* Insert slices into a battle branch, starting between attack and defense move
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by battle_branch_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void defense_branch_insert_slices(slice_index si,
                                  slice_index const prototypes[],
                                  unsigned int nr_prototypes);

/* Like defense_branch_insert_slices, but starting at a proxy slice
 * @param base used instead of proxy for determining the current position in the
 *             sequence of slices
 */
void defense_branch_insert_slices_behind_proxy(slice_index proxy,
                                               slice_index const prototypes[],
                                               unsigned int nr_prototypes,
                                               slice_index base);

/* Insert slices into a battle branch; the elements of
 * prototypes are *not* deallocated by battle_branch_insert_slices_nested().
 * The inserted slices are copies of the elements of prototypes).
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param adapter identifies starting point of insertion (of type STAttackAdapter
 *                or STDefenseAdapter)
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void battle_branch_insert_slices_nested(slice_index adapter,
                                        slice_index const prototypes[],
                                        unsigned int nr_prototypes);

/* Produce slices representing set play.
 * @param si identifies the successor of the slice representing the
 *           move(s) not played in set play
 * @param state address of structure holding state
 */
void battle_branch_make_setplay(slice_index si, spin_off_state_type *state);

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
void post_key_play_stipulation_modifier_solve(slice_index si);

/* Create the root slices of a battle branch
 * @param adapter identifies the adapter slice at the beginning of the branch
 * @param state address of structure holding state
 */
void battle_branch_make_root_slices(slice_index adapter,
                                    spin_off_state_type *state);

/* Wrap the slices representing the initial moves of the solution with
 * slices of appropriately equipped slice types
 * @param si identifies slice where to start
 * @param state address of structure holding state
 */
void battle_make_root(slice_index si, spin_off_state_type *state);

/* Instrument a branch with STEndOfBranchForced slices (typically for a
 * (semi-)reflex stipulation)
 * @param si root of branch to be instrumented
 * @param forced identifies what branch needs to be guarded from
 */
void battle_branch_insert_end_of_branch_forced(slice_index si,
                                               slice_index forced);

/* Instrument a branch with slices dealing with direct play
 * @param si root of branch to be instrumented
 * @param next identifies slice leading towards goal
 */
void battle_branch_insert_direct_end_of_branch(slice_index si,
                                               slice_index next);

/* Instrument a branch with slices dealing with direct play
 * @param si root of branch to be instrumented
 * @param goal identifies slice leading towards goal
 */
void battle_branch_insert_direct_end_of_branch_goal(slice_index si,
                                                    slice_index goal);

/* Instrument a branch with slices dealing with self play
 * @param si root of branch to be instrumented
 * @param next identifies slice leading towards goal
 */
void battle_branch_insert_self_end_of_branch(slice_index si, slice_index goal);

/* Instrument a branch for detecting whether the defense was forced to reach a
 * goal
 * @param si root of branch to be instrumented
 * @param goal identifies slice leading towards goal
 */
void battle_branch_insert_self_end_of_branch_goal(slice_index si,
                                                  slice_index goal);

/* Instrument a battle branch with STConstraint* slices (typically for a reflex
 * stipulation)
 * @param si entry slice of branch to be instrumented
 * @param constraint identifies branch that constrains the attacker
 */
void battle_branch_insert_attack_constraint(slice_index si,
                                            slice_index constraint);

/* Instrument a battle branch with STConstraint* slices (typically for a reflex
 * stipulation)
 * @param si entry slice of branch to be instrumented
 * @param constraint identifies branch that constrains the attacker
 */
void battle_branch_insert_attack_goal_constraint(slice_index si,
                                                 slice_index constraint);

/* Instrument a battle branch with STConstraint* slices (typically for a reflex
 * stipulation)
 * @param si entry slice of branch to be instrumented
 * @param constraint identifies branch that constrains the attacker
 */
void battle_branch_insert_defense_constraint(slice_index si,
                                             slice_index constraint);

/* Instrument a battle branch with STConstraint* slices (typically for a reflex
 * stipulation)
 * @param si entry slice of branch to be instrumented
 * @param constraint identifies branch that constrains the attacker
 */
void battle_branch_insert_defense_goal_constraint(slice_index si,
                                                  slice_index constraint);

/* Apply the option postkeyplay to a newly created stipulation
 * @param si identifies the postkeyplay applier slice
 */
void post_key_play_stipulation_modifier_solve(slice_index si);

#endif
