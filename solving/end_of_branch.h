#if !defined(SOLVING_END_OF_BRANCH_H)
#define SOLVING_END_OF_BRANCH_H

/* End of branch slices - they decide that when to continue play in branch
 * and when to change to slice representing subsequent play.
 * There are 4 different slice types - there main difference is there position
 * in the sequence of slices. E.g. we must not test for self-check before
 * testing for having reached ## - but we must test for self-check before many
 * other ends.
 */

#include "solving/machinery/solve.h"

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
void end_of_branch_solve(slice_index si);

#endif
