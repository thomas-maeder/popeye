#if !defined(CONDITIONS_WOOZLES_H)
#define CONDITIONS_WOOZLES_H

#include "stipulation/stipulation.h"
#include "utilities/boolean.h"

/* This module implements the condition Woozles */

extern boolean woozles_rex_inclusive;

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
void woozles_remove_illegal_captures_solve(slice_index si);

/* Validate an observer according to Woozles
 * @return true iff the observer can observe
 */
boolean woozles_validate_observer(slice_index si);

/* Validate an observer according to BiWoozles
 * @return true iff the observer can observe
 */
boolean biwoozles_validate_observer(slice_index si);

/* Validate an observer according to Heffalumps
 * @return true iff the observer can observe
 */
boolean heffalumps_validate_observer(slice_index si);

/* Validate an observer according to BiHeffalumps
 * @return true iff the observer can observe
 */
boolean biheffalumps_validate_observer(slice_index si);

/* Instrument solving in Woozles
 * @param si identifies the root slice of the stipulation
 */
void woozles_initialise_solving(slice_index si);

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
void biwoozles_remove_illegal_captures_solve(slice_index si);

/* Instrument solving in BiWoozles
 * @param si identifies the root slice of the stipulation
 */
void biwoozles_initialise_solving(slice_index si);

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
void heffalumps_remove_illegal_captures_solve(slice_index si);

/* Instrument solving in Heffalumps
 * @param si identifies the root slice of the stipulation
 */
void heffalumps_initialise_solving(slice_index si);

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
void biheffalumps_remove_illegal_captures_solve(slice_index si);

/* Instrument solving in BiHeffalumps
 * @param si identifies the root slice of the stipulation
 */
void biheffalumps_initialise_solving(slice_index si);

#endif
