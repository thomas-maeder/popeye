#include "stipulation/goals/slice_insertion.h"
#include "stipulation/stipulation.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/pipe.h"
#include "stipulation/slice.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

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
  STCirceCircuitSpecial,
  STGoalCircuitByRebirthReachedTester,
  STCirceExchangeSpecial,
  STGoalExchangeByRebirthReachedTester,
  STGoalAnyReachedTester,
  STGoalProofgameReachedTester,
  STGoalAToBReachedTester,
  STGoalMateOrStalemateReachedTester,
  STGoalChess81ReachedTester,
  STGoalKissReachedTester,
  STGoalKingCaptureReachedTester,
  STPiecesParalysingMateFilter,
  STPiecesParalysingMateFilterTester,
  STPiecesParalysingStalemateSpecial,
  STGoalCheckReachedTester,
  STSelfCheckGuard,
  STOhneschachStopIfCheckAndNotMate,
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
  TraceEnumerator(slice_type,type);
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
    slice_type const prototype_type = SLICE_TYPE(prototypes[0]);
    unsigned int prototype_rank = get_goal_slice_rank(prototype_type);

    if (prototype_rank==no_goal_slice_type)
    {
      if (nr_prototypes>1)
        goal_branch_insert_slices_nested(si,prototypes+1,nr_prototypes-1);
    }
    else
      do
      {
        slice_index const next = SLICE_NEXT1(si);
        if (SLICE_TYPE(next)==STProxy)
          si = next;
        else if (SLICE_TYPE(next)==STOr
                 || SLICE_TYPE(next)==STAnd)
        {
          goal_branch_insert_slices_nested(SLICE_NEXT1(next),
                                           prototypes,nr_prototypes);
          goal_branch_insert_slices_nested(SLICE_NEXT2(next),
                                           prototypes,nr_prototypes);
          break;
        }
        else
        {
          unsigned int const rank_next = get_goal_slice_rank(SLICE_TYPE(next));
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
      } while (prototype_type!=SLICE_TYPE(si));
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
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  goal_branch_insert_slices_nested(si,prototypes,nr_prototypes);
  deallocate_slice_insertion_prototypes(prototypes,nr_prototypes);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
