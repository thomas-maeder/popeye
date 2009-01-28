#if !defined(PYBRAH_H)
#define PYBRAH_H

#include "py.h"
#include "pystip.h"

/* This module provides functionality dealing with STBranchHelp
 * stipulation slices.
 */

/* Determine and write set play
 * @param si slice index
 * @return true iff >= 1 set play was found
 */
boolean branch_h_root_solve_setplay(slice_index si);

/* Solve a branch slice at root level.
 * @param si slice index
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

#endif
