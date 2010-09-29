#if !defined(PIECES_PARALYSING_STALEMATE_FILTER_H)
#define PIECES_PARALYSING_STALEMATE_FILTER_H

#include "pyslice.h"

/* This module provides slice type STPiecesParalysingStalemateSpecial - as a
 * consequence of the special mating rule, a side is stalemate if it is in check
 * but totally paralysed
 */

/* Allocate a STPiecesParalysingStalemateSpecial slice.
 * @return index of allocated slice
 */
slice_index alloc_paralysing_stalemate_special_slice(Side side);

/* Determine whether a slice has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type paralysing_stalemate_special_has_solution(slice_index si);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type paralysing_stalemate_special_solve(slice_index si);

#endif
