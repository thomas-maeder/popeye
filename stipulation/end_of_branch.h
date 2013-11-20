#if !defined(STIPULATION_END_OF_BRANCH_H)
#define STIPULATION_END_OF_BRANCH_H

/* End of branch slices - they decide that when to continue play in branch
 * and when to change to slice representing subsequent play.
 * There are 4 different slice types - there main difference is there position
 * in the sequence of slices. E.g. we must not test for self-check before
 * testing for having reached ## - but we must test for self-check before many
 * other ends.
 */

#include "solving/solve.h"

/* Allocate a STEndOfBranch slice.
 * @param to_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_end_of_branch_slice(slice_index to_goal);

/* Allocate a STEndOfBranchGoal slice
 * @param proxy_to_goal identifies slice that leads towards goal from
 *                      the branch
 * @return index of allocated slice
 */
slice_index alloc_end_of_branch_goal(slice_index proxy_to_goal);

/* Allocate a STEndOfBranchGoalImmobile slice
 * @param proxy_to_goal identifies slice that leads towards goal from
 *                      the branch
 * @return index of allocated slice
 */
slice_index alloc_end_of_branch_goal_immobile(slice_index proxy_to_goal);

/* Allocate a STEndOfBranchForced slice
 * @param proxy_to_goal identifies slice that leads towards goal from
 *                      the branch
 * @return index of allocated slice
 */
slice_index alloc_end_of_branch_forced(slice_index proxy_to_goal);

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
stip_length_type end_of_branch_solve(slice_index si, stip_length_type n);

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
stip_length_type end_of_branch_goal_solve(slice_index si, stip_length_type n);

#endif
