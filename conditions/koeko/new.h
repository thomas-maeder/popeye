#if !defined(CONDITIONS_KOEKO_NEW_H)
#define CONDITIONS_KOEKO_NEW_H

#include "py.h"

/* This module implements the condition New-Koeko */

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type newkoeko_remember_contact_solve(slice_index si,
                                                 stip_length_type n);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type newkoeko_legality_tester_solve(slice_index si, stip_length_type n);

/* Instrument the solvers with New-Koeko
 * @param si identifies the root slice of the stipulation
 */
void stip_insert_newkoeko(slice_index si);

#endif
