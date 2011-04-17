#if !defined(PYSELFGD_H)
#define PYSELFGD_H

/* In self stipulations, guard against defenses that prematurely reach
 * the goal
 */

#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/defense_play.h"

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type self_defense_can_attack(slice_index si,
                                         stip_length_type n,
                                         stip_length_type n_max_unsolvable);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @note n==n_max_unsolvable means that we are solving refutations
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type self_defense_attack(slice_index si,
                                     stip_length_type n,
                                     stip_length_type n_max_unsolvable);

/* Instrument a branch with STSelfAttack and STSelfDefense slices
 * @param si root of branch to be instrumented
 * @param proxy_to_goal identifies slice leading towards goal
 */
void slice_insert_self_guards(slice_index si, slice_index proxy_to_goal);

/* Instrument a branch leading to a goal to be a self goal branch
 * @param si identifies entry slice of branch
 */
void slice_make_self_goal_branch(slice_index si);

#endif
