#if !defined(PYQUODLI_H)
#define PYQUODLI_H

#include "py.h"
#include "pyslice.h"
#include "boolean.h"

/* This module provides functionality dealing with quodlibet
 * (i.e. logical OR) stipulation slices.
 */

/* Allocate a quodlibet slice.
 * @param proxy1 1st operand
 * @param proxy2 2nd operand
 * @return index of allocated slice
 */
slice_index alloc_quodlibet_slice(slice_index proxy1, slice_index proxy2);

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void quodlibet_insert_root(slice_index si, stip_structure_traversal *st);

/* Determine whether a quodlibet slice jas a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type quodlibet_has_solution(slice_index si);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type quodlibet_solve(slice_index si);

/* Solve a quodlibet slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean quodlibet_root_solve(slice_index si);

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void quodlibet_detect_starter(slice_index si, stip_structure_traversal *st);

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 */
void quodlibet_impose_starter(slice_index si, stip_structure_traversal *st);

#endif
