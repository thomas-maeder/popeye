#include "stipulation/goals/goals.h"
#include "pystip.h"
#include "pypipe.h"
#include "pydata.h"
#include "pyselfcg.h"
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

static void flesh_out_mate_reached_tester(slice_index si,
                                          stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_append(si,alloc_goal_immobile_reached_tester_slice(goal_applies_to_starter));
  pipe_append(si,alloc_selfcheck_guard_solvable_filter());
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
  pipe_append(si,alloc_selfcheck_guard_solvable_filter());

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
  pipe_append(si,alloc_selfcheck_guard_solvable_filter());

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
  pipe_append(si,alloc_selfcheck_guard_solvable_filter());

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

static void flesh_out_with_self_check_guard(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_append(si,alloc_selfcheck_guard_solvable_filter());

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
  { STGoalDoubleStalemateReachedTester, &flesh_out_double_stalemate_reached_tester },
  { STGoalAnyReachedTester,             &flesh_out_with_self_check_guard           },
  { STGoalCaptureReachedTester,         &flesh_out_with_self_check_guard           },
  { STGoalCastlingReachedTester,        &flesh_out_with_self_check_guard           },
  { STGoalCheckReachedTester,           &flesh_out_with_self_check_guard           },
  { STGoalCircuitReachedTester,         &flesh_out_with_self_check_guard           },
  { STGoalEnpassantReachedTester,       &flesh_out_with_self_check_guard           },
  { STGoalExchangeReachedTester,        &flesh_out_with_self_check_guard           },
  { STGoalAToBReachedTester,            &flesh_out_with_self_check_guard           },
  { STGoalProofgameReachedTester,       &flesh_out_with_self_check_guard           },
  { STGoalSteingewinnReachedTester,     &flesh_out_with_self_check_guard           },
  { STGoalTargetReachedTester,          &flesh_out_with_self_check_guard           },
  { STAnticirceCircuitSpecial,          &flesh_out_with_self_check_guard           },
  { STAnticirceExchangeSpecial,         &flesh_out_with_self_check_guard           },
  { STCirceCircuitSpecial,              &flesh_out_with_self_check_guard           },
  { STCirceExchangeSpecial,             &flesh_out_with_self_check_guard           }
};

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
