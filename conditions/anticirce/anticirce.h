#if !defined(CONDITIONS_ANTICIRCE_ANTICIRCE_H)
#define CONDITIONS_ANTICIRCE_ANTICIRCE_H

#include "solving/machinery/solve.h"
#include "conditions/circe/circe.h"

extern circe_variant_type anticirce_variant;

/* Reset a circe_variant object to the default values
 * @param variant address of the variant object to be reset
 */
void anticirce_reset_variant(circe_variant_type *variant);

/* Instrument the solving machinery with AntiCirce
 * @param si identifies the root slice of the solving machinery
 */
void anticirce_initialise_solving(slice_index si);

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
void anticirce_remove_capturer_solve(slice_index si);

#endif
