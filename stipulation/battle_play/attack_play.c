#include "stipulation/battle_play/attack_play.h"
#include "pybrafrk.h"
#include "pyhash.h"
#include "pykeepmt.h"
#include "pyselfcg.h"
#include "pymovenb.h"
#include "pykeepmt.h"
#include "pyflight.h"
#include "pydegent.h"
#include "pythreat.h"
#include "pynontrv.h"
#include "pyproof.h"
#include "stipulation/constraint.h"
#include "stipulation/dead_end.h"
#include "stipulation/end_of_branch.h"
#include "stipulation/end_of_branch_goal.h"
#include "stipulation/dummy_move.h"
#include "stipulation/move_played.h"
#include "stipulation/check_zigzag_jump.h"
#include "stipulation/boolean/or.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/goals/doublemate/filter.h"
#include "stipulation/goals/countermate/filter.h"
#include "stipulation/goals/prerequisite_optimiser.h"
#include "solving/fork_on_remaining.h"
#include "solving/find_shortest.h"
#include "solving/move_generator.h"
#include "solving/castling_intermediate_move_generator.h"
#include "solving/single_move_generator.h"
#include "solving/single_move_generator_with_king_capture.h"
#include "solving/single_piece_move_generator.h"
#include "solving/king_move_generator.h"
#include "solving/non_king_move_generator.h"
#include "solving/capture_counter.h"
#include "solving/legal_move_counter.h"
#include "solving/for_each_move.h"
#include "solving/find_by_increasing_length.h"
#include "solving/find_move.h"
#include "solving/play_suppressor.h"
#include "solving/avoid_unsolvable.h"
#include "solving/trivial_end_filter.h"
#include "solving/battle_play/threat.h"
#include "solving/battle_play/try.h"
#include "solving/battle_play/min_length_guard.h"
#include "solving/battle_play/min_length_optimiser.h"
#include "solving/battle_play/continuation.h"
#include "conditions/bgl.h"
#include "conditions/exclusive.h"
#include "conditions/ohneschach/immobility_tester.h"
#include "options/maxsolutions/guard.h"
#include "options/stoponshortsolutions/filter.h"
#include "options/no_short_variations/no_short_variations_attacker_filter.h"
#include "options/movenumbers/restart_guard_intelligent.h"
#include "options/maxtime.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_move_generator.h"
#include "optimisations/intelligent/moves_left.h"
#include "optimisations/intelligent/limit_nr_solutions_per_target.h"
#include "optimisations/intelligent/proof.h"
#include "optimisations/intelligent/duplicate_avoider.h"
#include "optimisations/intelligent/limit_nr_solutions_per_target.h"
#include "optimisations/intelligent/mate/filter.h"
#include "optimisations/intelligent/mate/goalreachable_guard.h"
#include "optimisations/intelligent/stalemate/immobilise_black.h"
#include "optimisations/intelligent/stalemate/goalreachable_guard.h"
#include "optimisations/intelligent/stalemate/filter.h"
#include "optimisations/goals/castling/filter.h"
#include "optimisations/goals/enpassant/filter.h"
#include "optimisations/killer_move/collector.h"
#include "optimisations/count_nr_opponent_moves/opponent_moves_counter.h"
#include "output/plaintext/tree/check_writer.h"
#include "output/plaintext/tree/zugzwang_writer.h"
#include "output/plaintext/tree/move_writer.h"
#include "output/plaintext/tree/refutation_writer.h"
#include "output/plaintext/tree/refuting_variation_writer.h"
#include "output/plaintext/tree/goal_writer.h"
#include "output/plaintext/line/line_writer.h"
#include "trace.h"

#include <assert.h>

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type attack(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(slice_type,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STThreatEnforcer:
      result = threat_enforcer_attack(si,n);
      break;

    case STZugzwangWriter:
      result = zugzwang_writer_attack(si,n);
      break;

    case STRefutationsAvoider:
      result = refutations_avoider_attack(si,n);
      break;

    case STRefutationsFilter:
      result = refutations_filter_attack(si,n);
      break;

    case STMoveWriter:
      result = move_writer_attack(si,n);
      break;

    case STTrivialEndFilter:
      result = trivial_end_filter_attack(si,n);
      break;

    case STRefutingVariationWriter:
      result = refuting_variation_writer_attack(si,n);
      break;

    case STNoShortVariations:
      result = no_short_variations_attack(si,n);
      break;

    case STOr:
      result = or_attack(si,n);
      break;

    case STFindShortest:
      result = find_shortest_attack(si,n);
      break;

    case STMoveGenerator:
      result = move_generator_attack(si,n);
      break;

    case STForEachMove:
      result = for_each_move_attack(si,n);
      break;

    case STMovePlayed:
      result = move_played_attack(si,n);
      break;

#if defined(DOTRACE)
    case STMoveTracer:
      result = move_tracer_attack(si,n);
      break;
#endif

    case STOrthodoxMatingMoveGenerator:
      result = orthodox_mating_move_generator_attack(si,n);
      break;

    case STDeadEnd:
      result = dead_end_attack(si,n);
      break;

    case STMinLengthOptimiser:
      result = min_length_optimiser_attack(si,n);
      break;

    case STForkOnRemaining:
      result = fork_on_remaining_attack(si,n);
      break;

    case STAttackHashed:
      result = attack_hashed_attack(si,n);
      break;

    case STEndOfBranch:
    case STEndOfBranchForced:
    case STEndOfBranchTester:
      result = end_of_branch_attack(si,n);
      break;

    case STEndOfBranchGoal:
    case STEndOfBranchGoalImmobile:
    case STEndOfBranchGoalTester:
      result = end_of_branch_goal_attack(si,n);
      break;

    case STGoalReachedTester:
      result = goal_reached_tester_attack(si,n);
      break;

    case STAvoidUnsolvable:
      result = avoid_unsolvable_attack(si,n);
      break;

    case STResetUnsolvable:
      result = reset_unsolvable_attack(si,n);
      break;

    case STLearnUnsolvable:
      result = learn_unsolvable_attack(si,n);
      break;

    case STConstraintSolver:
    case STConstraintTester:
      result = constraint_attack(si,n);
      break;

    case STSelfCheckGuard:
      result = selfcheck_guard_attack(si,n);
      break;

    case STKeepMatingFilter:
      result = keepmating_filter_attack(si,n);
      break;

    case STOutputPlaintextTreeCheckWriter:
      result = output_plaintext_tree_check_writer_attack(si,n);
      break;

    case STRefutationWriter:
      result = refutation_writer_attack(si,n);
      break;

    case STDoubleMateFilter:
      result = doublemate_filter_attack(si,n);
      break;

    case STCounterMateFilter:
      result = countermate_filter_attack(si,n);
      break;

    case STEnPassantFilter:
      result = enpassant_filter_attack(si,n);
      break;

    case STCastlingFilter:
      result = castling_filter_attack(si,n);
      break;

    case STPrerequisiteOptimiser:
      result = goal_prerequisite_optimiser_attack(si,n);
      break;

    case STOutputPlaintextTreeGoalWriter:
      result = output_plaintext_tree_goal_writer_attack(si,n);
      break;

    case STOutputPlaintextLineLineWriter:
      result = line_writer_attack(si,n);
      break;

    case STBGLFilter:
      result = bgl_filter_attack(si,n);
      break;

    case STRefutationsCollector:
      result = refutations_collector_attack(si,n);
      break;

    case STMinLengthGuard:
      result = min_length_guard_attack(si,n);
      break;

    case STFindMove:
      result = find_move_attack(si,n);
      break;

    case STAttackHashedTester:
      result = attack_hashed_tester_attack(si,n);
      break;

    case STDegenerateTree:
      result = degenerate_tree_attack(si,n);
      break;

    case STMaxNrNonTrivialCounter:
      result = max_nr_nontrivial_counter_attack(si,n);
      break;

    case STKillerMoveCollector:
      result = killer_move_collector_attack(si,n);
      break;

    case STFindByIncreasingLength:
      result = find_by_increasing_length_attack(si,n);
      break;

    case STHelpMovePlayed:
      result = help_move_played_attack(si,n);
      break;

    case STDeadEndGoal:
      result = dead_end_help(si,n);
      break;

    case STHelpHashed:
      result = help_hashed_attack(si,n);
      break;

    case STIntelligentMovesLeftInitialiser:
      result = intelligent_moves_left_initialiser_attack(si,n);
      break;

    case STRestartGuardIntelligent:
      result = restart_guard_intelligent_attack(si,n);
      break;

    case STIntelligentTargetCounter:
      result = intelligent_target_counter_attack(si,n);
      break;

    case STIntelligentMateFilter:
      result = intelligent_mate_filter_attack(si,n);
      break;

    case STIntelligentStalemateFilter:
      result = intelligent_stalemate_filter_attack(si,n);
      break;

    case STIntelligentProof:
      result = intelligent_proof_attack(si,n);
      break;

    case STIntelligentLimitNrSolutionsPerTargetPos:
      result = intelligent_limit_nr_solutions_per_target_position_attack(si,n);
      break;

    case STGoalReachableGuardFilterMate:
      result = goalreachable_guard_mate_attack(si,n);
      break;

    case STGoalReachableGuardFilterStalemate:
      result = goalreachable_guard_stalemate_attack(si,n);
      break;

    case STGoalReachableGuardFilterProof:
      result = goalreachable_guard_proofgame_attack(si,n);
      break;

    case STGoalReachableGuardFilterProofFairy:
      result = goalreachable_guard_proofgame_fairy_attack(si,n);
      break;

    case STRestartGuard:
      result = restart_guard_attack(si,n);
      break;

    case STMaxTimeGuard:
      result = maxtime_guard_attack(si,n);
      break;

    case STMaxSolutionsCounter:
      result = maxsolutions_counter_attack(si,n);
      break;

    case STMaxSolutionsGuard:
      result = maxsolutions_guard_attack(si,n);
      break;

    case STStopOnShortSolutionsFilter:
      result = stoponshortsolutions_attack(si,n);
      break;

    case STCheckZigzagJump:
      result = check_zigzag_jump_attack(si,n);
      break;

    case STDummyMove:
      result = dummy_move_attack(si,n);
      break;

    case STHelpHashedTester:
      result = help_hashed_tester_attack(si,n);
      break;

    case STFlightsquaresCounter:
      result = flightsquares_counter_attack(si,n);
      break;

    case STKingMoveGenerator:
      result = king_move_generator_attack(si,n);
      break;

    case STNonKingMoveGenerator:
      result = non_king_move_generator_attack(si,n);
      break;

    case STLegalMoveCounter:
    case STAnyMoveCounter:
      result = legal_move_counter_attack(si,n);
      break;

    case STCaptureCounter:
      result = capture_counter_attack(si,n);
      break;

    case STOhneschachSuspender:
      result = ohneschach_suspender_attack(si,n);
      break;

    case STOhneschachCheckGuard:
      result = ohneschach_check_guard_attack(si,n);
      break;

    case STExclusiveChessUnsuspender:
      result = exclusive_chess_unsuspender_attack(si,n);
      break;

    case STSingleMoveGeneratorWithKingCapture:
      result = single_move_generator_with_king_capture_attack(si,n);
      break;

    case STSinglePieceMoveGenerator:
      result = single_piece_move_generator_attack(si,n);
      break;

    case STCastlingIntermediateMoveGenerator:
      result = castling_intermediate_move_generator_attack(si,n);
      break;

    case STSingleMoveGenerator:
      result = single_move_generator_attack(si,n);
      break;

    case STOpponentMovesCounter:
      result = opponent_moves_counter_attack(si,n);
      break;

    case STIntelligentImmobilisationCounter:
      result = intelligent_immobilisation_counter_attack(si,n);
      break;

    case STIntelligentDuplicateAvoider:
      result = intelligent_duplicate_avoider_attack(si,n);
      break;

    case STIntelligentSolutionsPerTargetPosCounter:
      result = intelligent_nr_solutions_per_target_position_counter_attack(si,n);
      break;

    case STTrue:
      result = slack_length;
      break;

    case STFalse:
      result = n+2;
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
