#include "stipulation/goals/goals.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/slice.h"
#include "stipulation/boolean/false.h"
#include "debugging/trace.h"

#include <assert.h>

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
  , " c81"
  , ""
};

/* Order in which the slice types dealing with goals appear
 */
static slice_index const goal_slice_rank_order[] =
{
  STExclusiveChessGoalReachingMoveCounterSelfCheckGuard,
  STNot,
  STAmuMateFilter,
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
  STGoalChess81ReachedTester,
  STGoalKingCaptureReachedTester,
  STPiecesParalysingMateFilter,
  STPiecesParalysingMateFilterTester,
  STPiecesParalysingStalemateSpecial,
  STGoalCheckReachedTester,
  STSelfCheckGuard,
  STMummerDeadend,
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
static unsigned int get_goal_slice_rank(slice_type type)
{
  unsigned int result = no_goal_slice_type;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type,"");
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
    slice_type const prototype_type = slices[prototypes[0]].type;
    unsigned int prototype_rank = get_goal_slice_rank(prototype_type);

    if (prototype_rank==no_goal_slice_type)
    {
      if (nr_prototypes>1)
        goal_branch_insert_slices_nested(si,prototypes+1,nr_prototypes-1);
    }
    else
      do
      {
        slice_index const next = slices[si].next1;
        if (slices[next].type==STProxy)
          si = next;
        else if (slices[next].type==STOr
                 || slices[next].type==STAnd)
        {
          goal_branch_insert_slices_nested(slices[next].next1,
                                           prototypes,nr_prototypes);
          goal_branch_insert_slices_nested(slices[next].next2,
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
