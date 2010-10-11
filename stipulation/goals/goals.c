#include "stipulation/goals/goals.h"
#include "pystip.h"
#include "pydata.h"
#include "trace.h"

#include <assert.h>

#define ENUMERATION_TYPENAME goal_checker_result_type
#define ENUMERATORS                             \
  ENUMERATOR(goal_not_reached),                 \
    ENUMERATOR(goal_not_reached_selfcheck),     \
    ENUMERATOR(goal_reached)

#define ENUMERATION_MAKESTRINGS

#include "pyenum.h"

goal_checker_result_type goal_checker_mate(Side just_moved)
{
  Side const ad = advers(just_moved);
  if (echecc(nbply,ad))
  {
    if (echecc(nbply,just_moved))
      return goal_not_reached_selfcheck;
    else if (immobile(ad))
      return goal_reached;
    else
      return goal_not_reached;
  }
  else
    return goal_not_reached;
}

char const *goal_end_marker[nr_goals] =
{
  " #"
  , " ="
  , " =="
  , " z"
  , " +"
  , " x"
  , " %"
  , ""
  , " ##"
  , " ##!"
  , ""
  , " !="
  , ""
  , ""
  , ""
  , ""
  , ""
  , " dia"
  , " a=>b"
  , " #="
};

/* Extract the goal from a STGoal*ReachedTester slice
 * @param si identifies STGoal*ReachedTester slice
 * @return goal that si tests for
 */
Goal extractGoalFromTester(slice_index si)
{
  Goal result = { (goal_type)(slices[si].type-first_goal_tester_slice_type),
                  initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(slices[si].type>=first_goal_tester_slice_type);
  assert(slices[si].type<=last_goal_tester_slice_type);

  if (result.type==goal_target)
    result.target = slices[si].u.goal_target_reached_tester.target;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result.type);
  TraceFunctionResultEnd();
  return result;
}
