#if !defined(STIPULATION_CONSTRAINT_H)
#define STIPULATION_CONSTRAINT_H

/* Stop solving unless a condition is *not* met.
 */

#include "stipulation/structure_traversal.h"
#include "stipulation/moves_traversal.h"
#include "solving/solve.h"

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
stip_length_type constraint_solve(slice_index si, stip_length_type n);

/* Remove goal checker slices that we know can't possibly be met
 * @param si identifies entry slice to stipulation
 */
void stip_remove_irrelevant_constraints(slice_index si);

#endif
