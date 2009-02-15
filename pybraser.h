#if !defined(PYBRASER_H)
#define PYBRASER_H

#include "py.h"
#include "pystip.h"
#include "pyslice.h"

/* This module provides functionality dealing with STBranchSeries
 * stipulation slices.
 */

/* Allocate a STBranchSeries slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies next slice
 * @return index of allocated slice
 */
slice_index alloc_branch_ser_slice(stip_length_type length,
                                   stip_length_type min_length,
                                   slice_index next);

/* Spin off a set play slice
 * @param si slice index
 * @return set play slice spun off; no_slice if not applicable
 */
slice_index branch_ser_root_make_setplay_slice(slice_index si);

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

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter branch_ser_detect_starter(slice_index si,
                                                 boolean same_side_as_root);

/* Impose the starting side on a slice.
 * @param si identifies branch
 * @param s starting side of slice
 */
void branch_ser_impose_starter(slice_index si, Side s);

#endif
