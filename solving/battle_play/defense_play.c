#include "solving/battle_play/defense_play.h"
#include "solving/selfcheck_guard.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/constraint.h"
#include "stipulation/dead_end.h"
#include "stipulation/end_of_branch.h"
#include "stipulation/end_of_branch_goal.h"
#include "stipulation/move_player.h"
#include "stipulation/move_played.h"
#include "stipulation/dummy_move.h"
#include "stipulation/if_then_else.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/goals/countermate/filter.h"
#include "stipulation/goals/prerequisite_optimiser.h"
#include "stipulation/goals/doublemate/king_capture_avoider.h"
#include "solving/battle_play/attack_play.h"
#include "solving/fork_on_remaining.h"
#include "solving/move_generator.h"
#include "solving/for_each_move.h"
#include "solving/find_move.h"
#include "solving/play_suppressor.h"
#include "solving/battle_play/continuation.h"
#include "solving/battle_play/check_detector.h"
#include "solving/battle_play/try.h"
#include "solving/battle_play/threat.h"
#include "solving/battle_play/continuation.h"
#include "solving/find_by_increasing_length.h"
#include "solving/battle_play/min_length_guard.h"
#include "solving/avoid_unsolvable.h"
#include "conditions/bgl.h"
#include "conditions/blackchecks.h"
#include "conditions/extinction.h"
#include "conditions/exclusive.h"
#include "conditions/singlebox/type1.h"
#include "conditions/singlebox/type2.h"
#include "conditions/singlebox/type3.h"
#include "conditions/ohneschach/legality_tester.h"
#include "conditions/circe/assassin.h"
#include "conditions/patience.h"
#include "conditions/isardam.h"
#include "conditions/ultraschachzwang/legality_tester.h"
#include "optimisations/keepmating.h"
#include "optimisations/killer_move/final_defense_move.h"
#include "optimisations/killer_move/collector.h"
#include "optimisations/goals/enpassant/filter.h"
#include "optimisations/goals/castling/filter.h"
#include "options/maxsolutions/guard.h"
#include "options/maxtime.h"
#include "options/nontrivial.h"
#include "options/maxthreatlength.h"
#include "options/maxflightsquares.h"
#include "options/movenumbers.h"
#include "output/plaintext/tree/check_writer.h"
#include "output/plaintext/tree/key_writer.h"
#include "output/plaintext/tree/threat_writer.h"
#include "output/plaintext/tree/try_writer.h"
#include "output/plaintext/tree/refutation_writer.h"
#include "output/plaintext/tree/end_of_solution_writer.h"
#include "output/plaintext/tree/move_writer.h"
#include "output/plaintext/tree/goal_writer.h"
#include "output/plaintext/line/line_writer.h"
#include "debugging/trace.h"

#include <assert.h>

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type defend(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(slice_type,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STRefutationsAllocator:
      result = refutations_allocator_defend(si,n);
      break;

    case STRefutationsSolver:
      result = refutations_solver_defend(si,n);
      break;

    case STRefutationsIntroWriter:
      result = refutations_intro_writer_defend(si,n);
      break;

    case STContinuationSolver:
      result = continuation_solver_defend(si,n);
      break;

    case STCheckDetector:
      result = check_detector_defend(si,n);
      break;

    case STThreatSolver:
      result = threat_solver_defend(si,n);
      break;

    case STPlaySuppressor:
      result = play_suppressor_defend(si,n);
      break;

    case STDeadEnd:
    case STDeadEndGoal:
      result = dead_end_defend(si,n);
      break;

    case STThreatCollector:
      result = threat_collector_defend(si,n);
      break;

    case STMoveGenerator:
      result = move_generator_defend(si,n);
      break;

    case STForEachMove:
      result = for_each_move_defend(si,n);
      break;

    case STMovePlayer:
      result = move_player_defend(si,n);
      break;

    case STMovePlayed:
      result = move_played_defend(si,n);
      break;

#if defined(DOTRACE)
    case STMoveTracer:
      result = move_tracer_defend(si,n);
      break;
#endif

    case STForkOnRemaining:
      result = fork_on_remaining_defend(si,n);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_defend(si,n);
      break;

    case STMinLengthGuard:
      result = min_length_guard_defend(si,n);
      break;

    case STKeepMatingFilter:
      result = keepmating_filter_defend(si,n);
      break;

    case STMaxFlightsquares:
      result = maxflight_guard_defend(si,n);
      break;

    case STMaxNrNonTrivial:
      result = max_nr_nontrivial_guard_defend(si,n);
      break;

    case STMaxTimeGuard:
      result = maxtime_guard_defend(si,n);
      break;

    case STMaxSolutionsGuard:
      result = maxsolutions_guard_defend(si,n);
      break;

    case STMaxSolutionsCounter:
      result = maxsolutions_counter_defend(si,n);
      break;

    case STRestartGuard:
      result = restart_guard_defend(si,n);
      break;

    case STThreatWriter:
      result = threat_writer_defend(si,n);
      break;

    case STMoveWriter:
      result = move_writer_defend(si,n);
      break;

    case STOutputPlaintextTreeGoalWriter:
      result = output_plaintext_tree_goal_writer_defend(si,n);
      break;

    case STOutputPlaintextTreeCheckWriter:
      result = output_plaintext_tree_check_writer_defend(si,n);
      break;

    case STKeyWriter:
      result = key_writer_defend(si,n);
      break;

    case STTryWriter:
      result = try_writer_defend(si,n);
      break;

    case STEndOfSolutionWriter:
      result = end_of_solution_writer_defend(si,n);
      break;

    case STEnPassantFilter:
      result = enpassant_filter_defend(si,n);
      break;

    case STCastlingFilter:
      result = castling_filter_defend(si,n);
      break;

    case STCounterMateFilter:
      result = countermate_filter_defend(si,n);
      break;

    case STPrerequisiteOptimiser:
      result = goal_prerequisite_optimiser_defend(si,n);
      break;

    case STEndOfBranch:
    case STEndOfBranchForced:
      result = end_of_branch_defend(si,n);
      break;

    case STEndOfBranchGoal:
      result = end_of_branch_goal_defend(si,n);
      break;

    case STAvoidUnsolvable:
      result = avoid_unsolvable_defend(si,n);
      break;

    case STResetUnsolvable:
      result = reset_unsolvable_defend(si,n);
      break;

    case STConstraintSolver:
    case STConstraintTester:
    case STGoalConstraintTester:
      result = constraint_defend(si,n);
      break;

    case STDummyMove:
      result = dummy_move_defend(si,n);
      break;

    case STIfThenElse:
      result = if_then_else_defend(si,n);
      break;

    case STOutputPlaintextLineLineWriter:
      result = output_plaintext_line_line_writer_defend(si,n);
      break;

    case STBGLFilter:
      result = bgl_filter_defend(si,n);
      break;

    case STBlackChecks:
      result = blackchecks_defend(si,n);
      break;

    case STExtinctionRememberThreatened:
      result = extinction_remember_threatened_defend(si,n);
      break;

    case STExtinctionTester:
      result = extinction_tester_defend(si,n);
      break;

    case STSingleBoxType1LegalityTester:
      result = singlebox_type1_legality_tester_defend(si,n);
      break;

    case STSingleBoxType2LegalityTester:
      result = singlebox_type2_legality_tester_defend(si,n);
      break;

    case STSingleBoxType3LegalityTester:
      result = singlebox_type3_legality_tester_defend(si,n);
      break;

    case STExclusiveChessLegalityTester:
      result = exclusive_chess_legality_tester_defend(si,n);
      break;

    case STOhneschachLegalityTester:
      result = ohneschach_legality_tester_defend(si,n);
      break;

    case STUltraschachzwangLegalityTester:
      result = ultraschachzwang_legality_tester_defend(si,n);
      break;

    case STIsardamLegalityTester:
      result = isardam_legality_tester_defend(si,n);
      break;

    case STKingAssassinationAvoider:
      result = king_assassination_avoider_defend(si,n);
      break;

    case STKingCaptureAvoider:
      result = king_capture_avoider_defend(si,n);
      break;

    case STPatienceChessLegalityTester:
      result = patience_chess_legality_tester_defend(si,n);
      break;

    case STThreatDefeatedTester:
      result = threat_defeated_tester_defend(si,n);
      break;

    case STFindMove:
      result = find_move_defend(si,n);
      break;

    case STKillerMoveFinalDefenseMove:
      result = killer_move_final_defense_move_defend(si,n);
      break;

    case STMaxThreatLength:
      result = maxthreatlength_guard_defend(si,n);
      break;

    case STKillerMoveCollector:
      result = killer_move_collector_defend(si,n);
      break;

    case STGoalReachedTester:
      result = goal_reached_tester_defend(si,n);
      break;

    case STTrue:
      result = slack_length;
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
