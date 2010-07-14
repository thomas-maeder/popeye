#include "stipulation/series_play/move_to_goal.h"
#include "pydata.h"
#include "pyproc.h"
#include "pypipe.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_moves_generation.h"
#include "stipulation/series_play/play.h"
#include "trace.h"
#include "stipulation/branch.h"
#include "stipulation/help_play/move.h"
#include "stipulation/series_play/branch.h"
#include "stipulation/series_play/root.h"
#include "stipulation/series_play/shortcut.h"

#include <assert.h>

/* Allocate a STSeriesMoveToGoal slice.
 * @param goal goal to be reached
 * @return index of allocated slice
 */
slice_index alloc_series_move_to_goal_slice(Goal goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal.type);
  TraceFunctionParamListEnd();

  result = alloc_branch(STSeriesMoveToGoal,
                        slack_length_series+1,slack_length_series+1);
  slices[result].u.branch.imminent_goal = goal;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void series_move_to_goal_insert_root(slice_index si,
                                     stip_structure_traversal *st)
{
  slice_index * const root = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *root = alloc_series_root_slice(slack_length_series+1,slack_length_series+1,
                                  slices[si].prev,no_slice);
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void series_move_to_goal_detect_starter(slice_index si,
                                        stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].starter==no_side)
  {
    slice_index const next = slices[si].u.pipe.next;
    stip_traverse_structure_children(si,st);
    slices[si].starter = (slices[next].starter==no_side
                          ? no_side
                          : advers(slices[next].starter));
  }

  TraceValue("%u\n",slices[si].starter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try solving with all generated moves
 * @param si slice index
 * @return true iff a solution was found
 */
static boolean foreach_move_solve(slice_index si)
{
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const length = slack_length_series;
  stip_length_type result = false;

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && series_solve_in_n(next,length)==length)
      result = true;

    repcoup();
  }

  return result;
}

/* Determine and write the solution(s) in a series stipulation
 * @param si slice index
 * @param n exact number of moves to reach the end state
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type series_move_to_goal_solve_in_n(slice_index si,
                                                stip_length_type n)
{
  stip_length_type result = slack_length_series+2;
  Side const side_at_move = slices[si].starter;
  Goal const goal = slices[si].u.branch.imminent_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n==slack_length_series+1);
  assert(goal.type!=no_goal);

  if (are_prerequisites_for_reaching_goal_met(goal.type,side_at_move))
  {
    empile_for_goal = goal;
    generate_move_reaching_goal(side_at_move);
    empile_for_goal.type = no_goal;

    if (foreach_move_solve(si))
      result = slack_length_series+1;

    finply();
  }

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
  stip_length_type const length = slack_length_series;
  boolean result = false;
  
  while (encore())
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && series_has_solution_in_n(next,length)==length)
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
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type series_move_to_goal_has_solution_in_n(slice_index si,
                                                       stip_length_type n)
{
  Side const side_at_move = slices[si].starter;
  stip_length_type result = slack_length_series+2;
  Goal const goal = slices[si].u.branch.imminent_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n==slack_length_series+1);
  assert(goal.type!=no_goal);

  if (are_prerequisites_for_reaching_goal_met(goal.type,side_at_move))
  {
    empile_for_goal = goal;
    generate_move_reaching_goal(side_at_move);
    empile_for_goal.type = no_goal;

    if (find_solution(si))
      result = slack_length_series+1;

    finply();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
