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
#include "solving/castling.h"
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
#include "solving/en_passant.h"
#include "solving/moving_pawn_promotion.h"
#include "solving/post_move_iteration.h"
#include "solving/move_effect_journal.h"
#include "conditions/amu/attack_counter.h"
#include "conditions/bgl.h"
#include "conditions/blackchecks.h"
#include "conditions/extinction.h"
#include "conditions/exclusive.h"
#include "conditions/singlebox/type1.h"
#include "conditions/singlebox/type2.h"
#include "conditions/singlebox/type3.h"
#include "conditions/ohneschach/legality_tester.h"
#include "conditions/patience.h"
#include "conditions/isardam.h"
#include "conditions/masand.h"
#include "conditions/sat.h"
#include "conditions/dynasty.h"
#include "conditions/ultraschachzwang/legality_tester.h"
#include "conditions/oscillating_kings.h"
#include "conditions/messigny.h"
#include "conditions/actuated_revolving_centre.h"
#include "conditions/actuated_revolving_board.h"
#include "conditions/republican.h"
#include "conditions/circe/capture_fork.h"
#include "conditions/circe/rebirth_handler.h"
#include "conditions/circe/assassin.h"
#include "conditions/circe/frischauf.h"
#include "conditions/circe/cage.h"
#include "conditions/circe/kamikaze.h"
#include "conditions/circe/parrain.h"
#include "conditions/circe/volage.h"
#include "conditions/circe/promotion.h"
#include "conditions/circe/super.h"
#include "conditions/circe/april.h"
#include "conditions/circe/king_rebirth_avoider.h"
#include "conditions/anticirce/rebirth_handler.h"
#include "conditions/anticirce/super.h"
#include "conditions/sentinelles.h"
#include "conditions/duellists.h"
#include "conditions/haunted_chess.h"
#include "conditions/ghost_chess.h"
#include "conditions/kobul.h"
#include "conditions/andernach.h"
#include "conditions/antiandernach.h"
#include "conditions/chameleon_pursuit.h"
#include "conditions/norsk.h"
#include "conditions/protean.h"
#include "conditions/einstein/einstein.h"
#include "conditions/einstein/reverse.h"
#include "conditions/einstein/anti.h"
#include "conditions/traitor.h"
#include "conditions/volage.h"
#include "conditions/magic_square.h"
#include "conditions/tibet.h"
#include "conditions/degradierung.h"
#include "conditions/phantom.h"
#include "conditions/marscirce/anti.h"
#include "conditions/line_chameleon.h"
#include "conditions/haan.h"
#include "conditions/castling_chess.h"
#include "conditions/exchange_castling.h"
#include "conditions/transmuting_kings/super.h"
#include "conditions/imitator.h"
#include "conditions/football.h"
#include "conditions/castling_chess.h"
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
#include "pieces/attributes/neutral/initialiser.h"
#include "pieces/attributes/neutral/half.h"
#include "pieces/attributes/hurdle_colour_changing.h"
#include "pieces/attributes/magic.h"
#include "pieces/attributes/chameleon.h"
#include "pieces/attributes/kamikaze/kamikaze.h"
#include "debugging/trace.h"
#include "debugging/measure.h"

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

    case STCastlingRightsAdjuster:
      result = castling_rights_adjuster_defend(si,n);
      break;

    case STMovingPawnPromoter:
      result = moving_pawn_promoter_defend(si,n);
      break;

    case STForEachMove:
      result = for_each_move_defend(si,n);
      break;

    case STNullMovePlayer:
      result = null_move_player_defend(si,n);
      break;

    case STPostMoveIterationInitialiser:
      result = post_move_iteration_initialiser_defend(si,n);
      break;

    case STMoveEffectJournalReplayer:
      result = move_effect_journal_redoer_defend(si,n);
      break;

    case STMoveEffectJournalUndoer:
      result = move_effect_journal_undoer_defend(si,n);
      break;

    case STMessignyMovePlayer:
      result = messigny_move_player_defend(si,n);
      break;

    case STCastlingPlayer:
      result = castling_player_defend(si,n);
      break;

    case STMovePlayer:
      result = move_player_defend(si,n);
      break;

    case STEnPassantAdjuster:
      result = en_passant_adjuster_defend(si,n);
      break;

    case STPhantomChessEnPassantAdjuster:
      result = phantom_en_passant_adjuster_defend(si,n);
      break;

    case STAntiMarsCirceEnPassantAdjuster:
      result = antimars_en_passant_adjuster_defend(si,n);
      break;

    case STKamikazeCapturingPieceRemover:
      result = kamikaze_capturing_piece_remover_defend(si,n);
      break;

    case STHaanChessHoleInserter:
      result = haan_chess_hole_inserter_defend(si,n);
      break;

    case STCastlingChessMovePlayer:
      result = castling_chess_move_player_defend(si,n);
      break;

    case STExchangeCastlingMovePlayer:
      result = exchange_castling_move_player_defend(si,n);
      break;

    case STSuperTransmutingKingTransmuter:
      result = supertransmuting_kings_transmuter_defend(si,n);
      break;

    case STAMUAttackCounter:
      result = amu_attack_counter_defend(si,n);
      break;

    case STMutualCastlingRightsAdjuster:
      result = mutual_castling_rights_adjuster_defend(si,n);
      break;

    case STImitatorMover:
      result = imitator_mover_defend(si,n);
      break;

    case STMovingPawnToImitatorPromoter:
      result = moving_pawn_to_imitator_promoter_defend(si,n);
      break;

    case STMovePlayed:
      result = move_played_defend(si,n);
      break;

#if defined(DOTRACE)
    case STMoveTracer:
      result = move_tracer_defend(si,n);
      break;
#endif

#if defined(DOMEASURE)
    case STMoveCounter:
      result = move_counter_defend(si,n);
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

    case STMasandRecolorer:
      result = masand_recolorer_defend(si,n);
      break;

    case STActuatedRevolvingCentre:
      result = actuated_revolving_centre_defend(si,n);
      break;

    case STActuatedRevolvingBoard:
      result = actuated_revolving_board_defend(si,n);
      break;

    case STRepublicanKingPlacer:
      result = republican_king_placer_defend(si,n);
      break;

    case STCirceParrainRebirthHandler:
      result = circe_parrain_rebirth_handler_defend(si,n);
      break;

    case STCirceKingRebirthAvoider:
      result = circe_king_rebirth_avoider_defend(si,n);
      break;

    case STCirceCaptureFork:
      result = circe_capture_fork_defend(si,n);
      break;

    case STCirceRebirthHandler:
      result = circe_rebirth_handler_defend(si,n);
      break;

    case STAprilAprilFork:
      result = april_chess_fork_defend(si,n);
      break;

    case STSuperCirceNoRebirthFork:
      result = supercirce_no_rebirth_fork_defend(si,n);
      break;

    case STSuperCirceRebirthHandler:
      result = supercirce_rebirth_handler_defend(si,n);
      break;

    case STCirceRebirthPromoter:
      result = circe_promoter_defend(si,n);
      break;

    case STCirceVolageRecolorer:
      result = circe_volage_recolorer_defend(si,n);
      break;

    case STCirceKamikazeRebirthHandler:
      result = circe_kamikaze_rebirth_handler_defend(si,n);
      break;

    case STCirceCageNoCageFork:
      result = circe_cage_no_cage_fork_defend(si,n);
      break;

    case STCirceCageCageTester:
      result = circe_cage_cage_tester_defend(si,n);
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

    case STSingleBoxType3PawnPromoter:
      result = singlebox_type3_pawn_promoter_defend(si,n);
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

    case STCirceAssassinRebirth:
      result = circe_assassin_rebirth_defend(si,n);
      break;

    case STKingCaptureAvoider:
      result = king_capture_avoider_defend(si,n);
      break;

    case STPatienceChessLegalityTester:
      result = patience_chess_legality_tester_defend(si,n);
      break;

    case STStrictSATUpdater:
      result = strict_sat_updater_defend(si,n);
      break;

    case STDynastyKingSquareUpdater:
      result = dynasty_king_square_updater_defend(si,n);
      break;

    case STHurdleColourChanger:
      result = hurdle_colour_changer_defend(si,n);
      break;

    case STKingOscillator:
      result = king_oscillator_defend(si,n);
      break;

    case STPiecesNeutralInitialiser:
      result = neutral_initialiser_defend(si,n);
      break;

    case STPiecesNeutralRetractingRecolorer:
      result = neutral_retracting_recolorer_defend(si,n);
      break;

    case STPiecesNeutralReplayingRecolorer:
      result = neutral_replaying_recolorer_defend(si,n);
      break;

    case STSentinellesInserter:
      result = sentinelles_inserter_defend(si,n);
      break;

    case STMagicPiecesRecolorer:
      result = magic_pieces_recolorer_defend(si,n);
      break;

    case STHauntedChessGhostSummoner:
      result = haunted_chess_ghost_summoner_defend(si,n);
      break;

    case STHauntedChessGhostRememberer:
      result = haunted_chess_ghost_rememberer_defend(si,n);
      break;

    case STGhostChessGhostSummoner:
      result = ghost_chess_ghost_summoner_defend(si,n);
      break;

    case STGhostChessGhostRememberer:
      result = ghost_chess_ghost_rememberer_defend(si,n);
      break;

    case STAndernachSideChanger:
      result = andernach_side_changer_defend(si,n);
      break;

    case STAntiAndernachSideChanger:
      result = antiandernach_side_changer_defend(si,n);
      break;

    case STChameleonPursuitSideChanger:
      result = chameleon_pursuit_side_changer_defend(si,n);
      break;

    case STNorskArrivingAdjuster:
      result = norsk_arriving_adjuster_defend(si,n);
      break;

    case STProteanPawnAdjuster:
      result = protean_pawn_adjuster_defend(si,n);
      break;

    case STEinsteinArrivingAdjuster:
      result = einstein_moving_adjuster_defend(si,n);
      break;

    case STReverseEinsteinArrivingAdjuster:
      result = reverse_einstein_moving_adjuster_defend(si,n);
      break;

    case STAntiEinsteinArrivingAdjuster:
      result = anti_einstein_moving_adjuster_defend(si,n);
      break;

    case STTraitorSideChanger:
      result = traitor_side_changer_defend(si,n);
      break;

    case STVolageSideChanger:
      result = volage_side_changer_defend(si,n);
      break;

    case STMagicSquareSideChanger:
      result = magic_square_side_changer_defend(si,n);
      break;

    case STTibetSideChanger:
      result = tibet_defend(si,n);
      break;

    case STDoubleTibetSideChanger:
      result = double_tibet_defend(si,n);
      break;

    case STDegradierungDegrader:
      result = degradierung_degrader_defend(si,n);
      break;

    case STPromoteMovingIntoChameleon:
      result = chameleon_promote_moving_into_defend(si,n);
      break;

    case STPromoteCirceRebornIntoChameleon:
      result = chameleon_promote_circe_reborn_into_defend(si,n);
      break;

    case STPromoteAnticirceRebornIntoChameleon:
      result = chameleon_promote_anticirce_reborn_into_defend(si,n);
      break;

    case STChameleonArrivingAdjuster:
      result = chameleon_arriving_adjuster_defend(si,n);
      break;

    case STLineChameleonArrivingAdjuster:
      result = line_chameleon_arriving_adjuster_defend(si,n);
      break;

    case STFrischaufPromoteeMarker:
      result = frischauf_promotee_marker_defend(si,n);
      break;

    case STPiecesHalfNeutralRecolorer:
      result = half_neutral_recolorer_defend(si,n);
      break;

    case STKobulKingSubstitutor:
      result = kobul_king_substitutor_defend(si,n);
      break;

    case STDuellistsRememberDuellist:
      result = duellists_remember_duellist_defend(si,n);
      break;

    case STSingleboxType2LatentPawnSelector:
      result = singlebox_type2_latent_pawn_selector_defend(si,n);
      break;

    case STSingleboxType2LatentPawnPromoter:
      result = singlebox_type2_latent_pawn_promoter_defend(si,n);
      break;

    case STAnticirceRebirthHandler:
      result = anticirce_rebirth_handler_defend(si,n);
      break;

    case STAnticirceRebornPromoter:
      result = anticirce_reborn_promoter_defend(si,n);
      break;

    case STAntisupercirceRebirthHandler:
      result = antisupercirce_rebirth_handler_defend(si,n);
      break;

    case STFootballChessSubsitutor:
      result = football_chess_substitutor_defend(si,n);
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
