#if !defined(CONDITIONS_ANTICIRCE_SUPER_H)
#define CONDITIONS_ANTICIRCE_SUPER_H

/* This module implements Antisupercirce */

#include "solving/solve.h"

square antisupercirce_next_rebirth_square(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type antisupercirce_rebirth_handler_solve(slice_index si,
                                                       stip_length_type n);

/* Instrument a stipulation for strict Circe rebirths
 * @param si identifies root slice of stipulation
 */
void stip_insert_antisupercirce_rebirth_handlers(slice_index si);

#endif
