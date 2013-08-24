#if !defined(CONDITIONS_CIRCE_REBIRTH_HANDLER_H)
#define CONDITIONS_CIRCE_REBIRTH_HANDLER_H

/* This module implements regular Circe and provides hooks for the
 * implementation of Circe variations
 */

#include "pieces/pieces.h"
#include "solving/solve.h"
#include "stipulation/slice_type.h"
#include "solving/move_effect_journal.h"
#include "solving/ply.h"
#include "position/position.h"

extern square current_circe_rebirth_square[maxply+1];

extern PieNam current_circe_reborn_piece[maxply+1];
extern Flags current_circe_reborn_spec[maxply+1];

extern PieNam current_circe_relevant_piece[maxply+1];
extern Flags current_circe_relevant_spec[maxply+1];
extern Side current_circe_relevant_side[maxply+1];

extern move_effect_reason_type current_circe_rebirth_reason[maxply+1];

extern square (*circe_determine_rebirth_square)(PieNam, Flags, square, square, square, Side);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
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
 *                                     unintended immobility on the next move
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
 *                                     unintended immobility on the next move
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
 *                                     unintended immobility on the next move
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


square renfile(PieNam p, Flags pspec, square j, square i, square ip, Side camp);
square renrank(PieNam p, Flags pspec, square j, square i, square ip, Side camp);
square renspiegelfile(PieNam p, Flags pspec, square j, square i, square ip, Side camp);
square renpwc(PieNam p, Flags pspec, square j, square i, square ip, Side camp);
square renequipollents(PieNam p, Flags pspec, square j, square i, square ip, Side camp);
square renequipollents_anti(PieNam p, Flags pspec, square j, square i, square ip, Side camp);
square rensymmetrie(PieNam p, Flags pspec, square j, square i, square ip, Side camp);
square renantipoden(PieNam p, Flags pspec, square j, square i, square ip, Side camp);
square rendiagramm(PieNam p, Flags pspec, square j, square i, square ip, Side camp);
square rennormal(PieNam p, Flags pspec, square j, square i, square ip, Side camp);
square renspiegel(PieNam p, Flags pspec, square j, square i, square ip, Side camp);
square rendiametral(PieNam p, Flags pspec, square j, square i, square ia, Side camp);

#endif
