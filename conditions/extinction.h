#if !defined(CONDITIONS_EXTINCTION_H)
#define CONDITIONS_EXTINCTION_H

/* Implementation of condition Extinction chess
 */

#include "solving/solve.h"

/* Determine whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean exctinction_all_square_observation_tester_is_in_check(slice_index si,
                                                              Side side_attacked);

/* Determine whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean extinction_extincted_tester_solve(slice_index si, Side side_in_check);

/* Initialise the solving machinery with Extinction Chess
 * @param si identifies root slice of stipulation
 */
void extinction_initialise_solving(slice_index si);

#endif
