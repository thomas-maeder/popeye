#include "conditions/anticirce/target_square_filter.h"
#include "pystip.h"
#include "stipulation/pipe.h"
#include "pydata.h"
#include "debugging/trace.h"

#include <assert.h>

/* This module provides slice type STAnticirceTargetSquareFilter
 */

/* Allocate a STAnticirceTargetSquareFilter slice.
 * @param target target square to be reached
 * @return index of allocated slice
 */
slice_index alloc_anticirce_target_square_filter_slice(square target)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STAnticirceTargetSquareFilter);
  slices[result].u.goal_handler.goal.type = goal_target;
  slices[result].u.goal_handler.goal.target = target;

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
  boolean result;
  square const target = slices[si].u.goal_handler.goal.target;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = pprise[nbply]!=vide && sq_rebirth_capturing[nbply]!=target;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type anticirce_target_square_filter_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (is_mover_removed_from_target(si))
    result = n+2;
  else
    result = attack(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
