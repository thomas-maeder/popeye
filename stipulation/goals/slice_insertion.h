#if !defined(STIPULATION_GOALS_SLICE_INSERTION_H)
#define STIPULATION_GOALS_SLICE_INSERTION_H

#include "stipulation/stipulation.h"

/* Insert slices into a goal branch.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by goal_branch_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void goal_branch_insert_slices(slice_index si,
                               slice_index const prototypes[],
                               unsigned int nr_prototypes);

#endif
