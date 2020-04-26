#if !defined(CONDITIONS_MASAND_H)
#define CONDITIONS_MASAND_H

#include "solving/machinery/solve.h"

/* Validate an observation or observer by making sure it's the checking piece
 * @param si identifies the validator slice
 */
boolean masand_enforce_observer(slice_index si);

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
void masand_recolorer_solve(slice_index si);

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void solving_insert_masand(slice_index si);

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
void masand_generalised_recolorer_solve(slice_index si);

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void solving_insert_masand_generalised(slice_index si);

#endif
