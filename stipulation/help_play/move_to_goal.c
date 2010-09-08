#include "stipulation/help_play/move_to_goal.h"
#include "pydata.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_moves_generation.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/help_play/play.h"
#include "stipulation/help_play/root.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STHelpMoveToGoal slice.
 * @param goal goal to be reached
 * @return index of allocated slice
 */
slice_index alloc_help_move_to_goal_slice(Goal goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal.type);
  TraceFunctionParamListEnd();

  result = alloc_branch(STHelpMoveToGoal,
                        slack_length_help+1,slack_length_help+1);
  slices[result].u.branch.imminent_goal = goal;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try solving with all generated moves
 * @param si slice index
 * @param n exact number of moves to reach the end state
 * @return true iff a solution was found
 */
static boolean foreach_move_solve(slice_index si)
{
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const length = slack_length_help;
  boolean result = false;

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && help_solve_in_n(next,length)==length)
      result = true;

    repcoup();
  }

  return result;
}

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type help_move_to_goal_solve_in_n(slice_index si,
                                              stip_length_type n)
{
  stip_length_type result = slack_length_help+3;
  Side const side_at_move = slices[si].starter;
  Goal const goal = slices[si].u.branch.imminent_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(goal.type!=no_goal);
  assert(n==slack_length_help+1);

  empile_for_goal = goal;
  generate_move_reaching_goal(side_at_move);
  empile_for_goal.type = no_goal;

  if (foreach_move_solve(si))
    result = slack_length_help+1;

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Iterate moves until a solution has been found
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return true iff a solution was found
 */
static boolean find_solution(slice_index si)
{
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const length = slack_length_help;
  boolean result = false;
  
  while (encore())
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && help_has_solution_in_n(next,length)==length)
    {
      result = true;
      repcoup();
      break;
    }
    else
      repcoup();

  return result;
}

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type help_move_to_goal_has_solution_in_n(slice_index si,
                                                     stip_length_type n)
{
  Side const side_at_move = slices[si].starter;
  stip_length_type result = slack_length_help+3;
  Goal const goal = slices[si].u.branch.imminent_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(goal.type!=no_goal);
  assert(n==slack_length_help+1);

  empile_for_goal = goal;
  generate_move_reaching_goal(side_at_move);
  empile_for_goal.type = no_goal;

  if (find_solution(si))
    result = slack_length_help+1;

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
