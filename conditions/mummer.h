#if !defined(CONDITIONS_MUMMER_H)
#define CONDITIONS_MUMMER_H

/* This module implements "mummer" conditions (maximummer, minimiummer etc.) */

#include "solving/solve.h"

typedef enum
{
  mummer_strictness_none,
  mummer_strictness_regular,
  mummer_strictness_exact,
  mummer_strictness_ultra,

  nr_mummer_strictness
} mummer_strictness_type;

extern mummer_strictness_type mummer_strictness[nr_sides];

extern mummer_strictness_type mummer_strictness_default_side;

typedef int (*mummer_length_measurer_type)(square departure,
                                           square arrival,
                                           square capture);

/* Determine the length of a move for the Maximummer condition; the higher the
 * value the more likely the move is going to be played.
 * @param sq_departure departure square
 * @param sq_arrival arrival square
 * @param sq_capture capture square
 * @return a value expressing the precedence of this move
 */
int maximummer_measure_length(square sq_departure, square sq_arrival, square sq_capture);

/* Determine the length of a move for the Minimummer condition; the higher the
 * value the more likely the move is going to be played.
 * @param sq_departure departure square
 * @param sq_arrival arrival square
 * @param sq_capture capture square
 * @return a value expressing the precedence of this move
 */
int minimummer_measure_length(square sq_departure, square sq_arrival, square sq_capture);

/* Forget previous mummer activations and definition of length measurers */
void mummer_reset_length_measurers(void);

/* Activate mummer for a side and define the length measurer to be used.
 * @param side Side for which to activate mummer
 * @param measurer length measurer to be used
 * @return false iff mummer was already activated for side
 */
boolean mummer_set_length_measurer(Side side,
                                   mummer_length_measurer_type measurer);

/* Validate an observation according to Ultra-Mummer
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean ultra_mummer_validate_observation(slice_index si,
                                          square sq_observer,
                                          square sq_landing,
                                          square sq_observee);

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
stip_length_type ultra_mummer_measurer_deadend_solve(slice_index si,
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
stip_length_type mummer_orchestrator_solve(slice_index si, stip_length_type n);

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
stip_length_type mummer_bookkeeper_solve(slice_index si, stip_length_type n);

/* Initialise solving in mummer
 * @param si identifies root slice of stipulation
 */
void mummer_initialise_solving(slice_index si);

#endif
