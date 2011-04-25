#include "stipulation/goals/goals.h"
#include "pystip.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/goals/immobile/reached_tester.h"
#include "stipulation/goals/check/reached_tester.h"
#include "stipulation/goals/notcheck/reached_tester.h"
#include "trace.h"

#include <assert.h>

#define ENUMERATION_TYPENAME goal_checker_result_type
#define ENUMERATORS                             \
  ENUMERATOR(goal_not_reached),                 \
    ENUMERATOR(goal_not_reached_selfcheck),     \
    ENUMERATOR(goal_reached)

#define ENUMERATION_MAKESTRINGS

#include "pyenum.h"

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

static void flesh_out_mate_reached_tester(slice_index si,
                                          stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_append(si,alloc_goal_immobile_reached_tester_slice(goal_applies_to_starter));
  pipe_append(si,alloc_goal_check_reached_tester_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_stalemate_reached_tester(slice_index si,
                                               stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_append(si,alloc_goal_immobile_reached_tester_slice(goal_applies_to_starter));
  pipe_append(si,alloc_goal_notcheck_reached_tester_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_auto_stalemate_tester(slice_index si,
                                            stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_append(si,alloc_goal_immobile_reached_tester_slice(goal_applies_to_adversary));
  pipe_append(si,alloc_goal_notcheck_reached_tester_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_double_stalemate_reached_tester(slice_index si,
                                                      stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  pipe_append(si,alloc_goal_immobile_reached_tester_slice(goal_applies_to_adversary));
  pipe_append(si,alloc_goal_immobile_reached_tester_slice(goal_applies_to_starter));
  pipe_append(si,alloc_goal_notcheck_reached_tester_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_double_mate_reached_tester(slice_index si,
                                                 stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  pipe_append(si,alloc_goal_immobile_reached_tester_slice(goal_applies_to_adversary));
  pipe_append(si,alloc_goal_immobile_reached_tester_slice(goal_applies_to_starter));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors flesh_outers[] =
{
  { STGoalMateReachedTester,            &flesh_out_mate_reached_tester             },
  { STGoalStalemateReachedTester,       &flesh_out_stalemate_reached_tester        },
  { STGoalAutoStalemateReachedTester,   &flesh_out_auto_stalemate_tester           },
  { STGoalDoubleMateReachedTester,      &flesh_out_double_mate_reached_tester      },
  { STGoalCounterMateReachedTester,     &flesh_out_double_mate_reached_tester      },
  { STGoalDoubleStalemateReachedTester, &flesh_out_double_stalemate_reached_tester }};

enum
{
  nr_flesh_outers = (sizeof flesh_outers / sizeof flesh_outers[0])
};

/* Allow goal tester slices to surround themselves with auxiliary slices
 * @param si identifies entry slice to stipulation
 */
void stip_flesh_out_goal_testers(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,flesh_outers,nr_flesh_outers);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
