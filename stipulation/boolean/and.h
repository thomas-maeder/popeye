#if !defined(PYRECIPR_H)
#define PYRECIPR_H

#include "boolean.h"
#include "py.h"
#include "pyslice.h"

/* This module provides functionality dealing with reciprocal
 * (i.e. logical AND) stipulation slices.
 */

/* Allocate a reciprocal slice.
 * @param proxy1 proxy to 1st operand
 * @param proxy2 proxy to 2nd operand
 * @return index of allocated slice
 */
slice_index alloc_reciprocal_slice(slice_index proxy1, slice_index proxy2);

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void reci_insert_root(slice_index si, stip_structure_traversal *st);

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param si slice index
 * @param n number of moves until goal
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean reci_are_threats_refuted(table threats, slice_index si);

/* Determine whether there is a solution at the end of a reciprocal
 * slice. 
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type reci_has_solution(slice_index si);

/* Determine and write threats of a slice
 * @param threats table where to store threats
 * @param si index of branch slice
 */
void reci_solve_threats(table threats, slice_index si);

/* Solve at root level at the end of a reciprocal slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean reci_root_solve(slice_index si);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type reci_solve(slice_index si);

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void reci_detect_starter(slice_index si, stip_structure_traversal *st);

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 */
void reci_impose_starter(slice_index si, stip_structure_traversal *st);

#endif
