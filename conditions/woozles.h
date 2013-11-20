#if !defined(CONDITIONS_WOOZLES_H)
#define CONDITIONS_WOOZLES_H

#include "stipulation/stipulation.h"
#include "utilities/boolean.h"

/* This module implements the condition Woozles */

extern boolean woozles_rex_exclusive;

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
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
 * @return true iff the observation is valid
 */
boolean woozles_validate_observation(slice_index si);

/* Validate an observation according to BiWoozles
 * @return true iff the observation is valid
 */
boolean biwoozles_validate_observation(slice_index si);

/* Validate an observation according to Heffalumps
 * @return true iff the observation is valid
 */
boolean heffalumps_validate_observation(slice_index si);

/* Validate an observation according to BiHeffalumps
 * @return true iff the observation is valid
 */
boolean biheffalumps_validate_observation(slice_index si);

/* Instrument solving in Woozles
 * @param si identifies the root slice of the stipulation
 */
void woozles_initialise_solving(slice_index si);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
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
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
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
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
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
