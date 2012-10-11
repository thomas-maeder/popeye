#if !defined(CONDITIONS_CIRCE_REBIRTH_HANDLER_H)
#define CONDITIONS_CIRCE_REBIRTH_HANDLER_H

#include "solving/solve.h"
#include "solving/move_effect_journal.h"
#include "stipulation/slice_type.h"

extern pilecase current_circe_rebirth_square;
extern piece current_circe_reborn_piece[maxply+1];
extern Flags current_circe_reborn_spec[maxply+1];

/* Execute a Circe rebirth.
 * This is a helper function for alternative Circe types
 * @param reason reason for rebirth
 */
void circe_do_rebirth(move_effect_reason_type reason);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type circe_determine_reborn_piece_solve(slice_index si,
                                                    stip_length_type n);

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

/* Use an alternative type of slices for determining the reborn piece
 * @param si identifies root slice of stipulation
 * @param substitute substitute slice type
 */
void stip_replace_circe_determine_reborn_piece(slice_index si,
                                               slice_type substitute);

/* Instrument a stipulation for strict Circe rebirths
 * @param si identifies root slice of stipulation
 */
void stip_insert_circe(slice_index si);

#endif
