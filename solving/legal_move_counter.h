#if !defined(SOLVING_LEGAL_MOVE_COUNTER_H)
#define SOLVING_LEGAL_MOVE_COUNTER_H

#include "stipulation/help_play/play.h"
#include "pyslice.h"

/* This module provides functionality dealing with the attacking side
 * in STLegalMoveCounter stipulation slices.
 */

/* current value of the count */
extern unsigned int legal_move_counter_count[maxply];

/* stop the move iteration once legal_move_counter_count exceeds this number */
extern unsigned int legal_move_counter_interesting[maxply];

/* Allocate a STLegalMoveCounter slice.
 * @return index of allocated slice
 */
slice_index alloc_legal_move_counter_slice(void);

/* Allocate a STAnyMoveCounter slice.
 * @return index of allocated slice
 */
slice_index alloc_any_move_counter_slice(void);

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type legal_move_counter_has_solution(slice_index si);

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type legal_move_counter_can_help(slice_index si, stip_length_type n);

#endif
