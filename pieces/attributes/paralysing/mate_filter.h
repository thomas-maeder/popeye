#if !defined(PIECES_PARALYSING_MATE_FILTER_H)
#define PIECES_PARALYSING_MATE_FILTER_H

#include "pyslice.h"

/* This module provides slice type STPiecesParalysingMateFilter - tests
 * additional conditions on some goals imposed by the presence of paralysing
 * pieces:
 * - the mated side must have >=1 move that only fails because of self-check
 */

/* Allocate a STPiecesParalysingMateFilter slice.
 * @param side side to be mated
 * @return index of allocated slice
 */
slice_index alloc_paralysing_mate_filter_slice(Side side);

/* Determine whether a slice has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type paralysing_mate_filter_has_solution(slice_index si);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type paralysing_mate_filter_solve(slice_index si);

#endif
