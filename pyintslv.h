#if !defined(PYINTSLV_H)
#define PYINTSLV_H

#include "pystip.h"

/* Interface intelligent_solvable
 * Implemented by stipulations that can be solved using intelligent mode.
 */

/* Solve a slice in exactly n moves at root level
 * @param si slice index
 * @param n exact number of moves
 */
void intelligent_solvable_root_solve_in_n(slice_index si, stip_length_type n);

#endif
