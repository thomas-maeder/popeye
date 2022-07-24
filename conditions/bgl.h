#if !defined(CONDITIONS_BGL_H)
#define CONDITIONS_BGL_H

#include "solving/machinery/solve.h"
#include "solving/move_effect_journal.h"

/* This module implements the BGL condition */

extern long int BGL_values[nr_sides];
extern boolean BGL_global;

enum
{
  BGL_infinity = 10000000   /* this will do I expect; e.g. max len = 980 maxply < 1000 */
};

/* Validate an observation according to BGL
 * @return true iff the observation is valid
 */
boolean bgl_validate_observation(slice_index si);

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
void bgl_enforcer_solve(slice_index si);

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
void bgl_adjuster_solve(slice_index si);

/* Initialise solving with BGL
 */
void bgl_initialise_solving(slice_index si);

#endif
