#if !defined(PYLEAFF_H)
#define PYLEAFF_H

#include "stipulation/battle_play/defense_play.h"

/* This module provides functionality dealing with leaf slices that
 * detect whether the defending side has just the goal that it defends
 * against.
 */

/* Determine whether a leaf slice.has just been solved with the move
 * by the non-starter 
 * @param leaf slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type leaf_forced_has_solution(slice_index leaf);

/* Solve at non-root level
 * @param leaf slice index
 * @return true iff >=1 solution was found
 */
boolean leaf_forced_solve(slice_index leaf);

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void leaf_forced_detect_starter(slice_index si, stip_structure_traversal *st);

#endif
