#if !defined(CONDITIONS_ANTICIRCE_SUPER_H)
#define CONDITIONS_ANTICIRCE_SUPER_H

/* This module implements Antisupercirce */

#include "solving/solve.h"

square antisupercirce_next_rebirth_square(void);

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
stip_length_type antisupercirce_determine_rebirth_square_solve(slice_index si,
                                                               stip_length_type n);

/* Instrument a stipulation for Antisupercirce
 * @param si identifies root slice of stipulation
 */
void stip_insert_antisupercirce(slice_index si);

#endif
