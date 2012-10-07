#if !defined(CONDITIONS_CIRCE_KING_REBIRTH_AVOIDER_H)
#define CONDITIONS_CIRCE_KING_REBIRTH_AVOIDER_H

/* Avoid king rebirths in conditions like Isardam where kings may be under
 * (apparent) attack
 */

#include "solving/solve.h"

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type circe_king_rebirth_avoider_solve(slice_index si,
                                                   stip_length_type n);

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_circe_king_rebirth_avoiders(slice_index si);

#endif
