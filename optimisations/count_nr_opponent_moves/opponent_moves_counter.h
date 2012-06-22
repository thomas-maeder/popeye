#if !defined(OPTIMISATIONS_COUNTNROPPONENTMOVES_COUNTER_H)
#define OPTIMISATIONS_COUNTNROPPONENTMOVES_COUNTER_H

#include "solving/battle_play/attack_play.h"

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

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type opponent_moves_counter_attack(slice_index si,
                                               stip_length_type n);

#endif
