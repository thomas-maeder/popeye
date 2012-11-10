#if !defined(CONDITIONS_SINGLEBOX_TYPE3_H)
#define CONDITIONS_SINGLEBOX_TYPE3_H

/* Implementation of condition Singlebox Type 3
 */

#include "solving/solve.h"

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_singlebox_type3(slice_index si);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type singlebox_type3_pawn_promoter_solve(slice_index si,
                                                      stip_length_type n);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type singlebox_type3_legality_tester_solve(slice_index si,
                                                        stip_length_type n);

#endif
