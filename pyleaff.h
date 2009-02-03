#if !defined(PYLEAFF_H)
#define PYLEAFF_H

#include "pyslice.h"

/* This module provides functionality dealing with leaf slices that
 * find forced half-moves reaching the goal.
 */

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param si slice index
 * @return true iff starter must resign
 */
boolean leaf_forced_must_starter_resign(slice_index leaf);

/* Determine and write the solution of a leaf forced slice at root level
 * @param leaf identifies leaf slice
 * @return true iff >=1 solution was found
 */
void leaf_forced_root_solve(slice_index leaf);

#endif
