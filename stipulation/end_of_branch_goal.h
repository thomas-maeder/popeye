#if !defined(STIPULATION_END_OF_BRANCH_GOAL_H)
#define STIPULATION_END_OF_BRANCH_GOAL_H

/* End of branch by reaching goal
 */

#include "solving/battle_play/attack_play.h"
#include "solving/battle_play/defense_play.h"

/* Allocate a STEndOfBranchGoal slice
 * @param proxy_to_goal identifies slice that leads towards goal from
 *                      the branch
 * @return index of allocated slice
 */
slice_index alloc_end_of_branch_goal(slice_index proxy_to_goal);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type end_of_branch_goal_attack(slice_index si, stip_length_type n);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type end_of_branch_goal_defend(slice_index si, stip_length_type n);

#endif
