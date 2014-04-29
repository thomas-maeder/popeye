#if !defined(STIPULATION_CONSTRAINT_H)
#define STIPULATION_CONSTRAINT_H

/* Stop solving unless a condition is *not* met.
 */

#include "stipulation/structure_traversal.h"
#include "solving/moves_traversal.h"

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

/* Remove goal checker slices that we know can't possibly be met
 * @param si identifies entry slice to stipulation
 */
void solving_remove_irrelevant_constraints(slice_index si);

#endif
