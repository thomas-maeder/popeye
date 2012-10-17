#if !defined(CONDITIONS_CIRCE_SUPER_H)
#define CONDITIONS_CIRCE_SUPER_H

#include "solving/solve.h"

/* This module implements Supercirce */

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type supercirce_no_rebirth_fork_solve(slice_index si,
                                                   stip_length_type n);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type supercirce_rebirth_handler_solve(slice_index si,
                                                   stip_length_type n);

/* Instrument a stipulation for Super Circe
 * @param si identifies root slice of stipulation
 */
void stip_insert_supercirce_rebirth_handlers(slice_index si);

#endif
