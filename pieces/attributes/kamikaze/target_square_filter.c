#include "pieces/attributes/kamikaze/target_square_filter.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/defense_play.h"
#include "trace.h"

#include <assert.h>

/* This module provides slice type STPiecesKamikazeTargetSquareFilter
 */

/* Allocate a STPiecesKamikazeTargetSquareFilter slice.
 * @param target target square to be reached
 * @return index of allocated slice
 */
slice_index alloc_kamikaze_target_square_filter_slice(square target)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STPiecesKamikazeTargetSquareFilter);
  slices[result].u.goal_reached_tester.goal.type = goal_target;
  slices[result].u.goal_reached_tester.goal.target = target;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Is the piece that has just moved removed from the target square because it is
 * a Kamikaze piece?
 * @param si identifies filter slice
 * @return true iff the piece is removed
 */
static boolean is_mover_removed_from_target(slice_index si)
{
  square const sq_arrival = move_generation_stack[nbcou].arrival;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (e[slices[si].u.goal_reached_tester.goal.target]==vide)
    result = true;
  else if (!TSTFLAG(spec[sq_arrival],Kamikaze))
    result = false;
  else if (sq_rebirth_capturing[nbply]
           ==slices[si].u.goal_reached_tester.goal.target)
    result = false;
  else
    result = true;


  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type kamikaze_target_square_filter_has_solution(slice_index si)
{
  has_solution_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (is_mover_removed_from_target(si))
    result = has_no_solution;
  else
    result = slice_has_solution(next);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type kamikaze_target_square_filter_solve(slice_index si)
{
  has_solution_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (is_mover_removed_from_target(si))
    result = has_no_solution;
  else
    result = slice_solve(next);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
