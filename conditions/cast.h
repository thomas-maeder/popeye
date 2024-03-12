#if !defined(CONDITIONS_CAST_H)
#define CONDITIONS_CAST_H

/* This module implements the condition CAST */

#include "solving/machinery/solve.h"

typedef enum
{
  cast_regular,
  cast_inverse,

  nr_cast_modes
} cast_mode_type;

extern cast_mode_type cast_mode;


/* Validate an observation according to CAST
 * @return true iff the observation is valid
 */
boolean cast_remove_illegal_captures_solve(slice_index si);

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
void cast_multi_captures_remover_solve(slice_index si);

/* Instrument the solving machinery for CAST
 * @param si identifies root slice of stipulation
 */
void cast_initialise_solving(slice_index si);

#endif
