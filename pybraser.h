#if !defined(PYBRASER_H)
#define PYBRASER_H

#include "py.h"
#include "pystip.h"

/* This module provides functionality dealing with STBranchSeries
 * stipulation slices.
 */

/* Solve a branch slice at root level.
 * @param si slice index
 */
void branch_ser_root_solve(slice_index si);

/* Solve a branch in exactly n moves at root level
 * @param si slice index
 * @param n exact number of moves
 */
void branch_ser_root_solve_in_n(slice_index si, stip_length_type n);

/* Solve a branch slice at non-root level
 * @param si slice index
 * @return true iff >= 1 solution was found
 */
boolean branch_ser_solve(slice_index si);

/* Impose the starting side on a slice.
 * @param si identifies branch
 * @param s starting side of slice
 */
void branch_ser_impose_starter(slice_index si, Side s);

#endif
