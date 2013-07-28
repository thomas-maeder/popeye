#if !defined(CONDITIONS_WOOZLES_H)
#define CONDITIONS_WOOZLES_H

#include "py.h"

/* This module implements the condition Woozles */

extern boolean woozles_rex_exclusive;

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
stip_length_type woozles_remove_illegal_captures_solve(slice_index si,
                                                       stip_length_type n);

/* Validate an observation according to Woozles
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean woozles_validate_observation(slice_index si,
                                     square sq_observer,
                                     square sq_landing,
                                     square sq_observee);

/* Validate an observation according to BiWoozles
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean biwoozles_validate_observation(slice_index si,
                                       square sq_observer,
                                       square sq_landing,
                                       square sq_observee);

/* Validate an observation according to Heffalumps
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean heffalumps_validate_observation(slice_index si,
                                        square sq_observer,
                                        square sq_landing,
                                        square sq_observee);

/* Validate an observation according to BiHeffalumps
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean biheffalumps_validate_observation(slice_index si,
                                          square sq_observer,
                                          square sq_landing,
                                          square sq_observee);

/* Instrument solving in Woozles
 * @param si identifies the root slice of the stipulation
 */
void woozles_initialise_solving(slice_index si);

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
stip_length_type biwoozles_remove_illegal_captures_solve(slice_index si,
                                                         stip_length_type n);

/* Instrument solving in BiWoozles
 * @param si identifies the root slice of the stipulation
 */
void biwoozles_initialise_solving(slice_index si);

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
stip_length_type heffalumps_remove_illegal_captures_solve(slice_index si,
                                                          stip_length_type n);

/* Instrument solving in Heffalumps
 * @param si identifies the root slice of the stipulation
 */
void heffalumps_initialise_solving(slice_index si);

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
stip_length_type biheffalumps_remove_illegal_captures_solve(slice_index si,
                                                            stip_length_type n);

/* Instrument solving in BiHeffalumps
 * @param si identifies the root slice of the stipulation
 */
void biheffalumps_initialise_solving(slice_index si);

#endif
