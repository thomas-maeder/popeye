#if !defined(CONDITIONS_ANTICIRCE_PROMOTION_H)
#define CONDITIONS_ANTICIRCE_PROMOTION_H

#include "solving/solve.h"

extern PieNam current_promotion_of_reborn_moving[maxply+1];

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type anticirce_reborn_promoter_solve(slice_index si,
                                                  stip_length_type n);

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_anticirce_promotion(slice_index si);

#endif
