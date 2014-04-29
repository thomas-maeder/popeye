#if !defined(CONDITIONS_OSCILLATING_KINGS_H)
#define CONDITIONS_OSCILLATING_KINGS_H

/* This module implements the condition Oscillating Kings */

#include "conditions/conditions.h"
#include "solving/machinery/solve.h"

extern ConditionLetteredVariantType OscillatingKings[nr_sides];

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
void oscillating_kings_type_a_solve(slice_index si);
void oscillating_kings_type_b_solve(slice_index si);
void oscillating_kings_type_c_solve(slice_index si);

/* Instrument a stipulation for Oscillating Kings
 * @param si identifies root slice of stipulation
 */
void solving_insert_king_oscillators(slice_index si);

#endif
