#include "stipulation/series_play/play.h"
#include "pyhash.h"
#include "pykeepmt.h"
#include "pyselfcg.h"
#include "pymovenb.h"
#include "pyint.h"
#include "stipulation/constraint.h"
#include "stipulation/dead_end.h"
#include "stipulation/end_of_branch.h"
#include "stipulation/end_of_branch_goal.h"
#include "stipulation/move.h"
#include "stipulation/dummy_move.h"
#include "stipulation/goals/countermate/filter.h"
#include "stipulation/goals/doublemate/filter.h"
#include "stipulation/goals/prerequisite_optimiser.h"
#include "solving/fork_on_remaining.h"
#include "solving/find_by_increasing_length.h"
#include "solving/find_shortest.h"
#include "solving/move_generator.h"
#include "optimisations/goals/castling/filter.h"
#include "optimisations/intelligent/series_filter.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_move_generator.h"
#include "options/maxtime.h"
#include "options/maxsolutions/guard.h"
#include "options/stoponshortsolutions/filter.h"
#include "trace.h"

#include <assert.h>


/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type series(slice_index si, stip_length_type n)
{
  stip_length_type result = n+2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(slice_type,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STFindShortest:
      result = find_shortest_series(si,n);
      break;

    case STFindByIncreasingLength:
      result = find_by_increasing_length_series(si,n);
      break;

    case STMove:
      result = move_series(si,n);
      break;

    case STEndOfBranch:
    case STEndOfBranchGoalImmobile:
      result = end_of_branch_series(si,n);
      break;

    case STEndOfBranchGoal:
    case STEndOfBranchForced:
      result = end_of_branch_goal_series(si,n);
      break;

    case STDeadEnd:
    case STDeadEndGoal:
      result = dead_end_series(si,n);
      break;

    case STSeriesHashed:
      result = series_hashed_series(si,n);
      break;

    case STDummyMove:
      result = dummy_move_series(si,n);
      break;

    case STConstraint:
      result = constraint_series(si,n);
      break;

    case STKeepMatingFilter:
      result = keepmating_filter_series(si,n);
      break;

    case STIntelligentSeriesFilter:
      result = intelligent_series_filter_series(si,n);
      break;

    case STGoalReachableGuardFilter:
      result = goalreachable_guard_series(si,n);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_series(si,n);
      break;

    case STRestartGuard:
      result = restart_guard_series(si,n);
      break;

    case STMaxTimeGuard:
      result = maxtime_guard_series(si,n);
      break;

    case STMaxSolutionsGuard:
      result = maxsolutions_guard_series(si,n);
      break;

    case STStopOnShortSolutionsFilter:
      result = stoponshortsolutions_series(si,n);
      break;

    case STCounterMateFilter:
      result = countermate_filter_series(si,n);
      break;

    case STDoubleMateFilter:
      result = doublemate_filter_series(si,n);
      break;

    case STCastlingFilter:
      result = castling_filter_series(si,n);
      break;

    case STPrerequisiteOptimiser:
      result = goal_prerequisite_optimiser_series(si,n);
      break;

    case STMoveGenerator:
      result = move_generator_series(si,n);
      break;

    case STOrthodoxMatingMoveGenerator:
      result = orthodox_mating_move_generator_series(si,n);
      break;

    case STForkOnRemaining:
      result = fork_on_remaining_series(si,n);
      break;

    default:
      assert(n=slack_length_series);
      switch (slice_solve(si))
      {
        case opponent_self_check:
          result = slack_length_series+2;
          break;

        case has_no_solution:
          result = slack_length_series+1;
          break;

        case has_solution:
          result = slack_length_series;
          break;

        default:
          assert(0);
          break;
      }
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type has_series(slice_index si, stip_length_type n)
{
  stip_length_type result = n+2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(slice_type,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STFindShortest:
      result = find_shortest_has_series(si,n);
      break;

    case STMove:
      result = move_has_series(si,n);
      break;

    case STEndOfBranch:
    case STEndOfBranchGoalImmobile:
      result = end_of_branch_has_series(si,n);
      break;

    case STEndOfBranchGoal:
    case STEndOfBranchForced:
      result = end_of_branch_goal_has_series(si,n);
      break;

    case STDeadEnd:
    case STDeadEndGoal:
      result = dead_end_has_series(si,n);
      break;

    case STSeriesHashed:
      result = series_hashed_has_series(si,n);
      break;

    case STConstraint:
      result = constraint_has_series(si,n);
      break;

    case STKeepMatingFilter:
      result = keepmating_filter_has_series(si,n);
      break;

    case STGoalReachableGuardFilter:
      result = goalreachable_guard_has_series(si,n);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_has_series(si,n);
      break;

    case STDummyMove:
      result = dummy_move_has_series(si,n);
      break;

    case STMaxTimeGuard:
      result = maxtime_guard_has_series(si,n);
      break;

    case STMaxSolutionsGuard:
      result = maxsolutions_guard_has_series(si,n);
      break;

    case STStopOnShortSolutionsFilter:
      result = stoponshortsolutions_has_series(si,n);
      break;

    case STCounterMateFilter:
      result = countermate_filter_has_series(si,n);
      break;

    case STDoubleMateFilter:
      result = doublemate_filter_has_series(si,n);
      break;

    case STMoveGenerator:
      result = move_generator_has_series(si,n);
      break;

    case STOrthodoxMatingMoveGenerator:
      result = orthodox_mating_move_generator_has_series(si,n);
      break;

    case STForkOnRemaining:
      result = fork_on_remaining_has_series(si,n);
      break;

    case STCastlingFilter:
      result = castling_filter_has_series(si,n);
      break;

    case STPrerequisiteOptimiser:
      result = goal_prerequisite_optimiser_has_series(si,n);
      break;

    default:
      assert(n=slack_length_series);
      switch (slice_has_solution(si))
      {
        case opponent_self_check:
          result = slack_length_series+2;
          break;

        case has_no_solution:
          result = slack_length_series+1;
          break;

        case has_solution:
          result = slack_length_series;
          break;

        default:
          assert(0);
          break;
      }
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
