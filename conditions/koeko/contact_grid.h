#if !defined(CONDITIONS_KOEKO_CONTACT_GRID_H)
#define CONDITIONS_KOEKO_CONTACT_GRID_H

#include "solving/solve.h"

/* This module implements the condition Contact Grid */

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type contact_grid_legality_tester_solve(slice_index si,
                                                    stip_length_type n);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type contact_grid_avoid_circe_rebirth(slice_index si,
                                                  stip_length_type n);

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_contact_grid(slice_index si);

#endif
