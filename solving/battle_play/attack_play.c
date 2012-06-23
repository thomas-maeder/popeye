#include "solving/battle_play/attack_play.h"
#include "stipulation/fork.h"
#include "pyproof.h"
#include "solving/selfcheck_guard.h"
#include "conditions/amu/mate_filter.h"
#include "conditions/anticirce/circuit_special.h"
#include "conditions/anticirce/exchange_filter.h"
#include "conditions/anticirce/exchange_special.h"
#include "conditions/anticirce/target_square_filter.h"
#include "conditions/bgl.h"
#include "conditions/blackchecks.h"
#include "conditions/extinction.h"
#include "conditions/circe/circuit_by_rebirth_special.h"
#include "conditions/circe/exchange_by_rebirth_special.h"
#include "conditions/circe/steingewinn_filter.h"
#include "conditions/exclusive.h"
#include "conditions/maff/immobility_tester.h"
#include "conditions/ohneschach/immobility_tester.h"
#include "conditions/owu/immobility_tester.h"
#include "conditions/ultraschachzwang/goal_filter.h"
#include "conditions/singlebox/type1.h"
#include "conditions/singlebox/type2.h"
#include "optimisations/hash.h"
#include "optimisations/keepmating.h"
#include "optimisations/count_nr_opponent_moves/opponent_moves_counter.h"
#include "optimisations/goals/castling/filter.h"
#include "optimisations/goals/enpassant/filter.h"
#include "optimisations/intelligent/duplicate_avoider.h"
#include "optimisations/intelligent/limit_nr_solutions_per_target.h"
#include "optimisations/intelligent/mate/filter.h"
#include "optimisations/intelligent/mate/goalreachable_guard.h"
#include "optimisations/intelligent/moves_left.h"
#include "optimisations/intelligent/proof.h"
#include "optimisations/intelligent/stalemate/filter.h"
#include "optimisations/intelligent/stalemate/goalreachable_guard.h"
#include "optimisations/intelligent/stalemate/immobilise_black.h"
#include "optimisations/killer_move/collector.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_move_generator.h"
#include "options/maxsolutions/guard.h"
#include "options/maxsolutions/initialiser.h"
#include "options/maxtime.h"
#include "options/movenumbers.h"
#include "options/degenerate_tree.h"
#include "options/nontrivial.h"
#include "options/maxthreatlength.h"
#include "options/maxflightsquares.h"
#include "options/movenumbers/restart_guard_intelligent.h"
#include "options/no_short_variations/no_short_variations_attacker_filter.h"
#include "options/stoponshortsolutions/filter.h"
#include "options/stoponshortsolutions/initialiser.h"
#include "output/plaintext/end_of_phase_writer.h"
#include "output/plaintext/illegal_selfcheck_writer.h"
#include "output/plaintext/line/end_of_intro_series_marker.h"
#include "output/plaintext/line/line_writer.h"
#include "output/plaintext/move_inversion_counter.h"
#include "output/plaintext/tree/check_writer.h"
#include "output/plaintext/tree/goal_writer.h"
#include "output/plaintext/tree/move_writer.h"
#include "output/plaintext/tree/refutation_writer.h"
#include "output/plaintext/tree/refuting_variation_writer.h"
#include "output/plaintext/tree/zugzwang_writer.h"
#include "pieces/attributes/paralysing/mate_filter.h"
#include "pieces/attributes/paralysing/stalemate_special.h"
#include "solving/avoid_unsolvable.h"
#include "solving/battle_play/continuation.h"
#include "solving/battle_play/min_length_guard.h"
#include "solving/battle_play/min_length_optimiser.h"
#include "solving/battle_play/threat.h"
#include "solving/battle_play/try.h"
#include "solving/capture_counter.h"
#include "solving/castling_intermediate_move_generator.h"
#include "solving/find_by_increasing_length.h"
#include "solving/find_move.h"
#include "solving/find_shortest.h"
#include "solving/for_each_move.h"
#include "solving/fork_on_remaining.h"
#include "solving/king_move_generator.h"
#include "solving/legal_move_counter.h"
#include "solving/move_generator.h"
#include "solving/non_king_move_generator.h"
#include "solving/play_suppressor.h"
#include "solving/single_move_generator.h"
#include "solving/single_move_generator_with_king_capture.h"
#include "solving/single_piece_move_generator.h"
#include "solving/trivial_end_filter.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/move_inverter.h"
#include "stipulation/battle_play/attack_adapter.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/defense_adapter.h"
#include "stipulation/boolean/and.h"
#include "stipulation/boolean/false.h"
#include "stipulation/boolean/not.h"
#include "stipulation/boolean/or.h"
#include "stipulation/boolean/true.h"
#include "stipulation/if_then_else.h"
#include "stipulation/constraint.h"
#include "stipulation/dead_end.h"
#include "stipulation/end_of_branch_goal.h"
#include "stipulation/end_of_branch.h"
#include "stipulation/goals/any/reached_tester.h"
#include "stipulation/goals/capture/reached_tester.h"
#include "stipulation/goals/castling/reached_tester.h"
#include "stipulation/goals/check/reached_tester.h"
#include "stipulation/goals/chess81/reached_tester.h"
#include "stipulation/goals/circuit_by_rebirth/reached_tester.h"
#include "stipulation/goals/circuit/reached_tester.h"
#include "stipulation/goals/countermate/filter.h"
#include "stipulation/goals/countermate/reached_tester.h"
#include "stipulation/goals/doublemate/filter.h"
#include "stipulation/goals/doublemate/reached_tester.h"
#include "stipulation/goals/enpassant/reached_tester.h"
#include "stipulation/goals/exchange_by_rebirth/reached_tester.h"
#include "stipulation/goals/exchange/reached_tester.h"
#include "stipulation/goals/immobile/reached_tester.h"
#include "stipulation/goals/notcheck/reached_tester.h"
#include "stipulation/goals/prerequisite_optimiser.h"
#include "stipulation/goals/proofgame/reached_tester.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/goals/steingewinn/reached_tester.h"
#include "stipulation/goals/target/reached_tester.h"
#include "stipulation/help_play/adapter.h"
#include "stipulation/move_player.h"
#include "stipulation/move_played.h"
#include "stipulation/setplay_fork.h"
#include "debugging/trace.h"

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

    case STMovePlayer:
      result = move_player_attack(si,n);
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
    case STGoalConstraintTester:
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

    case STIfThenElse:
      result = if_then_else_attack(si,n);
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

    case STSetplayFork:
      result = setplay_fork_attack(si,n);
      break;

    case STAttackAdapter:
      result = attack_adapter_attack(si,n);
      break;

    case STDefenseAdapter:
      result = defense_adapter_attack(si,n);
      break;

    case STHelpAdapter:
      result = help_adapter_attack(si,n);
      break;

    case STAnd:
      result = and_attack(si,n);
      break;

    case STNot:
      result = not_attack(si,n);
      break;

    case STMoveInverter:
      result = move_inverter_attack(si,n);
      break;

    case STMaxSolutionsInitialiser:
      result = maxsolutions_initialiser_attack(si,n);
      break;

    case STStopOnShortSolutionsInitialiser:
      result = stoponshortsolutions_initialiser_attack(si,n);
      break;

    case STIllegalSelfcheckWriter:
      result = illegal_selfcheck_writer_attack(si,n);
      break;

    case STEndOfPhaseWriter:
      result = end_of_phase_writer_attack(si,n);
      break;

    case STOutputPlaintextMoveInversionCounter:
      result = output_plaintext_move_inversion_counter_attack(si,n);
      break;

    case STOutputPlaintextLineEndOfIntroSeriesMarker:
      result = output_plaintext_line_end_of_intro_series_marker_attack(si,n);
      break;

    case STPiecesParalysingMateFilter:
      result = paralysing_mate_filter_attack(si,n);
      break;

    case STPiecesParalysingStalemateSpecial:
      result = paralysing_stalemate_special_attack(si,n);
      break;

    case STAmuMateFilter:
      result = amu_mate_filter_attack(si,n);
      break;

    case STUltraschachzwangGoalFilter:
      result = ultraschachzwang_goal_filter_attack(si,n);
      break;

    case STCirceSteingewinnFilter:
      result = circe_steingewinn_filter_attack(si,n);
      break;

    case STCirceCircuitSpecial:
      result = circe_circuit_special_attack(si,n);
      break;

    case STCirceExchangeSpecial:
      result = circe_exchange_special_attack(si,n);
      break;

    case STAnticirceTargetSquareFilter:
      result = anticirce_target_square_filter_attack(si,n);
      break;

    case STAnticirceCircuitSpecial:
      result = anticirce_circuit_special_attack(si,n);
      break;

    case STAnticirceExchangeSpecial:
      result = anticirce_exchange_special_attack(si,n);
      break;

    case STAnticirceExchangeFilter:
      result = anticirce_exchange_filter_attack(si,n);
      break;

    case STTemporaryHackFork:
      result = attack(slices[si].next1,length_unspecified);
      break;

    case STGoalTargetReachedTester:
      result = goal_target_reached_tester_attack(si,n);
      break;

    case STGoalCheckReachedTester:
      result = goal_check_reached_tester_attack(si,n);
      break;

    case STGoalCaptureReachedTester:
      result = goal_capture_reached_tester_attack(si,n);
      break;

    case STGoalSteingewinnReachedTester:
      result = goal_steingewinn_reached_tester_attack(si,n);
      break;

    case STGoalEnpassantReachedTester:
      result = goal_enpassant_reached_tester_attack(si,n);
      break;

    case STGoalDoubleMateReachedTester:
      result = goal_doublemate_reached_tester_attack(si,n);
      break;

    case STGoalCounterMateReachedTester:
      result = goal_countermate_reached_tester_attack(si,n);
      break;

    case STGoalCastlingReachedTester:
      result = goal_castling_reached_tester_attack(si,n);
      break;

    case STGoalCircuitReachedTester:
      result = goal_circuit_reached_tester_attack(si,n);
      break;

    case STGoalExchangeReachedTester:
      result = goal_exchange_reached_tester_attack(si,n);
      break;

    case STGoalCircuitByRebirthReachedTester:
      result = goal_circuit_by_rebirth_reached_tester_attack(si,n);
      break;

    case STGoalExchangeByRebirthReachedTester:
      result = goal_exchange_by_rebirth_reached_tester_attack(si,n);
      break;

    case STGoalProofgameReachedTester:
    case STGoalAToBReachedTester:
      result = goal_proofgame_reached_tester_attack(si,n);
      break;

    case STGoalImmobileReachedTester:
      result = goal_immobile_reached_tester_attack(si,n);
      break;

    case STImmobilityTester:
      result = immobility_tester_attack(si,n);
      break;

    case STMaffImmobilityTesterKing:
      result = maff_immobility_tester_king_attack(si,n);
      break;

    case STOWUImmobilityTesterKing:
      result = owu_immobility_tester_king_attack(si,n);
      break;

    case STGoalNotCheckReachedTester:
      result = goal_notcheck_reached_tester_attack(si,n);
      break;

    case STGoalAnyReachedTester:
      result = goal_any_reached_tester_attack(si,n);
      break;

    case STGoalChess81ReachedTester:
      result = goal_chess81_reached_tester_attack(si,n);
      break;

    case STPiecesParalysingMateFilterTester:
      result = paralysing_mate_filter_tester_attack(si,n);
      break;

    case STBlackChecks:
      result = blackchecks_attack(si,n);
      break;

    case STExtinctionRememberThreatened:
      result = extinction_remember_threatened_attack(si,n);
      break;

    case STExtinctionTester:
      result = extinction_tester_attack(si,n);
      break;

    case STSingleBoxType1LegalityTester:
      result = singlebox_type1_legality_tester_attack(si,n);
      break;

    case STSingleBoxType2LegalityTester:
      result = singlebox_type2_legality_tester_attack(si,n);
      break;

    case STPlaySuppressor:
      result = play_suppressor_attack(si,n);
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
