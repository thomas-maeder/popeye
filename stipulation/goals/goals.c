#include "stipulation/goals/goals.h"
#include "stipulation/stipulation.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Determine whether two goals are equal
 * @param goal1 first goal
 * @param goal2 second goal
 * @return true iff goal1 and goal2 are equal
 */
boolean are_goals_equal(Goal goal1, Goal goal2)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal1.type);
  TraceFunctionParam("%u",goal2.type);
  TraceFunctionParamListEnd();

  if (goal1.type==goal2.type)
    result = goal1.type!=goal_target || goal1.target==goal2.target;
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

char const *goal_end_marker[nr_goals] =
{
  "#"
  , "="
  , "=="
  , "z"
  , "+"
  , "x"
  , "%"
  , ""
  , "##"
  , "##!"
  , ""
  , "!="
  , ""
  , ""
  , ""
  , ""
  , ""
  , "dia"
  , "a=>b"
  , "#="
  , "c81"
  , ""
  , "!#"
  , ""
};
