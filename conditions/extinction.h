#if !defined(CONDITIONS_EXTINCTION_H)
#define CONDITIONS_EXTINCTION_H

/* Implementation of condition Extinction chess
 */

#include "position/side.h"
#include "solving/machinery/solve.h"

/* Determine whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean exctinction_all_piece_observation_tester_is_in_check(slice_index si,
                                                             Side side_attacked);

/* Determine whether a side is in check
 * @param si identifies the check tester
 * @return true iff side_in_check is in check according to slice si
 */
void extinction_extincted_tester_solve(slice_index si);

/* Initialise the solving machinery with Extinction Chess
 * @param si identifies root slice of stipulation
 */
void extinction_initialise_solving(slice_index si);

#endif
