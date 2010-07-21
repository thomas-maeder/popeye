#include "stipulation/battle_play/defense_play.h"
#include "pydirctg.h"
#include "pyflight.h"
#include "pykeepmt.h"
#include "pymovenb.h"
#include "pynontrv.h"
#include "pyreflxg.h"
#include "pyselfcg.h"
#include "pyselfgd.h"
#include "pythreat.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/postkeyplay.h"
#include "stipulation/battle_play/continuation.h"
#include "stipulation/battle_play/try.h"
#include "stipulation/battle_play/threat.h"
#include "stipulation/battle_play/defense_move.h"
#include "stipulation/battle_play/defense_move_against_goal.h"
#include "stipulation/battle_play/defense_fork.h"
#include "stipulation/help_play/root.h"
#include "stipulation/help_play/play.h"
#include "optimisations/maxsolutions/root_defender_filter.h"
#include "optimisations/maxtime/root_defender_filter.h"
#include "optimisations/maxtime/defender_filter.h"
#include "output/plaintext/tree/check_detector.h"
#include "output/plaintext/tree/battle_play_solution_writer.h"
#include "output/plaintext/tree/end_of_solution_writer.h"
#include "trace.h"

#include <assert.h>

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - more refutations found than acceptable
 */
stip_length_type defense_defend_in_n(slice_index si,
                                     stip_length_type n,
                                     stip_length_type n_min,
                                     stip_length_type n_max_unsolvable)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STBattlePlaySolver:
      result = battle_play_solver_defend_in_n(si,n,n_min,n_max_unsolvable);
      break;

    case STBattlePlaySolutionWriter:
      result = battle_play_solution_writer_defend_in_n(si,
                                                       n,n_min,
                                                       n_max_unsolvable);
      break;

    case STContinuationSolver:
      result = continuation_solver_defend_in_n(si,n,n_min,n_max_unsolvable);
      break;

    case STContinuationWriter:
      result = continuation_writer_defend_in_n(si,n,n_min,n_max_unsolvable);
      break;

    case STThreatSolver:
      result = threat_solver_defend_in_n(si,n,n_min,n_max_unsolvable);
      break;

    case STPostKeyPlaySuppressor:
      result = postkeyplay_suppressor_defend_in_n(si,n,n_min,n_max_unsolvable);
      break;

    case STThreatCollector:
      result = threat_collector_defend_in_n(si,n,n_min,n_max_unsolvable);
      break;

    case STDefenseMove:
    case STDefenseMoveAgainstGoal:
      result = defense_move_defend_in_n(si,n,n_min,n_max_unsolvable);
      break;

    case STDefenseFork:
      result = defense_fork_defend_in_n(si,n,n_min,n_max_unsolvable);
      break;

    case STDirectDefenderFilter:
      result = direct_defender_filter_defend_in_n(si,n,n_min,n_max_unsolvable);
      break;

    case STSelfCheckGuardDefenderFilter:
      result = selfcheck_guard_defend_in_n(si,n,n_min,n_max_unsolvable);
      break;

    case STReflexDefenderFilter:
      result = reflex_defender_filter_defend_in_n(si,n,n_min,n_max_unsolvable);
      break;

    case STKeepMatingGuardDefenderFilter:
      result = keepmating_guard_defend_in_n(si,n,n_min,n_max_unsolvable);
      break;

    case STMaxFlightsquares:
      result = maxflight_guard_defend_in_n(si,n,n_min,n_max_unsolvable);
      break;

    case STMaxThreatLength:
      result = maxthreatlength_guard_defend_in_n(si,n,n_min,n_max_unsolvable);
      break;

    case STMaxNrNonTrivial:
      result = max_nr_nontrivial_guard_defend_in_n(si,n,n_min,n_max_unsolvable);
      break;

    case STMaxTimeDefenderFilter:
      result = maxtime_defender_filter_defend_in_n(si,n,n_min,n_max_unsolvable);
      break;

    case STMaxSolutionsRootDefenderFilter:
      result = maxsolutions_root_defender_filter_defend_in_n(si,
                                                             n,n_min,
                                                             n_max_unsolvable);
      break;

    case STRestartGuardRootDefenderFilter:
      result = restart_guard_defend_in_n(si,n,n_min,n_max_unsolvable);
      break;

    case STMaxTimeRootDefenderFilter:
      result = maxtime_root_defender_filter_defend_in_n(si,
                                                        n,n_min,
                                                        n_max_unsolvable);
      break;

    case STGoalReachedTester:
    case STQuodlibet:
      assert(n==slack_length_battle);
      switch (slice_solve(si))
      {
        case opponent_self_check:
          result = n+4;
          break;

        case has_solution:
          result = n;
          break;

        case has_no_solution:
          result = n+2;
          break;

        default:
          assert(0);
          result = n+4;
          break;
      }
      break;

    case STOutputPlaintextTreeCheckDetectorDefenderFilter:
      result = output_plaintext_tree_check_detector_defend_in_n(si,
                                                                n,n_min,
                                                                n_max_unsolvable);
      break;

    case STEndOfSolutionWriter:
      result = end_of_solution_writer_defend_in_n(si,
                                                  n,n_min,
                                                  n_max_unsolvable);
      break;

    default:
      assert(0);
      result = n+4;
      break;
  }

  assert(result>n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @param max_nr_refutations how many refutations should we look for
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=max_nr_refutations refutations found
 *         n+4 refuted - >max_nr_refutations refutations found
 */
stip_length_type defense_can_defend_in_n(slice_index si,
                                         stip_length_type n,
                                         stip_length_type n_max_unsolvable,
                                         unsigned int max_nr_refutations)
{
  stip_length_type result = n+4;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParam("%u",max_nr_refutations);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STBattlePlaySolver:
      result = battle_play_solver_can_defend_in_n(si,
                                                  n,n_max_unsolvable,
                                                  max_nr_refutations);
      break;

    case STBattlePlaySolutionWriter:
      result = battle_play_solution_writer_can_defend_in_n(si,
                                                           n,n_max_unsolvable,
                                                           max_nr_refutations);
      break;

    case STPostKeyPlaySuppressor:
      result = postkeyplay_suppressor_can_defend_in_n(si,
                                                      n,n_max_unsolvable,
                                                      max_nr_refutations);
      break;

    case STContinuationSolver:
      result = continuation_solver_can_defend_in_n(si,
                                                   n,n_max_unsolvable,
                                                   max_nr_refutations);
      break;

    case STContinuationWriter:
      result = continuation_writer_can_defend_in_n(si,
                                                   n,n_max_unsolvable,
                                                   max_nr_refutations);
      break;

    case STThreatSolver:
      result = threat_solver_can_defend_in_n(si,
                                             n,n_max_unsolvable,
                                             max_nr_refutations);
      break;


    case STThreatCollector:
      result = threat_collector_can_defend_in_n(si,
                                                n,n_max_unsolvable,
                                                max_nr_refutations);
      break;

    case STDefenseMove:
      result = defense_move_can_defend_in_n(si,
                                            n,n_max_unsolvable,
                                            max_nr_refutations);
      break;

    case STDefenseMoveAgainstGoal:
      result = defense_move_against_goal_can_defend_in_n(si,
                                                         n,n_max_unsolvable,
                                                         max_nr_refutations);
      break;

    case STDefenseFork:
      result = defense_fork_can_defend_in_n(si,
                                            n,n_max_unsolvable,
                                            max_nr_refutations);
      break;

    case STDirectDefenderFilter:
      result = direct_defender_filter_can_defend_in_n(si,
                                                      n,n_max_unsolvable,
                                                      max_nr_refutations);
      break;

    case STReflexDefenderFilter:
      result = reflex_defender_filter_can_defend_in_n(si,
                                                      n,n_max_unsolvable,
                                                      max_nr_refutations);
      break;

    case STSelfCheckGuardDefenderFilter:
      result = selfcheck_guard_can_defend_in_n(si,
                                               n,n_max_unsolvable,
                                               max_nr_refutations);
      break;

    case STKeepMatingGuardDefenderFilter:
      result = keepmating_guard_can_defend_in_n(si,
                                                n,n_max_unsolvable,
                                                max_nr_refutations);
      break;

    case STMaxFlightsquares:
      result = maxflight_guard_can_defend_in_n(si,
                                               n,n_max_unsolvable,
                                               max_nr_refutations);
      break;

    case STMaxThreatLength:
      result = maxthreatlength_guard_can_defend_in_n(si,
                                                     n,n_max_unsolvable,
                                                     max_nr_refutations);
      break;

    case STMaxNrNonTrivial:
      result = max_nr_nontrivial_guard_can_defend_in_n(si,
                                                       n,n_max_unsolvable,
                                                       max_nr_refutations);
      break;

    case STMaxTimeDefenderFilter:
      result = maxtime_defender_filter_can_defend_in_n(si,
                                                       n,n_max_unsolvable,
                                                       max_nr_refutations);
      break;

    case STGoalReachedTester:
    case STQuodlibet:
      assert(n==slack_length_battle);
      switch (slice_has_solution(si))
      {
        case opponent_self_check:
          result = n+4;
          break;

        case has_solution:
          result = n;
          break;

        case has_no_solution:
          result = n+4;
          break;

        default:
          assert(0);
          result = n+4;
          break;
      }
      break;

    case STOutputPlaintextTreeCheckDetectorDefenderFilter:
      result = output_plaintext_tree_check_detector_can_defend_in_n(si,
                                                                    n,n_max_unsolvable,
                                                                    max_nr_refutations);
      break;

    default:
      assert(0);
      break;
  }

  assert(result>n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 no solution found
 */
boolean defense_defend(slice_index si)
{
  boolean result = true;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type min_length = slices[si].u.branch.min_length;
  stip_length_type n_max_unsolvable = min_length-2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = defense_defend_in_n(si,length,min_length,n_max_unsolvable)>length;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @return true iff there is >=1 refutation
 */
boolean defense_can_defend(slice_index si)
{
  boolean result = true;
  stip_length_type const n = slices[si].u.branch.length;
  stip_length_type n_max_unsolvable = slices[si].u.branch.min_length-2;
  unsigned int const max_nr_allowed_refutations = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = defense_can_defend_in_n(si,
                                   n,n_max_unsolvable,
                                   max_nr_allowed_refutations)>n;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
