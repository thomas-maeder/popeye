#if !defined(CONDITIONS_CIRCE_REBIRTH_HANDLER_H)
#define CONDITIONS_CIRCE_REBIRTH_HANDLER_H

/* This module implements regular Circe and provides hooks for the
 * implementation of Circe variations
 */

#include "solving/solve.h"
#include "stipulation/slice_type.h"

extern pilecase current_circe_rebirth_square;

extern piece current_circe_reborn_piece[maxply+1];
extern Flags current_circe_reborn_spec[maxply+1];

extern piece current_circe_relevant_piece[maxply+1];
extern Flags current_circe_relevant_spec[maxply+1];
extern Side current_circe_relevant_side[maxply+1];

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type circe_determine_reborn_piece_solve(slice_index si,
                                                    stip_length_type n);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type circe_determine_relevant_piece_solve(slice_index si,
                                                      stip_length_type n);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type circe_determine_rebirth_square_solve(slice_index si,
                                              stip_length_type n);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type circe_place_reborn_solve(slice_index si, stip_length_type n);

/* Use an alternative type of slices for determining the reborn piece
 * @param si identifies root slice of stipulation
 * @param substitute substitute slice type
 */
void stip_replace_circe_determine_reborn_piece(slice_index si,
                                               slice_type substitute);

/* Use an alternative type of slices for determining the piece relevant for
 * determining the rebirth square
 * @param si identifies root slice of stipulation
 * @param substitute substitute slice type
 */
void stip_replace_circe_determine_relevant_piece(slice_index si,
                                                 slice_type substitute);

/* Instrument a stipulation for Circe
 * @param si identifies root slice of stipulation
 */
void stip_insert_circe(slice_index si);

#endif
