#if !defined(CONDITIONS_CIRCE_CAPTURE_FORK_H)
#define CONDITIONS_CIRCE_CAPTURE_FORK_H

/* bypass Circe slices if there is no capture */

#include "stipulation/slice_type.h"
#include "solving/solve.h"

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type circe_capture_fork_solve(slice_index si,
                                           stip_length_type n);

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_circe_capture_forks(slice_index si);

/* Instrument a stipulation with a type of "Circe rebirth avoiders" (i.e.
 * slices that may detour around Circe rebirth under a certain condition;
 * STCaptureFork is an example).
 * @param si identifies root slice of stipulation
 * @param type tye of Circe rebirth avoider
 */
void stip_insert_rebirth_avoider(slice_index si, slice_type type);

#endif
