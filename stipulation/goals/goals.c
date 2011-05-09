#include "stipulation/goals/goals.h"
#include "pystip.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/boolean/false.h"
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
  { STGoalDoubleStalemateReachedTester, &flesh_out_double_stalemate_reached_tester }
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

/* Order in which the slice types dealing with goals appear
 */
static slice_index const goal_slice_rank_order[] =
{
  STNot,
  STAmuMateFilter,
  STUltraschachzwangGoalFilter,
  STGoalMateReachedTester,
  STGoalStalemateReachedTester,
  STGoalDoubleStalemateReachedTester,
  STAnticirceTargetSquareFilter,
  STGoalTargetReachedTester,
  STGoalCaptureReachedTester,
  STCirceSteingewinnFilter,
  STGoalSteingewinnReachedTester,
  STGoalEnpassantReachedTester,
  STGoalDoubleMateReachedTester,
  STGoalCounterMateReachedTester,
  STGoalCastlingReachedTester,
  STGoalAutoStalemateReachedTester,
  STGoalCircuitReachedTester,
  STAnticirceExchangeFilter,
  STGoalExchangeReachedTester,
  STAnticirceCircuitSpecial,
  STCirceCircuitSpecial,
  STGoalCircuitByRebirthReachedTester,
  STCirceExchangeSpecial,
  STAnticirceExchangeSpecial,
  STGoalExchangeByRebirthReachedTester,
  STGoalAnyReachedTester,
  STGoalProofgameReachedTester,
  STGoalAToBReachedTester,
  STGoalMateOrStalemateReachedTester,
  STGoalCheckReachedTester,
  STSelfCheckGuard,
  STGoalNotCheckReachedTester,
  STGoalImmobileReachedTester,
  STTrue
};

enum
{
  nr_goal_slice_rank_order_elmts = (sizeof goal_slice_rank_order
                                    / sizeof goal_slice_rank_order[0]),
  no_goal_slice_type = INT_MAX
};

/* Determine the rank of a slice type
 * @param type defense slice type
 * @return rank of type; nr_defense_slice_rank_order_elmts if the rank can't
 *         be determined
 */
static unsigned int get_goal_slice_rank(SliceType type)
{
  unsigned int result = no_goal_slice_type;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceEnumerator(SliceType,type,"");
  TraceFunctionParamListEnd();

  for (i = 0; i!=nr_goal_slice_rank_order_elmts; ++i)
    if (goal_slice_rank_order[i]==type)
    {
      result = i;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert slices into a goal branch.
 * The inserted slices are copies of the elements of prototypes).
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
static void goal_branch_insert_slices_nested(slice_index si,
                                             slice_index const prototypes[],
                                             unsigned int nr_prototypes)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  {
    SliceType const prototype_type = slices[prototypes[0]].type;
    unsigned int prototype_rank = get_goal_slice_rank(prototype_type);

    if (prototype_rank==no_goal_slice_type)
    {
      if (nr_prototypes>1)
        goal_branch_insert_slices_nested(si,prototypes+1,nr_prototypes-1);
    }
    else
      do
      {
        slice_index const next = slices[si].u.pipe.next;
        if (slices[next].type==STProxy)
          si = next;
        else if (slices[next].type==STOr
                 || slices[next].type==STAnd)
        {
          goal_branch_insert_slices_nested(slices[next].u.binary.op1,
                                           prototypes,nr_prototypes);
          goal_branch_insert_slices_nested(slices[next].u.binary.op2,
                                           prototypes,nr_prototypes);
          break;
        }
        else
        {
          unsigned int const rank_next = get_goal_slice_rank(slices[next].type);
          if (rank_next==no_goal_slice_type)
            break;
          else if (rank_next>prototype_rank)
          {
            slice_index const copy = copy_slice(prototypes[0]);
            pipe_append(si,copy);
            if (nr_prototypes>1)
              goal_branch_insert_slices_nested(copy,prototypes+1,nr_prototypes-1);
            break;
          }
          else
            si = next;
        }
      } while (prototype_type!=slices[si].type);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert slices into a goal branch.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by goal_branch_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void goal_branch_insert_slices(slice_index si,
                               slice_index const prototypes[],
                               unsigned int nr_prototypes)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  goal_branch_insert_slices_nested(si,prototypes,nr_prototypes);

  for (i = 0; i!=nr_prototypes; ++i)
    dealloc_slice(prototypes[i]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remove_unsatisfiable_constraint_goal(slice_index si,
                                                 stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_link(slices[si].prev,alloc_false_slice());
  dealloc_slices(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
static structure_traversers_visitors unsatisfiable_goal_checker_removers[] =
{
  { STGoalCaptureReachedTester,           &remove_unsatisfiable_constraint_goal },
  { STGoalCastlingReachedTester,          &remove_unsatisfiable_constraint_goal },
  { STGoalCircuitReachedTester,           &remove_unsatisfiable_constraint_goal },
  { STGoalCircuitByRebirthReachedTester,  &remove_unsatisfiable_constraint_goal },
  { STGoalEnpassantReachedTester,         &remove_unsatisfiable_constraint_goal },
  { STGoalExchangeReachedTester,          &remove_unsatisfiable_constraint_goal },
  { STGoalExchangeByRebirthReachedTester, &remove_unsatisfiable_constraint_goal },
  { STGoalSteingewinnReachedTester,       &remove_unsatisfiable_constraint_goal },
  { STGoalTargetReachedTester,            &remove_unsatisfiable_constraint_goal },
  { STAttackAdapter,                      &stip_structure_visitor_noop          },
  { STDefenseAdapter,                     &stip_structure_visitor_noop          },
  { STHelpAdapter,                        &stip_structure_visitor_noop          },
  { STSeriesAdapter,                      &stip_structure_visitor_noop          }
};

enum
{
  nr_unsatisfiable_goal_checker_removers =
    (sizeof unsatisfiable_goal_checker_removers
     / sizeof unsatisfiable_goal_checker_removers[0])
};

/* Remove goal checker slices that we know can't possibly be met
 * @param si identifies entry slice to stipulation
 */
void stip_remove_unsatisfiable_goals(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    unsatisfiable_goal_checker_removers,
                                    nr_unsatisfiable_goal_checker_removers);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();

}
