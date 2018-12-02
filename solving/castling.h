#if !defined(SOLVING_CASTLING_H)
#define SOLVING_CASTLING_H

/* This module implements castling.
 */

#include "stipulation/slice_type.h"
#include "solving/machinery/solve.h"
#include "solving/move_effect_journal.h"
#include "position/position.h"
#include "pieces/pieces.h"

extern castling_rights_type castling_mutual_exclusive[nr_sides][2];
extern castling_rights_type castling_flags_no_castling;

#define TSTCASTLINGFLAGMASK(side,mask) TSTFLAGMASK(being_solved.castling_rights>>(side)*black_castling_rights_offset,(mask))
#define SETCASTLINGFLAGMASK(side,mask) SETFLAGMASK(being_solved.castling_rights,(mask)<<((side)*black_castling_rights_offset))
#define CLRCASTLINGFLAGMASK(side,mask) CLRFLAGMASK(being_solved.castling_rights,(mask)<<((side)*black_castling_rights_offset))

/* Enable castling rights for the piece that just arrived (for whatever reason)
 * on a square
 * @param reason why
 * @param on the arrival square
 */
void enable_castling_rights(move_effect_reason_type reason,
                            square sq_arrival);

/* Disable castling rights for the piece that just left (for whatever reason)
 * a square
 * @param reason why
 * @param on the square left
 */
void disable_castling_rights(move_effect_reason_type reason,
                             square sq_departure);

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
void castling_player_solve(slice_index si);

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
void castling_rights_adjuster_solve(slice_index si);

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void castling_generator_generate_castling(slice_index si);

/* Instrument the solving machinery with castling
 * @param si identifies root slice of solving machinery
 */
void solving_initialise_castling(slice_index si);

/* disable castling altogether
* @param si identifies root slice of solving machinery
*/
void solving_disable_castling(slice_index si);

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void castling_generate_test_departure(slice_index si);

/* make sure that the king's departure square is tested before a castling is
 * generated for a king
 * @param si identifies root slice of solving machinery
 */
void castling_generation_test_departure(slice_index si);

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
void mutual_castling_rights_adjuster_solve(slice_index si);

/* Instrument a stipulation with alternative move player slices
 * @param si identifies root slice of stipulation
 */
void insert_alternative_move_players(slice_index si, slice_type type);

/* Instrument the solving machinery with mutual castling right adjusters
 * @param si identifies the root slice of the solving machinery
 */
void solving_insert_mutual_castling_rights_adjusters(slice_index si);

void generate_castling(void);

boolean castling_is_intermediate_king_move_legal(Side side, square to);

#endif
