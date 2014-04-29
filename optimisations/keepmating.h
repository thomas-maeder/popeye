#if !defined(OPTIMISATIONS_KEEPMATING_H)
#define OPTIMISATIONS_KEEPMATING_H

/* Implementation of the "keep mating piece" optimisation:
 * Solving stops once the last piece of the mating side that could
 * deliver mate has been captured.
 */

#include "solving/machinery/solve.h"

/* Instrument stipulation with STKeepMatingFilter slices
 * @param si identifies slice where to start
 */
void solving_insert_keepmating_filters(slice_index si);


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
void keepmating_filter_solve(slice_index si);

#endif
