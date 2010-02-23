#if !defined(PYLEAFD_H)
#define PYLEAFD_H

#include "boolean.h"
#include "pyslice.h"
#include "pytable.h"
#include "py.h"
#include "stipulation/battle_play/attack_play.h"

/* This module provides functionality dealing with direct leaf
 * stipulation slices.
 */

/* Determine and write threats of a slice
 * @param threats table where to store threats
 * @param leaf index of branch slice
 */
void leaf_d_solve_threats(table threats, slice_index leaf);

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param leaf slice index
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean leaf_d_are_threats_refuted(table threats, slice_index leaf);

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
 * @return true iff slice has been successfully traversed
 */
boolean leaf_d_detect_starter(slice_index si, slice_traversal *st);

#endif
