#if !defined(STIPULATION_LEAF_H)
#define STIPULATION_LEAF_H

#include "pyslice.h"

/* This module provides functionality dealing with true slices
 */

/* Allocate a STTrue slice.
 * @return index of allocated slice
 */
slice_index alloc_true_slice(void);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type true_solve(slice_index si);

/* Spin a copy off a pipe to add it to the root or set play branch
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void leaf_spin_off_copy(slice_index si, stip_structure_traversal *st);

/* Callback to stip_spin_off_testers
 * Spin a tester slice off a leaf slice
 * @param si identifies the testing pipe slice
 * @param st address of structure representing traversal
 */
void stip_spin_off_testers_leaf(slice_index si, stip_structure_traversal *st);

#endif
