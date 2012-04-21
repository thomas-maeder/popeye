#include "stipulation/goals/doublemate/filter.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/conditional_pipe.h"
#include "stipulation/goals/prerequisite_guards.h"
#include "stipulation/goals/immobile/reached_tester.h"
#include "debugging/trace.h"

#include <assert.h>

/* Allocate a STDoubleMateFilter slice.
 * @return index of allocated slice
 */
slice_index alloc_doublemate_filter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_conditional_pipe(STDoubleMateFilter,
                                  alloc_goal_immobile_reached_tester_system());

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
stip_length_type doublemate_filter_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (attack(slices[si].next2,length_unspecified)==has_no_solution)
    SETFLAG(goal_preprequisites_met[nbply],goal_doublemate);
  result = attack(slices[si].next1,n);
  CLRFLAG(goal_preprequisites_met[nbply],goal_doublemate);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
