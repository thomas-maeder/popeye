#if !defined(PYLEAFH_H)
#define PYLEAFH_H

#include "boolean.h"
#include "pyslice.h"
#include "pydirect.h"
#include "py.h"

/* This module provides functionality dealing with leaf stipulation
 * slices.
 */

/* Find and write post key play
 * @param leaf slice index
 * @return true iff >=1 solution was found
 */
boolean leaf_h_solve_postkey(slice_index leaf);

/* Try to defend after an attempted key move at non-root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean leaf_h_defend(slice_index leaf);

/* Determine whether there is a solution in a leaf.
 * @param leaf slice index of leaf slice
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type leaf_h_has_solution(slice_index leaf);

/* Determine and write the solution of a leaf slice.
 * @param leaf identifies leaf slice
 * @return true iff >=1 solution was found
 */
boolean leaf_h_solve(slice_index leaf);

/* Determine and write the solution of a leaf slice at root level
 * @param leaf identifies leaf slice
 * @return true iff >=1 solution was found
 */
boolean leaf_h_root_solve(slice_index leaf);

/* Detect starter field with the starting side if possible. 
 * @param leaf identifies leaf
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter leaf_h_detect_starter(slice_index leaf,
                                             boolean same_side_as_root);

#endif
