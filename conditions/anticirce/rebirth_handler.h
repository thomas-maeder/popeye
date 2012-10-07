#if !defined(CONDITIONS_ANTICIRCE_REBIRTH_HANDLER_H)
#define CONDITIONS_ANTICIRCE_REBIRTH_HANDLER_H

#include "solving/solve.h"
#include "solving/move_effect_journal.h"

extern PieNam current_promotion_of_reborn_moving[maxply+1];

/* Perform an Anticirce rebirth
 * @param reason reason for rebirth
 */
void anticirce_do_rebirth(move_effect_reason_type reason);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type anticirce_rebirth_handler_solve(slice_index si,
                                                   stip_length_type n);

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

/* Instrument a stipulation for strict Circe rebirths
 * @param si identifies root slice of stipulation
 */
void stip_insert_anticirce_rebirth_handlers(slice_index si);

#endif
