#include "stipulation/reflex_attack_solver.h"
#include "pybrafrk.h"
#include "pypipe.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/help_play/play.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STStipulationReflexAttackSolver slice
 * @param avoided prototype of slice that must not be solvable
 * @return index of allocated slice
 */
slice_index alloc_reflex_attack_solver(slice_index avoided)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",avoided);
  TraceFunctionParamListEnd();

  /* ab(use) the fact that .avoided and .towards_goal are collocated */
  result = alloc_branch_fork(STStipulationReflexAttackSolver,0,0,avoided);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type reflex_attack_solver_solve(slice_index si)
{
  has_solution_type result;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slice_solve(avoided)==has_solution)
    result = slice_solve(next);
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type reflex_attack_solver_has_solution(slice_index si)
{
  has_solution_type result;
  slice_index const avoided = slices[si].u.reflex_guard.avoided;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slice_has_solution(avoided)==has_solution)
    result = slice_has_solution(next);
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
