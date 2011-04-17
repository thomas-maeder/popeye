#include "stipulation/battle_play/defense_play.h"
#include "pyflight.h"
#include "pykeepmt.h"
#include "pymovenb.h"
#include "pynontrv.h"
#include "pyreflxg.h"
#include "pyselfcg.h"
#include "pyselfgd.h"
#include "pythreat.h"
#include "stipulation/battle_play/end_of_branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/postkeyplay.h"
#include "stipulation/battle_play/continuation.h"
#include "stipulation/battle_play/check_detector.h"
#include "stipulation/battle_play/try.h"
#include "stipulation/battle_play/threat.h"
#include "stipulation/battle_play/defense_move_generator.h"
#include "stipulation/battle_play/defense_move.h"
#include "stipulation/battle_play/dead_end.h"
#include "stipulation/battle_play/min_length_guard.h"
#include "stipulation/goals/countermate/filter.h"
#include "stipulation/goals/prerequisite_optimiser.h"
#include "stipulation/help_play/root.h"
#include "stipulation/help_play/play.h"
#include "optimisations/optimisation_fork.h"
#include "optimisations/killer_move/final_defense_move.h"
#include "optimisations/killer_move/collector.h"
#include "optimisations/maxsolutions/guard.h"
#include "options/maxtime.h"
#include "optimisations/goals/enpassant/filter.h"
#include "output/plaintext/tree/check_writer.h"
#include "output/plaintext/tree/decoration_writer.h"
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
 * @note n==n_max_unsolvable means that we are solving refutations
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type defend(slice_index si,
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
    case STRefutationsAllocator:
      result = refutations_allocator_defend(si,n,n_max_unsolvable);
      break;

    case STTrySolver:
      result = try_solver_defend(si,n,n_max_unsolvable);
      break;

    case STContinuationSolver:
      result = continuation_solver_defend(si,n,n_max_unsolvable);
      break;

    case STCheckDetector:
      result = check_detector_defend(si,n,n_max_unsolvable);
      break;

    case STThreatSolver:
      result = threat_solver_defend(si,n,n_max_unsolvable);
      break;

    case STPostKeyPlaySuppressor:
      result = postkeyplay_suppressor_defend(si,n,n_max_unsolvable);
      break;

    case STBattleDeadEnd:
      result = battle_play_dead_end_defend(si,n,n_max_unsolvable);
      break;

    case STThreatCollector:
      result = threat_collector_defend(si,n,n_max_unsolvable);
      break;

    case STDefenseMoveGenerator:
    case STKillerMoveFinalDefenseMove:
      result = defense_move_generator_defend(si,n,n_max_unsolvable);
      break;

    case STDefenseMove:
      result = defense_move_defend(si,n,n_max_unsolvable);
      break;

    case STOptimisationFork:
      result = optimisation_fork_defend(si,n,n_max_unsolvable);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_defend(si,n,n_max_unsolvable);
      break;

    case STReflexDefenderFilter:
      result = reflex_defender_filter_defend(si,n,n_max_unsolvable);
      break;

    case STMinLengthGuard:
      result = min_length_guard_defend(si,n,n_max_unsolvable);
      break;

    case STKeepMatingFilter:
      result = keepmating_filter_defend(si,n,n_max_unsolvable);
      break;

    case STMaxFlightsquares:
      result = maxflight_guard_defend(si,n,n_max_unsolvable);
      break;

    case STMaxThreatLength:
      result = maxthreatlength_guard_defend(si,n,n_max_unsolvable);
      break;

    case STMaxNrNonTrivial:
      result = max_nr_nontrivial_guard_defend(si,n,n_max_unsolvable);
      break;

    case STMaxNrNonChecks:
      result = max_nr_noncheck_guard_defend(si,n,n_max_unsolvable);
      break;

    case STMaxTimeGuard:
      result = maxtime_guard_defend(si,n,n_max_unsolvable);
      break;

    case STMaxSolutionsGuard:
      result = maxsolutions_guard_defend(si,n,n_max_unsolvable);
      break;

    case STRestartGuard:
      result = restart_guard_defend(si,n,n_max_unsolvable);
      break;

    case STContinuationWriter:
      result = continuation_writer_defend(si,n,n_max_unsolvable);
      break;

    case STOutputPlaintextTreeGoalWriter:
      result = output_plaintext_tree_goal_writer_defend(si,n,n_max_unsolvable);
      break;

    case STOutputPlaintextTreeCheckWriter:
      result = output_plaintext_tree_check_writer_defend(si,n,n_max_unsolvable);
      break;

    case STOutputPlaintextTreeDecorationWriter:
      result = output_plaintext_tree_decoration_writer_defend(si,
                                                              n,
                                                              n_max_unsolvable);
      break;

    case STKeyWriter:
      result = key_writer_defend(si,n,n_max_unsolvable);
      break;

    case STTryWriter:
      result = try_writer_defend(si,n,n_max_unsolvable);
      break;

    case STEndOfSolutionWriter:
      result = end_of_solution_writer_defend(si,n,n_max_unsolvable);
      break;

    case STEnPassantFilter:
      result = enpassant_filter_defend(si,n,n_max_unsolvable);
      break;

    case STKillerMoveCollector:
      result = killer_move_collector_defend(si,n,n_max_unsolvable);
      break;

    case STCounterMateFilter:
      result = countermate_defender_filter_defend(si,n,n_max_unsolvable);
      break;

    case STPrerequisiteOptimiser:
      result = goal_prerequisite_optimiser_defend(si,n,n_max_unsolvable);
      break;

    case STEndOfBranch:
      result = end_of_branch_defend(si,n,n_max_unsolvable);
      break;

    case STLeaf:
      result = n;
      break;

    default:
      switch (slice_solve(si))
      {
        case opponent_self_check:
          result = n+4;
          break;

        case has_solution:
          result = slack_length_battle;
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
  }

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
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type can_defend(slice_index si,
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
    case STRefutationsAllocator:
      result = refutations_allocator_can_defend(si,n,n_max_unsolvable);
      break;

    case STTrySolver:
      result = try_solver_can_defend(si,n,n_max_unsolvable);
      break;

    case STPostKeyPlaySuppressor:
      result = postkeyplay_suppressor_can_defend(si,n,n_max_unsolvable);
      break;

    case STContinuationSolver:
      result = continuation_solver_can_defend(si,n,n_max_unsolvable);
      break;

    case STCheckDetector:
      result = check_detector_can_defend(si,n,n_max_unsolvable);
      break;

    case STThreatSolver:
      result = threat_solver_can_defend(si,n,n_max_unsolvable);
      break;

    case STBattleDeadEnd:
      result = battle_play_dead_end_can_defend(si,n,n_max_unsolvable);
      break;

    case STThreatCollector:
      result = threat_collector_can_defend(si,n,n_max_unsolvable);
      break;

    case STDefenseMoveGenerator:
      result = defense_move_generator_can_defend(si,n,n_max_unsolvable);
      break;

    case STDefenseMove:
      result = defense_move_can_defend(si,n,n_max_unsolvable);
      break;

    case STKillerMoveFinalDefenseMove:
      result = killer_move_final_defense_move_can_defend(si,n,n_max_unsolvable);
      break;

    case STOptimisationFork:
      result = optimisation_fork_can_defend(si,n,n_max_unsolvable);
      break;

    case STReflexDefenderFilter:
      result = reflex_defender_filter_can_defend(si,n,n_max_unsolvable);
      break;

    case STMinLengthGuard:
      result = min_length_guard_can_defend(si,n,n_max_unsolvable);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_can_defend(si,n,n_max_unsolvable);
      break;

    case STKeepMatingFilter:
      result = keepmating_filter_can_defend(si,n,n_max_unsolvable);
      break;

    case STMaxFlightsquares:
      result = maxflight_guard_can_defend(si,n,n_max_unsolvable);
      break;

    case STMaxThreatLength:
      result = maxthreatlength_guard_can_defend_in_n(si,n,n_max_unsolvable);
      break;

    case STMaxNrNonTrivial:
      result = max_nr_nontrivial_guard_can_defend(si,n,n_max_unsolvable);
      break;

    case STMaxNrNonChecks:
      result = max_nr_noncheck_guard_can_defend(si,n,n_max_unsolvable);
      break;

    case STMaxTimeGuard:
      result = maxtime_guard_can_defend(si,n,n_max_unsolvable);
      break;

    case STKeyWriter:
      result = key_writer_can_defend(si,n,n_max_unsolvable);
      break;

    case STTryWriter:
      result = try_writer_can_defend(si,n,n_max_unsolvable);
      break;

    case STContinuationWriter:
      result = continuation_writer_can_defend(si,n,n_max_unsolvable);
      break;

    case STOutputPlaintextTreeGoalWriter:
      result = output_plaintext_tree_goal_writer_can_defend(si,
                                                            n,n_max_unsolvable);
      break;

    case STOutputPlaintextTreeCheckWriter:
      result = output_plaintext_tree_check_writer_can_defend(si,
                                                             n,n_max_unsolvable);
      break;

    case STOutputPlaintextTreeDecorationWriter:
      result = output_plaintext_tree_decoration_writer_can_defend(si,
                                                                  n,
                                                                  n_max_unsolvable);
      break;

    case STEndOfSolutionWriter:
      result = end_of_solution_writer_can_defend(si,n,n_max_unsolvable);
      break;

    case STEnPassantFilter:
      result = enpassant_filter_can_defend(si,n,n_max_unsolvable);
      break;

    case STKillerMoveCollector:
      result = killer_move_collector_can_defend(si,n,n_max_unsolvable);
      break;

    case STCounterMateFilter:
      result = countermate_defender_filter_can_defend(si,n,n_max_unsolvable);
      break;

    case STPrerequisiteOptimiser:
      result = goal_prerequisite_optimiser_can_defend(si,n,n_max_unsolvable);
      break;

    case STEndOfBranch:
      result = end_of_branch_can_defend(si,n,n_max_unsolvable);
      break;

    case STLeaf:
      result = n;
      break;

    default:
      switch (slice_has_solution(si))
      {
        case opponent_self_check:
          result = n+4;
          break;

        case has_solution:
          result = slack_length_battle;
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
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
