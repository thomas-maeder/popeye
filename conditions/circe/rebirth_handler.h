#if !defined(CONDITIONS_CIRCE_REBIRTH_HANDLER_H)
#define CONDITIONS_CIRCE_REBIRTH_HANDLER_H

#include "solving/solve.h"
#include "solving/move_effect_journal.h"

/* Execute a Circe rebirth.
 * This is a helper function for alternative Circe types
 * @param reason reason for rebirth
 * @param sq_rebirth rebirth square
 * @param pi_reborn type of piece to be reborn
 * @param spec_reborn flags of the piece to be reborn
 */
void circe_do_rebirth(move_effect_reason_type reason,
                      square sq_rebirth, piece pi_reborn, Flags spec_reborn);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type circe_rebirth_handler_solve(slice_index si,
                                              stip_length_type n);

/* Instrument a stipulation for strict Circe rebirths
 * @param si identifies root slice of stipulation
 */
void stip_insert_circe(slice_index si);

#endif
