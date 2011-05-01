#if !defined(STIPULATION_HELP_PLAY_BRANCH_H)
#define STIPULATION_HELP_PLAY_BRANCH_H

#include "pystip.h"
#include "pyslice.h"

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

/* Shorten a help slice by 2 half moves
 * @param si identifies slice to be shortened
 */
void help_branch_shorten_slice(slice_index si);

/* Shorten a help branch by 1 half move
 * @param identifies entry slice of branch to be shortened
 */
void help_branch_shorten(slice_index si);

/* Insert a fork to the next branch
 * @param si identifies the entry slice of a help branch
 * @param next identifies the entry slice of the next branch
 */
void help_branch_set_end(slice_index si, slice_index next);

/* Insert a fork to the goal branch
 * @param si identifies the entry slice of a help branch
 * @param to_goal identifies the entry slice of the branch leading to the goal
 */
void help_branch_set_end_goal(slice_index si, slice_index to_goal);

/* Insert a fork to the next branch
 * @param si identifies the entry slice of a help branch
 * @param next identifies the entry slice of the next branch
 */
void help_branch_set_end_forced(slice_index si, slice_index next);

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

/* Insert slices into a help branch; the elements of
 * prototypes are *not* deallocated by help_branch_insert_slices_nested().
 * The inserted slices are copies of the elements of prototypes).
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void help_branch_insert_slices_nested(slice_index si,
                                      slice_index const prototypes[],
                                      unsigned int nr_prototypes);

/* Wrap the slices representing the initial moves of the solution with
 * slices of appropriately equipped slice types
 * @param adapter identifies the adapter slice at the beginning of the branch
 * @return identifier of initial root slice
 */
slice_index help_branch_make_root(slice_index si);

/* Produce slices representing set play.
 * @param si identifies the successor of the slice representing the
 *           move(s) not played in set play
 * @return entry point of the slices representing set play
 *         no_slice if set play is not applicable
 */
slice_index help_branch_make_setplay(slice_index si);

#endif
