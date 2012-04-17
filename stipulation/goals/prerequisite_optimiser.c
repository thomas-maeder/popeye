#include "stipulation/goals/prerequisite_optimiser.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/branch.h"
#include "stipulation/goals/prerequisite_guards.h"
#include "solving/avoid_unsolvable.h"
#include "debugging/trace.h"

#include <assert.h>

/* Allocate a STPrerequisiteOptimiser defender slice.
 * @return index of allocated slice
 */
slice_index alloc_goal_prerequisite_optimiser_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STPrerequisiteOptimiser);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
goal_prerequisite_optimiser_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const save_max_unsolvable = max_unsolvable;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(max_unsolvable<=slack_length);

  if (goal_preprequisites_met[nbply]==0)
  {
    max_unsolvable = slack_length+1;
    TraceValue("->%u\n",max_unsolvable);
  }

  result = attack(next,n);

  max_unsolvable = save_max_unsolvable;
  TraceValue("->%u\n",max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type
goal_prerequisite_optimiser_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const save_max_unsolvable = max_unsolvable;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(max_unsolvable<=slack_length);

  if (goal_preprequisites_met[nbply]==0)
  {
    max_unsolvable = slack_length+2;
    TraceValue("->%u\n",max_unsolvable);
  }

  result = defend(next,n);

  max_unsolvable = save_max_unsolvable;
  TraceValue("->%u\n",max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
