#if !defined(PYLEAFD_H)
#define PYLEAFD_H

#include "boolean.h"
#include "pyslice.h"
#include "py.h"
#include "stipulation/battle_play/attack_play.h"

/* This module provides functionality dealing with direct leaf
 * stipulation slices.
 */

/* Determine whether there is a solution in a leaf.
 * @param leaf slice index of leaf slice
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type leaf_d_has_solution(slice_index leaf);

/* Determine and write keys at root level
 * @param leaf leaf's slice index
 * @return true iff >=1 key was found and written
 */
boolean leaf_d_root_solve(slice_index leaf);

/* Solve a slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean leaf_d_solve(slice_index leaf);

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void leaf_d_detect_starter(slice_index si, stip_structure_traversal *st);

#endif
