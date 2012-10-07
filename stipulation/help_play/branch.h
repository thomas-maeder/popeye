#if !defined(STIPULATION_HELP_PLAY_BRANCH_H)
#define STIPULATION_HELP_PLAY_BRANCH_H

#include "stipulation/stipulation.h"
#include "solving/solve.h"

/* This module provides functionality dealing with STHelp*
 * stipulation slices.
 */

/* Allocate a help branch.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of entry slice into allocated series branch
 */
slice_index alloc_help_branch(stip_length_type length,
                              stip_length_type min_length);

/* Shorten a help branch by 1 half move
 * @param identifies entry slice of branch to be shortened
 */
void help_branch_shorten(slice_index si);

/* Find a STReadyMove slice with a specific parity
 * @param si identifies the entry slice of a help branch
 * @param parity indicates after which help move of the branch to insert
 * @return identifier of found STReadyMove slice; no_slice if no such slice was found
 */
slice_index help_branch_locate_ready(slice_index si, unsigned int parity);

/* Insert a fork to the next branch
 * @param si identifies the entry slice of a help branch
 * @param next identifies the entry slice of the next branch
 * @param parity indicates after which help move of the branch to insert
 */
void help_branch_set_end(slice_index si,
                         slice_index next,
                         unsigned int parity);

/* Insert a fork to the goal branch
 * @param si identifies the entry slice of a help branch
 * @param to_goal identifies the entry slice of the branch leading to the goal
 * @param parity indicates after which help move of the branch to insert
 */
void help_branch_set_end_goal(slice_index si,
                              slice_index to_goal,
                              unsigned int parity);

/* Instrument a series branch with STEndOfBranchForced slices (typically for a
 * hr stipulation)
 * @param si entry slice of branch to be instrumented
 * @param forced identifies branch forced on the defender
 * @param parity indicates after which help move of the branch to insert
 */
void help_branch_set_end_forced(slice_index si,
                                slice_index forced,
                                unsigned int parity);

/* Instrument a series branch with STConstraint* slices (typically for a hr
 * stipulation)
 * @param si entry slice of branch to be instrumented
 * @param constraint identifies branch that constrains the attacker
 * @param parity indicates after which help move of the branch to insert
 * @return true iff the constraint could be inserted
 * @note deallocates the constraint if couldn't be inserted
 */
boolean help_branch_insert_constraint(slice_index si,
                                      slice_index constraint,
                                      unsigned int parity);

/* Insert slices into a help branch.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by help_branch_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void help_branch_insert_slices(slice_index si,
                               slice_index const prototypes[],
                               unsigned int nr_prototypes);

/* Like help_branch_insert_slices, but starting at a proxy slice
 * @param base used instead of si for determining the current position in the
 *             sequence of defense branches
 */
void help_branch_insert_slices_behind_proxy(slice_index si,
                                            slice_index const prototypes[],
                                            unsigned int nr_prototypes,
                                            slice_index base);

/* Insert slices into a help branch; the elements of
 * prototypes are *not* deallocated by help_branch_insert_slices_nested().
 * The inserted slices are copies of the elements of prototypes).
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param adapter identifies starting point of insertion (of type STHelpAdapter)
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void help_branch_insert_slices_nested(slice_index adapter,
                                      slice_index const prototypes[],
                                      unsigned int nr_prototypes);

/* Create the root slices of a help branch
 * @param adapter identifies the adapter slice at the beginning of the branch
 * @param state address of structure holding state
 */
void help_branch_make_root_slices(slice_index adapter,
                                  spin_off_state_type *state);

/* Wrap the slices representing the initial moves of the solution with
 * slices of appropriately equipped slice types
 * @param adapter identifies the adapter slice at the beginning of the branch
 * @param state address of structure holding state
 */
void help_make_root(slice_index adapter, spin_off_state_type *state);

/* Spin the intro slices off a nested help branch
 * @param adapter identifies adapter slice of the nested help branch
 * @param state address of structure holding state
 */
void help_spin_off_intro(slice_index adapter, spin_off_state_type *state);

/* Produce slices representing set play.
 * @param adapter identifies the adapter slice at the beginning of the branch
 * @param state address of structure holding state
 */
void help_branch_make_setplay(slice_index adapter, spin_off_state_type *state);

/* Allocate a series branch.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of entry slice into allocated series branch
 */
slice_index alloc_series_branch(stip_length_type length,
                                stip_length_type min_length);

/* Produce slices representing set play.
 * @param adapter identifies the adapter slice at the beginning of the branch
 * @param state address of structure holding state
 */
void series_branch_make_setplay(slice_index adapter,
                                spin_off_state_type *state);

/* Instrument a series branch with STConstraint* slices (typically for a ser-r
 * stipulation)
 * @param si entry slice of branch to be instrumented
 * @param constraint identifies branch that constrains the attacker
 */
void series_branch_insert_constraint(slice_index si, slice_index constraint);

#endif
