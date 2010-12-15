#if !defined(STIPULATION_BRANCH_H)
#define STIPULATION_BRANCH_H

/* Functionality related to "branches"; i.e. (chains of) pipe slices
 */

#include "pystip.h"
#include "pyslice.h"

/* Allocate a new branch slice
 * @param type which slice type
 * @param length maximum number of half moves until end of slice
 * @param min_length minimum number of half moves until end of slice
 * @return newly allocated slice
 */
slice_index alloc_branch(SliceType type,
                         stip_length_type length,
                         stip_length_type min_length);

/* Find the next slice with a specific type in a branch
 * @param type type of slice to be found
 * @param si identifies the slice where to start searching (si is not
 *           visited at the start of the search, but if the branch is
 *           recursive, it may be visited as the last slice of the search)
 * @return identifier for slice with type type; no_slice if none is found
 */
slice_index branch_find_slice(SliceType type, slice_index si);

/* Initialise the full_length and remaing fields of a
 * stip_moves_traversal struct from a branch slice if necessary
 * @param si identifies the branch slice
 * @param st refers to the struct to be initialised
 */
void stip_traverse_moves_branch_init_full_length(slice_index si,
                                                 stip_moves_traversal *st);

/* Traversal of the moves of some branch slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_branch_slice(slice_index si, stip_moves_traversal *st);

/* Traversal of the moves of some branch slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_move_slice(slice_index si, stip_moves_traversal *st);

/* Traversal of the moves of a branch
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_branch(slice_index si, stip_moves_traversal *st);

/* Link a pipe slice to the entry slice of a branch
 * @param pipe identifies the pipe slice
 * @param entry identifies the entry slice of the branch
 */
void link_to_branch(slice_index pipe, slice_index entry);

/* Insert slices into a root branch.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by root_branch_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void root_branch_insert_slices(slice_index si,
                               slice_index const prototypes[],
                               unsigned int nr_prototypes);

/* Insert slices into a leaf branch.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by leaf_branch_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void leaf_branch_insert_slices(slice_index si,
                               slice_index const prototypes[],
                               unsigned int nr_prototypes);

/* Insert slices into a leaf branch; the elements of
 * prototypes are *not* deallocated by series_branch_insert_slices().
 * The inserted slices are copies of the elements of prototypes).
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void leaf_branch_insert_slices_nested(slice_index si,
                                         slice_index const prototypes[],
                                         unsigned int nr_prototypes);

#endif
