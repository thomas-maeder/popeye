#include "stipulation/battle_play/defense_play.h"
#include "pyflight.h"
#include "pykeepmt.h"
#include "pymovenb.h"
#include "pynontrv.h"
#include "pyselfcg.h"
#include "pyselfgd.h"
#include "pythreat.h"
#include "stipulation/constraint.h"
#include "stipulation/dead_end.h"
#include "stipulation/end_of_branch.h"
#include "stipulation/move.h"
#include "stipulation/dummy_move.h"
#include "stipulation/check_zigzag_jump.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/goals/countermate/filter.h"
#include "stipulation/goals/prerequisite_optimiser.h"
#include "stipulation/help_play/play.h"
#include "solving/fork_on_remaining.h"
#include "solving/move_generator.h"
#include "solving/play_suppressor.h"
#include "solving/battle_play/continuation.h"
#include "solving/battle_play/check_detector.h"
#include "solving/battle_play/try.h"
#include "solving/battle_play/threat.h"
#include "solving/battle_play/continuation.h"
#include "solving/find_by_increasing_length.h"
#include "solving/battle_play/min_length_guard.h"
#include "optimisations/killer_move/move_generator.h"
#include "optimisations/killer_move/final_defense_move.h"
#include "optimisations/killer_move/collector.h"
#include "optimisations/count_nr_opponent_moves/move_generator.h"
#include "optimisations/goals/enpassant/filter.h"
#include "options/maxsolutions/guard.h"
#include "options/maxtime.h"
#include "output/plaintext/tree/check_writer.h"
#include "output/plaintext/tree/decoration_writer.h"
#include "output/plaintext/tree/key_writer.h"
#include "output/plaintext/tree/try_writer.h"
#include "output/plaintext/tree/end_of_solution_writer.h"
#include "output/plaintext/tree/move_writer.h"
#include "output/plaintext/tree/goal_writer.h"
#include "output/plaintext/line/line_writer.h"
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

  TraceEnumerator(slice_type,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STRefutationsAllocator:
      result = refutations_allocator_defend(si,n,n_max_unsolvable);
      break;

    case STTrySolver:
      result = try_solver_defend(si,n,n_max_unsolvable);
      break;

    case STRefutationsSolver:
      result = refutations_solver_defend(si,n,n_max_unsolvable);
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

    case STPlaySuppressor:
      result = play_suppressor_defend(si,n,n_max_unsolvable);
      break;

    case STDeadEnd:
    case STDeadEndGoal:
      result = dead_end_defend(si,n,n_max_unsolvable);
      break;

    case STThreatCollector:
      result = threat_collector_defend(si,n,n_max_unsolvable);
      break;

    case STMoveGenerator:
    case STKillerMoveFinalDefenseMove:
      result = move_generator_defend(si,n,n_max_unsolvable);
      break;

    case STKillerMoveMoveGenerator:
      result = killer_move_move_generator_defend(si,n,n_max_unsolvable);
      break;

    case STCountNrOpponentMovesMoveGenerator:
      result = countnropponentmoves_move_generator_defend(si,n,n_max_unsolvable);
      break;

    case STMove:
      result = move_defend(si,n,n_max_unsolvable);
      break;

    case STForkOnRemaining:
      result = fork_on_remaining_defend(si,n,n_max_unsolvable);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_defend(si,n,n_max_unsolvable);
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

    case STMaxTimeGuard:
      result = maxtime_guard_defend(si,n,n_max_unsolvable);
      break;

    case STMaxSolutionsGuard:
      result = maxsolutions_guard_defend(si,n,n_max_unsolvable);
      break;

    case STRestartGuard:
      result = restart_guard_defend(si,n,n_max_unsolvable);
      break;

    case STMoveWriter:
      result = move_writer_defend(si,n,n_max_unsolvable);
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
      result = countermate_filter_defend(si,n,n_max_unsolvable);
      break;

    case STPrerequisiteOptimiser:
      result = goal_prerequisite_optimiser_defend(si,n,n_max_unsolvable);
      break;

    case STEndOfBranch:
    case STEndOfBranchForced:
    case STEndOfBranchGoal:
    case STEndOfBranchGoalImmobile:
      result = end_of_branch_defend(si,n,n_max_unsolvable);
      break;

    case STConstraint:
      result = constraint_defend(si,n,n_max_unsolvable);
      break;

    case STDummyMove:
      result = dummy_move_defend(si,n,n_max_unsolvable);
      break;

    case STCheckZigzagJump:
      result = check_zigzag_jump_defend(si,n,n_max_unsolvable);
      break;

    case STOutputPlaintextLineLineWriter:
      result = line_writer_defend(si,n,n_max_unsolvable);
      break;

    case STTrue:
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

  TraceEnumerator(slice_type,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STRefutationsAllocator:
      result = refutations_allocator_can_defend(si,n,n_max_unsolvable);
      break;

    case STTrySolver:
      result = try_solver_can_defend(si,n,n_max_unsolvable);
      break;

    case STRefutationsSolver:
      result = refutations_solver_can_defend(si,n,n_max_unsolvable);
      break;

    case STPlaySuppressor:
      result = play_suppressor_can_defend(si,n,n_max_unsolvable);
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

    case STDeadEnd:
    case STDeadEndGoal:
      result = dead_end_can_defend(si,n,n_max_unsolvable);
      break;

    case STThreatCollector:
      result = threat_collector_can_defend(si,n,n_max_unsolvable);
      break;

    case STMoveGenerator:
      result = move_generator_can_defend(si,n,n_max_unsolvable);
      break;

    case STKillerMoveMoveGenerator:
      result = killer_move_move_generator_can_defend(si,n,n_max_unsolvable);
      break;

    case STCountNrOpponentMovesMoveGenerator:
      result = countnropponentmoves_move_generator_can_defend(si,n,n_max_unsolvable);
      break;

    case STMove:
      result = move_can_defend(si,n,n_max_unsolvable);
      break;

    case STKillerMoveFinalDefenseMove:
      result = killer_move_final_defense_move_can_defend(si,n,n_max_unsolvable);
      break;

    case STForkOnRemaining:
      result = fork_on_remaining_can_defend(si,n,n_max_unsolvable);
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

    case STMaxTimeGuard:
      result = maxtime_guard_can_defend(si,n,n_max_unsolvable);
      break;

    case STKeyWriter:
      result = key_writer_can_defend(si,n,n_max_unsolvable);
      break;

    case STTryWriter:
      result = try_writer_can_defend(si,n,n_max_unsolvable);
      break;

    case STMoveWriter:
      result = move_writer_can_defend(si,n,n_max_unsolvable);
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
      result = countermate_filter_can_defend(si,n,n_max_unsolvable);
      break;

    case STPrerequisiteOptimiser:
      result = goal_prerequisite_optimiser_can_defend(si,n,n_max_unsolvable);
      break;

    case STEndOfBranch:
    case STEndOfBranchForced:
    case STEndOfBranchGoal:
    case STEndOfBranchGoalImmobile:
      result = end_of_branch_can_defend(si,n,n_max_unsolvable);
      break;

    case STConstraint:
      result = constraint_can_defend(si,n,n_max_unsolvable);
      break;

    case STCheckZigzagJump:
      result = check_zigzag_jump_can_defend(si,n,n_max_unsolvable);
      break;

    case STDummyMove:
      result = dummy_move_can_defend(si,n,n_max_unsolvable);
      break;

    case STOutputPlaintextLineLineWriter:
      result = line_writer_can_defend(si,n,n_max_unsolvable);
      break;

    case STTrue:
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
