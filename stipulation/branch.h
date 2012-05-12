#if !defined(STIPULATION_BRANCH_H)
#define STIPULATION_BRANCH_H

/* Functionality related to "branches"; i.e. (chains of) pipe slices
 */

#include "stipulation/structure_traversal.h"

/* Allocate a new branch slice
 * @param type which slice type
 * @param length maximum number of half moves until end of slice
 * @param min_length minimum number of half moves until end of slice
 * @return newly allocated slice
 */
slice_index alloc_branch(slice_type type,
                         stip_length_type length,
                         stip_length_type min_length);

/* Find the next1 slice with a specific type in a branch
 * @param type type of slice to be found
 * @param si identifies the slice where to start searching
 * @param context context at start of traversal
 * @return identifier for slice with type type; no_slice if none is found
 */
slice_index branch_find_slice(slice_type type,
                              slice_index si,
                              stip_traversal_context_type context);

/* Link a pipe slice to the entry slice of a branch
 * @param pipe identifies the pipe slice
 * @param entry identifies the entry slice of the branch
 */
void link_to_branch(slice_index pipe, slice_index entry);

/* Shorten slices of a branch by 2 half moves
 * @param start identfies start of sequence of slices to be shortened
 * @param end_type identifies type of slice where to stop shortening
 * @param context traversal context at start
 */
void branch_shorten_slices(slice_index start,
                           slice_type end_type,
                           stip_traversal_context_type context);

enum
{
  no_slice_rank = INT_MAX
};

typedef struct
{
    slice_index const *prototypes;
    unsigned int nr_prototypes;
    slice_index const *slice_rank_order;
    unsigned int nr_slice_rank_order_elmts;
    unsigned int base_rank;
    slice_index prev;
} branch_slice_insertion_state_type;

/* Determine the rank of a slice type
 * @param type slice type
 * @return rank of type; no_slice_rank if the rank can't be determined
 */
unsigned int get_slice_rank(slice_type type,
                            branch_slice_insertion_state_type const *state);

/* Initialise a structure traversal for inserting slices into a branch
 * @param st address of structure representing the traversal
 * @param state address of structure representing the insertion
 * @param context initial context of traversal
 */
void init_slice_insertion_traversal(stip_structure_traversal *st,
                                    branch_slice_insertion_state_type *state,
                                    stip_traversal_context_type context);

/* Deallocate an array of slice insertion prototypes
 * @param prototypes array of prototypes
 * @param nr_prototypes size of prototypes
 */
void deallocate_slice_insertion_prototypes(slice_index const prototypes[],
                                           unsigned int nr_prototypes);

/* Insert slices into a generic branch; the elements of
 * prototypes are *not* deallocated by leaf_branch_insert_slices_nested().
 * The inserted slices are copies of the elements of prototypes).
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void branch_insert_slices_nested(slice_index si,
                                 slice_index const prototypes[],
                                 unsigned int nr_prototypes);

/* Insert slices into a branch.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by leaf_branch_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void branch_insert_slices(slice_index si,
                          slice_index const prototypes[],
                          unsigned int nr_prototypes);

/* Instrument a traversal for traversing the "normal path" through a branch.
 * In particular, the traversal won't enter nested branches.
 * @param st traversal to be instrumented
 * @note The caller must already have invoked a stip_structure_traversal_init*
 *       function on st
 */
void branch_instrument_traversal_for_normal_path(stip_structure_traversal *st);

#endif
