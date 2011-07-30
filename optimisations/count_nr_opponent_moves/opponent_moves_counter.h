#if !defined(OPTIMISATIONS_COUNTNROPPONENTMOVES_COUNTER_H)
#define OPTIMISATIONS_COUNTNROPPONENTMOVES_COUNTER_H

#include "stipulation/help_play/play.h"

/* This module provides functionality dealing with the attacking side
 * in STOpponentMovesCounter stipulation slices.
 */

/* current value of the count */
extern int opponent_moves_counter_count;

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

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type opponent_moves_counter_can_help(slice_index si,
                                                 stip_length_type n);

#endif
