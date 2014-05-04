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

/* Find the next slice with a specific type in a branch
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

/* Instrument a traversal for traversing the "normal path" through a branch.
 * In particular, the traversal won't enter nested branches.
 * @param st traversal to be instrumented
 * @note The caller must already have invoked a stip_structure_traversal_init*
 *       function on st
 */
void branch_instrument_traversal_for_normal_path(stip_structure_traversal *st);

#endif
