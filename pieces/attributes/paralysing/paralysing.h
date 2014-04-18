#if !defined(PIECES_PARALYSING_H)
#define PIECES_PARALYSING_H

#include "pieces/pieces.h"
#include "stipulation/stipulation.h"
#include "utilities/boolean.h"

/* Validate an observation geometry according to Paralysing pieces
 * @return true iff the observation is valid
 */
boolean paralysing_validate_observation_geometry(slice_index si);

/* Validate an observer according to Paralysing pieces
 * @return true iff the observation is valid
 */
boolean paralysing_validate_observer(slice_index si);

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void paralysing_generate_moves_for_piece(slice_index si);

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
void paralysing_suffocation_finder_solve(slice_index si);

/* Determine whether a side is "suffocated by paralysis", i.e. would the side
 * have moves (possibly exposing the side to self check) if no piece were
 * paralysing.
 * @param side side for which to determine whether it is suffocated
 * @return true iff side is suffocated by paralysis
 */
boolean suffocated_by_paralysis(Side side);

/* Initialise solving with paralysing pieces
 * @param si root of branch to be instrumented
 */
void paralysing_initialise_solving(slice_index si);

#endif
