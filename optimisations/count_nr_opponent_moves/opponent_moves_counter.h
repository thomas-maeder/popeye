#if !defined(OPTIMISATIONS_COUNTNROPPONENTMOVES_COUNTER_H)
#define OPTIMISATIONS_COUNTNROPPONENTMOVES_COUNTER_H

#include "solving/solve.h"

/* This module provides functionality dealing with the attacking side
 * in STOpponentMovesCounter stipulation slices.
 */

/* Allocate a STOpponentMovesCounter slice.
 * @return index of allocated slice
 */
slice_index alloc_opponent_moves_counter_slice(void);

/* Initialise counting the opponent's moves after the move just generated */
void init_opponent_moves_counter();

/* Retrieve the number opponent's moves after the move just generated
 * @return number of opponent's moves
 */
int fini_opponent_moves_counter();

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type opponent_moves_counter_solve(slice_index si,
                                              stip_length_type n);

#endif
