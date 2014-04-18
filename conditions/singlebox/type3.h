#if !defined(CONDITIONS_SINGLEBOX_TYPE3_H)
#define CONDITIONS_SINGLEBOX_TYPE3_H

/* Implementation of condition Singlebox Type 3
 */

#include "pieces/pieces.h"
#include "solving/machinery/solve.h"
#include "solving/observation.h"

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_singlebox_type3(slice_index si);

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void singlebox_type3_pawn_promoter_solve(slice_index si);

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void singlebox_type3_legality_tester_solve(slice_index si);

/* Make sure to behave correctly while detecting observations by latent pawns
* @param si identifies tester slice
* @return true iff observation is valid
*/
boolean singleboxtype3_enforce_observer_walk(slice_index si);

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void singleboxtype3_generate_moves_for_piece(slice_index si);

#endif
