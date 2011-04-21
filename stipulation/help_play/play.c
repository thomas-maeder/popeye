#include "stipulation/help_play/play.h"
#include "pyhash.h"
#include "pyreflxg.h"
#include "pykeepmt.h"
#include "pyselfcg.h"
#include "pymovenb.h"
#include "pyint.h"
#include "stipulation/battle_play/attack_adapter.h"
#include "stipulation/help_play/root.h"
#include "stipulation/help_play/find_shortest.h"
#include "stipulation/help_play/move.h"
#include "stipulation/help_play/move_generator.h"
#include "stipulation/help_play/end_of_branch.h"
#include "stipulation/help_play/fork.h"
#include "stipulation/goals/countermate/filter.h"
#include "stipulation/goals/doublemate/filter.h"
#include "stipulation/goals/prerequisite_optimiser.h"
#include "optimisations/goals/enpassant/filter.h"
#include "optimisations/goals/castling/filter.h"
#include "optimisations/intelligent/help_filter.h"
#include "options/maxtime.h"
#include "options/maxsolutions/guard.h"
#include "options/stoponshortsolutions/filter.h"
#include "stipulation/fork_on_remaining.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_move_generator.h"
#include "trace.h"

#include <assert.h>

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type help(slice_index si, stip_length_type n)
{
  stip_length_type result = n+2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STHelpRoot:
      result = help_root_help(si,n);
      break;

    case STHelpFindShortest:
      result = help_find_shortest_help(si,n);
      break;

    case STForkOnRemaining:
      result = fork_on_remaining_help(si,n);
      break;

    case STHelpMoveGenerator:
      result = help_move_generator_help(si,n);
      break;

    case STOrthodoxMatingMoveGenerator:
      result = orthodox_mating_move_generator_help(si,n);
      break;

    case STHelpMove:
      result = help_move_help(si,n);
      break;

    case STAttackAdapter:
      result = attack_adapter_help(si,n);
      break;

    case STEndOfHelpBranch:
      result = end_of_help_branch_help(si,n);
      break;

    case STHelpFork:
      result = help_fork_help(si,n);
      break;

    case STHelpHashed:
      result = help_hashed_help(si,n);
      break;

    case STReflexHelpFilter:
      result = reflex_help_filter_help(si,n);
      break;

    case STKeepMatingFilter:
      result = keepmating_filter_help(si,n);
      break;

    case STIntelligentHelpFilter:
      result = intelligent_help_filter_help(si,n);
      break;

    case STGoalReachableGuardFilter:
      result = goalreachable_guard_help(si,n);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_help(si,n);
      break;

    case STRestartGuard:
      result = restart_guard_help(si,n);
      break;

    case STMaxTimeGuard:
      result = maxtime_guard_help(si,n);
      break;

    case STMaxSolutionsGuard:
      result = maxsolutions_guard_help(si,n);
      break;

    case STStopOnShortSolutionsFilter:
      result = stoponshortsolutions_help(si,n);
      break;

    case STCounterMateFilter:
      result = countermate_filter_help(si,n);
      break;

    case STDoubleMateFilter:
      result = doublemate_filter_help(si,n);
      break;

    case STEnPassantFilter:
      result = enpassant_filter_help(si,n);
      break;

    case STCastlingFilter:
      result = castling_filter_help(si,n);
      break;

    case STPrerequisiteOptimiser:
      result = goal_prerequisite_optimiser_help(si,n);
      break;

    default:
      assert(n=slack_length_help);
      switch (slice_solve(si))
      {
        case opponent_self_check:
          result = slack_length_help+4;
          break;

        case has_no_solution:
          result = slack_length_help+2;
          break;

        case has_solution:
          result = slack_length_help;
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
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type can_help(slice_index si, stip_length_type n)
{
  stip_length_type result = n+2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STHelpMove:
      result = help_move_can_help(si,n);
      break;

    case STForkOnRemaining:
      result = fork_on_remaining_can_help(si,n);
      break;

    case STHelpMoveGenerator:
      result = help_move_generator_can_help(si,n);
      break;

    case STOrthodoxMatingMoveGenerator:
      result = orthodox_mating_move_generator_can_help(si,n);
      break;

    case STHelpRoot:
      result = help_root_can_help(si,n);
      break;

    case STHelpFindShortest:
      result = help_find_shortest_can_help(si,n);
      break;

    case STEndOfHelpBranch:
      result = end_of_help_branch_can_help(si,n);
      break;

    case STHelpFork:
      result = help_fork_can_help(si,n);
      break;

    case STHelpHashed:
      result = help_hashed_can_help(si,n);
      break;

    case STReflexHelpFilter:
      result = reflex_help_filter_can_help(si,n);
      break;

    case STKeepMatingFilter:
      result = keepmating_filter_can_help(si,n);
      break;

    case STGoalReachableGuardFilter:
      result = goalreachable_guard_can_help(si,n);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_can_help(si,n);
      break;

    case STMaxTimeGuard:
      result = maxtime_guard_can_help(si,n);
      break;

    case STMaxSolutionsGuard:
      result = maxsolutions_guard_can_help(si,n);
      break;

    case STStopOnShortSolutionsFilter:
      result = stoponshortsolutions_can_help(si,n);
      break;

    case STCounterMateFilter:
      result = countermate_filter_can_help(si,n);
      break;

    case STDoubleMateFilter:
      result = doublemate_filter_can_help(si,n);
      break;

    case STEnPassantFilter:
      result = enpassant_filter_can_help(si,n);
      break;

    case STCastlingFilter:
      result = castling_filter_can_help(si,n);
      break;

    case STPrerequisiteOptimiser:
      result = goal_prerequisite_optimiser_can_help(si,n);
      break;

    default:
      assert(n=slack_length_help);
      switch (slice_has_solution(si))
      {
        case opponent_self_check:
          result = slack_length_help+4;
          break;

        case has_no_solution:
          result = slack_length_help+2;
          break;

        case has_solution:
          result = slack_length_help;
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
