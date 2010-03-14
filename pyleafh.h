#if !defined(PYLEAFH_H)
#define PYLEAFH_H

#include "boolean.h"
#include "pyslice.h"
#include "py.h"
#include "stipulation/battle_play/defense_play.h"

/* This module provides functionality dealing with leaf stipulation
 * slices.
 */

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
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void leaf_h_detect_starter(slice_index si, slice_traversal *st);

#endif
