#if !defined(STIPULATION_GOALS_DOUBLEMATE_FILTER_H)
#define STIPULATION_GOALS_DOUBLEMATE_FILTER_H

#include "solving/solve.h"

/* This module provides functionality dealing with the attacking side
 * in STDoubleMateFilter stipulation slices.
 * Slices of this type make sure that the prerequisites for reaching a
 * doublemate are fulfilled (i.e. that the side delivering the doublemate
 * isn't immobile before the move delivering it).
 */

/* Allocate a STDoubleMateFilter slice.
 * @return index of allocated slice
 */
slice_index alloc_doublemate_filter_slice(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type doublemate_filter_solve(slice_index si, stip_length_type n);

#endif
