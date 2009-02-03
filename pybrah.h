#if !defined(PYBRAH_H)
#define PYBRAH_H

#include "py.h"
#include "pystip.h"

/* This module provides functionality dealing with STBranchHelp
 * stipulation slices.
 */

/* Determine whether a slice has a solution
 * @param si slice index
 * @return true iff slice si has a solution
 */
boolean branch_h_has_solution(slice_index si);

/* Prepare a slice for spinning of a set play slice
 * @param si slice index
 */
slice_index branch_h_root_prepare_for_setplay(slice_index si);

/* Spin of a set play slice
 * Assumes that slice_root_prepare_for_setplay(si) was invoked and
 * did not return no_slice
 * @param si slice index
 * @return set play slice spun off
 */
slice_index branch_h_root_make_setplay_slice(slice_index si);

/* Determine and write set play
 * @param si slice index
 * @return true iff >= 1 set play was found
 */
boolean branch_h_root_solve_setplay(slice_index si);

/* Solve a branch slice at root level.
 * @param si slice index
 * @return no_slice if set play not applicable
 *         new root slice index (may be equal to old one) otherwise
 */
void branch_h_root_solve(slice_index si);

/* Solve a branch slice at non-root level.
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean branch_h_solve(slice_index si);

/* Solve a branch in exactly n moves at root level
 * @param si slice index
 * @param n exact number of moves
 */
void branch_h_root_solve_in_n(slice_index si, stip_length_type n);

/* Determine the starting side in a help branch in n
 * @param si slice index
 * @param n number of half-moves
 */
Side branch_h_starter_in_n(slice_index si, stip_length_type n);

/* Impose the starting side on a slice.
 * @param si identifies branch
 * @param s starting side of slice
 */
void branch_h_impose_starter(slice_index si, Side s);

/* Determine and write continuations of a slice
 * @param table table where to store continuing moves (i.e. threats)
 * @param si index of branch slice
 */
void branch_h_solve_continuations(int table, slice_index si);

#endif
