#include "stipulation/battle_play/defense_play.h"
#include "pyflight.h"
#include "pykeepmt.h"
#include "pymovenb.h"
#include "pynontrv.h"
#include "pyreflxg.h"
#include "pyselfcg.h"
#include "pyselfgd.h"
#include "pythreat.h"
#include "stipulation/goal_reached_tester.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/postkeyplay.h"
#include "stipulation/battle_play/continuation.h"
#include "stipulation/battle_play/check_detector.h"
#include "stipulation/battle_play/try.h"
#include "stipulation/battle_play/threat.h"
#include "stipulation/battle_play/defense_move.h"
#include "stipulation/battle_play/defense_fork.h"
#include "stipulation/battle_play/ready_for_defense.h"
#include "stipulation/help_play/root.h"
#include "stipulation/help_play/play.h"
#include "conditions/amu/mate_filter.h"
#include "conditions/circe/steingewinn_filter.h"
#include "conditions/ultraschachzwang/goal_filter.h"
#include "optimisations/killer_move/final_defense_move.h"
#include "optimisations/killer_move/collector.h"
#include "optimisations/maxsolutions/root_defender_filter.h"
#include "optimisations/maxtime/root_defender_filter.h"
#include "optimisations/maxtime/defender_filter.h"
#include "optimisations/goals/enpassant/defender_filter.h"
#include "output/plaintext/tree/check_writer.h"
#include "output/plaintext/tree/key_writer.h"
#include "output/plaintext/tree/try_writer.h"
#include "output/plaintext/tree/end_of_solution_writer.h"
#include "output/plaintext/tree/continuation_writer.h"
#include "output/plaintext/tree/goal_writer.h"
#include "trace.h"

#include <assert.h>

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type defense_defend_in_n(slice_index si,
                                     stip_length_type n,
                                     stip_length_type n_max_unsolvable)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STTrySolver:
      result = try_solver_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STSolutionSolver:
    case STContinuationSolver:
      result = continuation_solver_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STCheckDetector:
      result = check_detector_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STThreatSolver:
      result = threat_solver_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STPostKeyPlaySuppressor:
      result = postkeyplay_suppressor_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STThreatCollector:
      result = threat_collector_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STDefenseMove:
      result = defense_move_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STKillerMoveFinalDefenseMove:
      result = killer_move_final_defense_move_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STDefenseFork:
      result = defense_fork_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STReadyForDefense:
      result = ready_for_defense_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STSelfCheckGuardDefenderFilter:
      result = selfcheck_guard_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STReflexDefenderFilter:
      result = reflex_defender_filter_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STKeepMatingGuardDefenderFilter:
      result = keepmating_guard_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STMaxFlightsquares:
      result = maxflight_guard_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STMaxThreatLength:
      result = maxthreatlength_guard_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STMaxNrNonTrivial:
      result = max_nr_nontrivial_guard_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STMaxNrNonChecks:
      result = max_nr_noncheck_guard_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STMaxTimeDefenderFilter:
      result = maxtime_defender_filter_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STMaxSolutionsRootDefenderFilter:
      result = maxsolutions_root_defender_filter_defend_in_n(si,
                                                             n,
                                                             n_max_unsolvable);
      break;

    case STRestartGuardRootDefenderFilter:
      result = restart_guard_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STMaxTimeRootDefenderFilter:
      result = maxtime_root_defender_filter_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STGoalReachedTester:
      result = goal_reached_tester_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STContinuationWriter:
      result = continuation_writer_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STOutputPlaintextTreeGoalWriter:
      result = output_plaintext_tree_goal_writer_defend_in_n(si,
                                                             n,
                                                             n_max_unsolvable);
      break;

    case STOutputPlaintextTreeCheckWriterDefenderFilter:
      result = output_plaintext_tree_check_writer_defend_in_n(si,
                                                                n,
                                                                n_max_unsolvable);
      break;

    case STKeyWriter:
      result = key_writer_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STTryWriter:
      result = try_writer_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STEndOfSolutionWriter:
      result = end_of_solution_writer_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STEnPassantDefenderFilter:
      result = enpassant_defender_filter_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STKillerMoveCollector:
      result = killer_move_collector_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STLeaf:
      result = n;
      break;

    case STAmuMateFilter:
      result = amu_mate_filter_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STUltraschachzwangGoalFilter:
      result = ultraschachzwang_goal_filter_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STCirceSteingewinnFilter:
      result = circe_steingewinn_filter_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STQuodlibet:
    case STOutputPlaintextLineLineWriter:
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
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type defense_can_defend_in_n(slice_index si,
                                         stip_length_type n,
                                         stip_length_type n_max_unsolvable)
{
  stip_length_type result = n+4;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STTrySolver:
      result = try_solver_can_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STPostKeyPlaySuppressor:
      result = postkeyplay_suppressor_can_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STSolutionSolver:
    case STContinuationSolver:
      result = continuation_solver_can_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STCheckDetector:
      result = check_detector_can_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STThreatSolver:
      result = threat_solver_can_defend_in_n(si,n,n_max_unsolvable);
      break;


    case STThreatCollector:
      result = threat_collector_can_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STDefenseMove:
      result = defense_move_can_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STKillerMoveFinalDefenseMove:
      result = killer_move_final_defense_move_can_defend_in_n(si,
                                                              n,
                                                              n_max_unsolvable);
      break;

    case STDefenseFork:
      result = defense_fork_can_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STReadyForDefense:
      result = ready_for_defense_can_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STReflexDefenderFilter:
      result = reflex_defender_filter_can_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STSelfCheckGuardDefenderFilter:
      result = selfcheck_guard_can_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STKeepMatingGuardDefenderFilter:
      result = keepmating_guard_can_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STMaxFlightsquares:
      result = maxflight_guard_can_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STMaxThreatLength:
      result = maxthreatlength_guard_can_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STMaxNrNonTrivial:
      result = max_nr_nontrivial_guard_can_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STMaxNrNonChecks:
      result = max_nr_noncheck_guard_can_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STMaxTimeDefenderFilter:
      result = maxtime_defender_filter_can_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STGoalReachedTester:
      result = goal_reached_tester_can_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STKeyWriter:
      result = key_writer_can_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STTryWriter:
      result = try_writer_can_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STContinuationWriter:
      result = continuation_writer_can_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STOutputPlaintextTreeGoalWriter:
      result = output_plaintext_tree_goal_writer_can_defend_in_n(si,
                                                                 n,n_max_unsolvable);
      break;

    case STOutputPlaintextTreeCheckWriterDefenderFilter:
      result = output_plaintext_tree_check_writer_can_defend_in_n(si,
                                                                  n,n_max_unsolvable);
      break;

    case STEnPassantDefenderFilter:
      result = enpassant_defender_filter_can_defend_in_n(si,
                                                         n,n_max_unsolvable);
      break;

    case STKillerMoveCollector:
      result = killer_move_collector_can_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STLeaf:
      result = n;
      break;

    case STAmuMateFilter:
      result = amu_mate_filter_can_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STUltraschachzwangGoalFilter:
      result = ultraschachzwang_goal_filter_can_defend_in_n(si,
                                                            n,n_max_unsolvable);
      break;

    case STCirceSteingewinnFilter:
      result = circe_steingewinn_filter_can_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STQuodlibet:
    case STOutputPlaintextLineLineWriter:
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
  stip_length_type n_max_unsolvable = slack_length_battle-1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = defense_defend_in_n(si,length,n_max_unsolvable)>length;

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
  stip_length_type n_max_unsolvable = slices[si].u.branch.min_length-1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = defense_can_defend_in_n(si,n,n_max_unsolvable)>n;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
