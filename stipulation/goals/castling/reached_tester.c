#include "stipulation/goals/castling/reached_tester.h"
#include "pystip.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "debugging/trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether a castling goal has just been reached
 */

/* Allocate a system of slices that tests whether castling has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_castling_reached_tester_system(void)
{
  slice_index result;
  slice_index castling_tester;
  Goal const goal = { goal_castling, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  castling_tester = alloc_pipe(STGoalCastlingReachedTester);
  pipe_link(castling_tester,alloc_true_slice());
  result = alloc_goal_reached_tester_slice(goal,castling_tester);

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
stip_length_type goal_castling_reached_tester_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  unsigned char const diff = castling_flag[nbply-1]-castling_flag[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(nbply!=nil_ply);

  /* castling means that both bits of a castling were cleared */
  if (diff==whk_castling || diff==whq_castling
      || diff==blk_castling || diff==blq_castling)
    result = attack(slices[si].next1,n);
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
