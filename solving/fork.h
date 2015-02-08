#if !defined(SOLVING_FORK_H)
#define SOLVING_FORK_H

/* Forks
 */

#include "solving/machinery/solve.h"
#include "solving/observation.h"

/* Delegate solving to next2
 * @param si identifies the fork
 */
void fork_solve_delegate(slice_index si);

/* Delegate testing observation to next2
 * @param si identifies the fork
 */
void fork_is_square_observed_delegate(slice_index si);

/* Delegate testing observation to next2
 * @param si identifies the fork
 * @return true iff the target square is observed
 */
boolean fork_is_square_observed_nested_delegate(slice_index si,
                                                validator_id evaluate);

/* Delegate generating to next2
 * @param si identifies the fork
 */
void fork_move_generation_delegate(slice_index si);


/* Delegate validating to next2
 * @param si identifies the fork
 * @return true iff the observation is valid
 */
boolean fork_validate_observation_recursive_delegate(slice_index si);

/* Delegate testing to next2
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean fork_is_in_check_recursive_delegate(slice_index si, Side side_in_check);

#endif
