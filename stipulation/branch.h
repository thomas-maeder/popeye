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
 * stip_move_traversal struct from a branch slice if necessary
 * @param si identifies the branch slice
 * @param st refers to the struct to be initialised
 */
void stip_traverse_moves_branch_init_full_length(slice_index si,
                                                 stip_move_traversal *st);

/* Traversal of the moves of some branch slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_branch_slice(slice_index si, stip_move_traversal *st);

/* Traversal of the moves of a branch
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_branch(slice_index si, stip_move_traversal *st);

#endif
