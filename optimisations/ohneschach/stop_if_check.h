#if !defined(CONDITIONS_OHNESCHACH_STOP_IF_CHECK_H)
#define CONDITIONS_OHNESCHACH_STOP_IF_CHECK_H

/* This module provides support for optimising away unnecessarily strict
 * STOhneschachStopIfCheckAndNotMate slices (by replacing them with
 * STOhneschachStopIfCheck slices that ignore immobility).
 * An optimisation happens in two steps:
 * - a traversal which invokes ohneschach_stop_if_check_plan_to_optimise_away_stop()
 *   for all STOhneschachStopIfCheckAndNotMate slices reachable from root
 * - an invokation of ohneschach_stop_if_check_execute_optimisations to
 *   actually perform the planned optimisations
 */

#include "solving/solve.h"

/* Plan optimising away a STOhneschachStopIfCheckAndNotMate slice
 * @param stop identifies the slice
 * @param to_be_optimised true iff stop is planned to be optimised away
 */
void ohneschach_stop_if_check_plan_to_optimise_away_stop(slice_index stop,
                                                         boolean to_be_optimised);

/* Execute the optimisations planned before and communicated using a series of
 * invokations of ohneschach_stop_if_check_plan_to_optimise_away_stop()
 * @param root root slice of the stiptulation
 */
void ohneschach_stop_if_check_execute_optimisations(slice_index root);

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
stip_length_type ohneschach_stop_if_check_solve(slice_index si,
                                                stip_length_type n);

#endif
