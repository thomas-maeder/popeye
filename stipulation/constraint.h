#if !defined(STIPULATION_CONSTRAINT_H)
#define STIPULATION_CONSTRAINT_H

/* Stop solving unless a condition is *not* met.
 */

#include "stipulation/structure_traversal.h"
#include "stipulation/moves_traversal.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/defense_play.h"

/* Allocate a STConstraintSolver slice
 * @param proxy_to_constraint prototype of slice that must not be solvable
 * @return index of allocated slice
 */
slice_index alloc_constraint_solver_slice(slice_index proxy_to_constraint);

/* Allocate a STConstraintTester slice
 * @param proxy_to_constraint prototype of slice that must not be solvable
 * @return index of allocated slice
 */
slice_index alloc_constraint_tester_slice(slice_index proxy_to_constraint);

/* Allocate a STGoalConstraintTester slice
 * @param proxy_to_constraint prototype of slice that must not be solvable
 * @return index of allocated slice
 */
slice_index alloc_goal_constraint_tester_slice(slice_index proxy_to_constraint);

/* Spin a STContraintSolver slice off a STContraintTester slice to add it to the
 * root or set play branch
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void constraint_tester_make_root(slice_index si, stip_structure_traversal *st);

/* Copy a STGoalContraintTester slice to add it to the root or set play branch
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void goal_constraint_tester_make_root(slice_index si, stip_structure_traversal *st);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type constraint_attack(slice_index si, stip_length_type n);

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
stip_length_type constraint_defend(slice_index si, stip_length_type n);

/* Remove goal checker slices that we know can't possibly be met
 * @param si identifies entry slice to stipulation
 */
void stip_remove_irrelevant_constraints(slice_index si);

#endif
