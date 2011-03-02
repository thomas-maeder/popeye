#if !defined(STIPULATION_REFLEX_ATTACK_SOLVER_H)
#define STIPULATION_REFLEX_ATTACK_SOLVER_H

/* In reflex stipulations, find forced (in the diagram or by reflex-specific
 * refutations) attacks
 */

#include "pyslice.h"
#include "stipulation/battle_play/attack_play.h"

/* Allocate a STStipulationReflexAttackSolver slice
 * @param avoided prototype of slice that must not be solvable
 * @return index of allocated slice
 */
slice_index alloc_reflex_attack_solver(slice_index avoided);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type reflex_attack_solver_solve(slice_index si);

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type reflex_attack_solver_has_solution(slice_index si);

#endif
