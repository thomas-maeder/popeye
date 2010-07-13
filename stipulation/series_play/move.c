#include "stipulation/series_play/move.h"
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

/* Allocate a STSeriesMove slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_series_move_slice(stip_length_type length,
                                   stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STSeriesMove,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Produce slices representing set play
 * @param si slice index
 * @param st state of traversal
 */
void series_move_make_setplay_slice(slice_index si, stip_structure_traversal *st)
{
  slice_index * const result = st->param;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const length_h = (length-slack_length_series
                                     +slack_length_help);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *result = alloc_help_move_slice(length_h,length_h);
  pipe_set_successor(*result,slices[si].u.branch.next);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void series_move_insert_root(slice_index si, stip_structure_traversal *st)
{
  slice_index * const root = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    slice_index const prev = slices[si].prev;
    slice_index const root_branch = copy_slice(si);

    *root = alloc_series_root_slice(length,min_length,root_branch,prev);

    shorten_series_pipe(si);
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void series_move_detect_starter(slice_index si, stip_structure_traversal *st)
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

/* Spin off set play
 * @param si slice index
 * @param st state of traversal
 */
void series_move_apply_setplay(slice_index si, stip_structure_traversal *st)
{
  slice_index * const setplay_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *setplay_slice = stip_make_setplay(slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try solving with all generated moves
 * @param si slice index
 * @param n exact number of moves to reach the end state
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
static stip_length_type foreach_move_solve(slice_index si, stip_length_type n)
{
  stip_length_type result = n+1;
  slice_index const next = slices[si].u.pipe.next;

  while (encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && series_solve_in_n(next,n-1)==n-1)
      result = n;

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
stip_length_type series_move_solve_in_n(slice_index si, stip_length_type n)
{
  stip_length_type result = n+1;
  Side const side_at_move = slices[si].starter;
  Goal const goal = slices[si].u.branch.imminent_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_series);

  if (n==slack_length_series+1 && goal.type!=no_goal)
  {
    if (are_prerequisites_for_reaching_goal_met(goal.type,side_at_move))
    {
      empile_for_goal = goal;
      generate_move_reaching_goal(side_at_move);
      empile_for_goal.type = no_goal;
      result = foreach_move_solve(si,n);
      finply();
    }
  }
  else
  {
    move_generation_mode = move_generation_not_optimized;
    TraceValue("->%u\n",move_generation_mode);
    genmove(side_at_move);
    result = foreach_move_solve(si,n);
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
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
static stip_length_type find_solution(slice_index si, stip_length_type n)
{
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type result = n+1;
  
  while (encore())
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && series_has_solution_in_n(next,n-1)==n-1)
    {
      result = n;
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
stip_length_type series_move_has_solution_in_n(slice_index si,
                                               stip_length_type n)
{
  Side const side_at_move = slices[si].starter;
  stip_length_type result;
  Goal const goal = slices[si].u.branch.imminent_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (n==slack_length_series+1 && goal.type!=no_goal)
  {
    if (are_prerequisites_for_reaching_goal_met(goal.type,side_at_move))
    {
      empile_for_goal = goal;
      generate_move_reaching_goal(side_at_move);
      empile_for_goal.type = no_goal;
      result = find_solution(si,n);
      finply();
    }
    else
      result = n+1;
  }
  else
  {
    move_generation_mode = move_generation_not_optimized;
    TraceValue("->%u\n",move_generation_mode);
    genmove(side_at_move);
    result = find_solution(si,n);
    finply();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
