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

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type legal_move_counter_solve(slice_index si);

/* Solve in a number of half-moves
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move leading to the current position has
 *                           turned out to be illegal
 *            n   solution found
 *            n+2 no solution found
 */
stip_length_type legal_move_counter_help(slice_index si, stip_length_type n);

#endif
