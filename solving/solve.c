#include "solving/solve.h"
#include "stipulation/fork.h"
#include "pyproof.h"
#include "conditions/amu/mate_filter.h"
#include "conditions/amu/attack_counter.h"
#include "conditions/anticirce/cheylan.h"
#include "conditions/anticirce/capture_fork.h"
#include "conditions/anticirce/circuit_special.h"
#include "conditions/anticirce/couscous.h"
#include "conditions/anticirce/exchange_filter.h"
#include "conditions/anticirce/exchange_special.h"
#include "conditions/anticirce/target_square_filter.h"
#include "conditions/anticirce/relaxed.h"
#include "conditions/anticirce/strict.h"
#include "conditions/anticirce/promotion.h"
#include "conditions/anticirce/magic_square.h"
#include "conditions/bgl.h"
#include "conditions/blackchecks.h"
#include "conditions/koeko/koeko.h"
#include "conditions/koeko/contact_grid.h"
#include "conditions/koeko/anti.h"
#include "conditions/koeko/new.h"
#include "conditions/circe/chameleon.h"
#include "conditions/circe/circuit_by_rebirth_special.h"
#include "conditions/circe/clone.h"
#include "conditions/circe/couscous.h"
#include "conditions/circe/exchange_by_rebirth_special.h"
#include "conditions/circe/steingewinn_filter.h"
#include "conditions/circe/assassin.h"
#include "conditions/circe/frischauf.h"
#include "conditions/circe/super.h"
#include "conditions/circe/april.h"
#include "conditions/circe/turncoats.h"
#include "conditions/circe/takemake.h"
#include "conditions/exclusive.h"
#include "conditions/ohneschach.h"
#include "conditions/maff/immobility_tester.h"
#include "conditions/owu/immobility_tester.h"
#include "conditions/immune.h"
#include "conditions/provocateurs.h"
#include "conditions/lortap.h"
#include "conditions/patrol.h"
#include "conditions/monochrome.h"
#include "conditions/bichrome.h"
#include "conditions/ultraschachzwang/legality_tester.h"
#include "conditions/singlebox/type1.h"
#include "conditions/singlebox/type2.h"
#include "conditions/singlebox/type3.h"
#include "conditions/patience.h"
#include "conditions/isardam.h"
#include "conditions/sat.h"
#include "conditions/dynasty.h"
#include "conditions/masand.h"
#include "conditions/oscillating_kings.h"
#include "conditions/messigny.h"
#include "conditions/actuated_revolving_centre.h"
#include "conditions/actuated_revolving_board.h"
#include "conditions/republican.h"
#include "conditions/circe/capture_fork.h"
#include "conditions/circe/circe.h"
#include "conditions/circe/cage.h"
#include "conditions/circe/double_agents.h"
#include "conditions/circe/kamikaze.h"
#include "conditions/circe/parrain.h"
#include "conditions/circe/volage.h"
#include "conditions/circe/promotion.h"
#include "conditions/anticirce/anticirce.h"
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
#include "conditions/einstein/en_passant.h"
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
#include "conditions/mummer.h"
#include "conditions/woozles.h"
#include "conditions/nocapture.h"
#include "conditions/nopromotion.h"
#include "conditions/geneva.h"
#include "conditions/edgemover.h"
#include "conditions/grid.h"
#include "conditions/take_and_make.h"
#include "conditions/superguards.h"
#include "conditions/wormhole.h"
#include "conditions/backhome.h"
#include "optimisations/hash.h"
#include "optimisations/keepmating.h"
#include "optimisations/count_nr_opponent_moves/opponent_moves_counter.h"
#include "optimisations/count_nr_opponent_moves/prioriser.h"
#include "optimisations/goals/castling/filter.h"
#include "optimisations/goals/enpassant/filter.h"
#include "optimisations/goals/enpassant/remove_non_reachers.h"
#include "optimisations/goals/castling/remove_non_reachers.h"
#include "optimisations/goals/chess81/remove_non_reachers.h"
#include "optimisations/goals/capture/remove_non_reachers.h"
#include "optimisations/goals/target/remove_non_reachers.h"
#include "optimisations/goals/mate/neutralretractable.h"
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
#include "optimisations/killer_move/final_defense_move.h"
#include "optimisations/killer_move/prioriser.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_move_generator.h"
#include "optimisations/orthodox_mating_moves/king_contact_move_generator.h"
#include "optimisations/ohneschach/non_checking_first.h"
#include "optimisations/ohneschach/stop_if_check.h"
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
#include "output/plaintext/ohneschach_detect_undecidable_goal.h"
#include "output/plaintext/illegal_selfcheck_writer.h"
#include "output/plaintext/goal_writer.h"
#include "output/plaintext/move_inversion_counter.h"
#include "output/plaintext/line/line_writer.h"
#include "output/plaintext/line/end_of_intro_series_marker.h"
#include "output/plaintext/line/exclusive.h"
#include "output/plaintext/tree/end_of_solution_writer.h"
#include "output/plaintext/tree/check_writer.h"
#include "output/plaintext/tree/key_writer.h"
#include "output/plaintext/tree/move_writer.h"
#include "output/plaintext/tree/refutation_writer.h"
#include "output/plaintext/tree/refuting_variation_writer.h"
#include "output/plaintext/tree/threat_writer.h"
#include "output/plaintext/tree/try_writer.h"
#include "output/plaintext/tree/zugzwang_writer.h"
#include "output/plaintext/tree/exclusive.h"
#include "pieces/attributes/paralysing/mate_filter.h"
#include "pieces/attributes/paralysing/stalemate_special.h"
#include "pieces/attributes/paralysing/remove_captures.h"
#include "pieces/attributes/neutral/half.h"
#include "pieces/attributes/hurdle_colour_changing.h"
#include "pieces/attributes/magic.h"
#include "pieces/attributes/chameleon.h"
#include "pieces/attributes/jigger.h"
#include "pieces/attributes/uncapturable.h"
#include "pieces/attributes/kamikaze/kamikaze.h"
#include "solving/avoid_unsolvable.h"
#include "solving/battle_play/continuation.h"
#include "solving/battle_play/min_length_guard.h"
#include "solving/battle_play/min_length_optimiser.h"
#include "solving/battle_play/threat.h"
#include "solving/battle_play/try.h"
#include "solving/battle_play/threat.h"
#include "solving/castling.h"
#include "solving/capture_counter.h"
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
#include "solving/single_piece_move_generator.h"
#include "solving/trivial_end_filter.h"
#include "pieces/pawns/en_passant.h"
#include "solving/moving_pawn_promotion.h"
#include "solving/selfcheck_guard.h"
#include "solving/post_move_iteration.h"
#include "solving/move_effect_journal.h"
#include "solving/king_capture_avoider.h"
#include "stipulation/has_solution_type.h"
#include "solving/recursion_stopper.h"
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
#include "stipulation/dummy_move.h"
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
#include "debugging/measure.h"

#include <assert.h>

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceEnumerator(slice_type,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STThreatSolver:
      result = threat_solver_solve(si,n);
      break;

    case STDummyMove:
      result = dummy_move_solve(si,n);
      break;

    case STThreatCollector:
      result = threat_collector_solve(si,n);
      break;

    case STThreatEnforcer:
      result = threat_enforcer_solve(si,n);
      break;

    case STThreatDefeatedTester:
      result = threat_defeated_tester_solve(si,n);
      break;

    case STThreatWriter:
      result = threat_writer_solve(si,n);
      break;

    case STZugzwangWriter:
      result = zugzwang_writer_solve(si,n);
      break;

    case STKeyWriter:
      result = key_writer_solve(si,n);
      break;

    case STTryWriter:
      result = try_writer_solve(si,n);
      break;

    case STRefutationsAllocator:
      result = refutations_allocator_solve(si,n);
      break;

    case STRefutationsSolver:
      result = refutations_solver_solve(si,n);
      break;

    case STRefutationsIntroWriter:
      result = refutations_intro_writer_solve(si,n);
      break;

    case STRefutationsAvoider:
      result = refutations_avoider_solve(si,n);
      break;

    case STRefutationsFilter:
      result = refutations_filter_solve(si,n);
      break;

    case STRefutingVariationWriter:
      result = refuting_variation_writer_solve(si,n);
      break;

    case STMoveWriter:
      result = move_writer_solve(si,n);
      break;

    case STTrivialEndFilter:
      result = trivial_end_filter_solve(si,n);
      break;

    case STNoShortVariations:
      result = no_short_variations_solve(si,n);
      break;

    case STOr:
      result = or_solve(si,n);
      break;

    case STFindShortest:
      result = find_shortest_solve(si,n);
      break;

    case STMoveGenerator:
      result = move_generator_solve(si,n);
      break;

    case STForEachAttack:
      result = for_each_attack_solve(si,n);
      break;

    case STFindAttack:
      result = find_attack_solve(si,n);
      break;

    case STForEachDefense:
      result = for_each_defense_solve(si,n);
      break;

    case STFindDefense:
      result = find_defense_solve(si,n);
      break;

    case STNullMovePlayer:
      result = null_move_player_solve(si,n);
      break;

    case STPostMoveIterationInitialiser:
      result = post_move_iteration_initialiser_solve(si,n);
      break;

    case STMoveEffectJournalUndoer:
      result = move_effect_journal_undoer_solve(si,n);
      break;

    case STMessignyMovePlayer:
      result = messigny_move_player_solve(si,n);
      break;

    case STCastlingPlayer:
      result = castling_player_solve(si,n);
      break;

    case STMovePlayer:
      result = move_player_solve(si,n);
      break;

    case STEnPassantAdjuster:
      result = en_passant_adjuster_solve(si,n);
      break;

    case STEinsteinEnPassantAdjuster:
      result = einstein_en_passant_adjuster_solve(si,n);
      break;

    case STMovingPawnPromoter:
      result = moving_pawn_promoter_solve(si,n);
      break;

    case STPhantomChessEnPassantAdjuster:
      result = phantom_en_passant_adjuster_solve(si,n);
      break;

    case STAntiMarsCirceEnPassantAdjuster:
      result = antimars_en_passant_adjuster_solve(si,n);
      break;

    case STKamikazeCapturingPieceRemover:
      result = kamikaze_capturing_piece_remover_solve(si,n);
      break;

    case STHaanChessHoleInserter:
      result = haan_chess_hole_inserter_solve(si,n);
      break;

    case STCastlingChessMovePlayer:
      result = castling_chess_move_player_solve(si,n);
      break;

    case STExchangeCastlingMovePlayer:
      result = exchange_castling_move_player_solve(si,n);
      break;

    case STSuperTransmutingKingTransmuter:
      result = supertransmuting_kings_transmuter_solve(si,n);
      break;

    case STAMUAttackCounter:
      result = amu_attack_counter_solve(si,n);
      break;

    case STMutualCastlingRightsAdjuster:
      result = mutual_castling_rights_adjuster_solve(si,n);
      break;

    case STImitatorMover:
      result = imitator_mover_solve(si,n);
      break;

    case STImitatorRemoveIllegalMoves:
      result = imitator_remove_illegal_moves_solve(si,n);
      break;

    case STMovingPawnToImitatorPromoter:
      result = moving_pawn_to_imitator_promoter_solve(si,n);
      break;

    case STAttackPlayed:
      result = attack_played_solve(si,n);
      break;

#if defined(DOTRACE)
    case STMoveTracer:
      result = move_tracer_solve(si,n);
      break;
#endif

#if defined(DOMEASURE)
    case STMoveCounter:
      result = move_counter_solve(si,n);
      break;
#endif

    case STOrthodoxMatingMoveGenerator:
      result = orthodox_mating_move_generator_solve(si,n);
      break;

    case STOrthodoxMatingKingContactGenerator:
      result = orthodox_mating_king_contact_generator_solve(si,n);
      break;

    case STDeadEnd:
    case STDeadEndGoal:
      result = dead_end_solve(si,n);
      break;

    case STMinLengthOptimiser:
      result = min_length_optimiser_solve(si,n);
      break;

    case STForkOnRemaining:
      result = fork_on_remaining_solve(si,n);
      break;

    case STHashOpener:
      result = hash_opener_solve(si,n);
      break;

    case STAttackHashed:
      result = attack_hashed_solve(si,n);
      break;

    case STAttackHashedTester:
      result = attack_hashed_tester_solve(si,n);
      break;

    case STHelpHashed:
      result = help_hashed_solve(si,n);
      break;

    case STHelpHashedTester:
      result = help_hashed_tester_solve(si,n);
      break;

    case STEndOfBranch:
    case STEndOfBranchForced:
    case STEndOfBranchTester:
      result = end_of_branch_solve(si,n);
      break;

    case STEndOfBranchGoal:
    case STEndOfBranchGoalImmobile:
    case STEndOfBranchGoalTester:
      result = end_of_branch_goal_solve(si,n);
      break;

    case STGoalReachedTester:
      result = goal_reached_tester_solve(si,n);
      break;

    case STAvoidUnsolvable:
      result = avoid_unsolvable_solve(si,n);
      break;

    case STResetUnsolvable:
      result = reset_unsolvable_solve(si,n);
      break;

    case STLearnUnsolvable:
      result = learn_unsolvable_solve(si,n);
      break;

    case STConstraintSolver:
    case STConstraintTester:
    case STGoalConstraintTester:
      result = constraint_solve(si,n);
      break;

    case STSelfCheckGuard:
    case STExclusiveChessGoalReachingMoveCounterSelfCheckGuard:
      result = selfcheck_guard_solve(si,n);
      break;

    case STKeepMatingFilter:
      result = keepmating_filter_solve(si,n);
      break;

    case STOutputPlaintextTreeCheckWriter:
      result = output_plaintext_tree_check_writer_solve(si,n);
      break;

    case STRefutationWriter:
      result = refutation_writer_solve(si,n);
      break;

    case STDoubleMateFilter:
      result = doublemate_filter_solve(si,n);
      break;

    case STCounterMateFilter:
      result = countermate_filter_solve(si,n);
      break;

    case STEnPassantFilter:
      result = enpassant_filter_solve(si,n);
      break;

    case STEnPassantRemoveNonReachers:
      result = enpassant_remove_non_reachers_solve(si,n);
      break;

    case STCastlingFilter:
      result = castling_filter_solve(si,n);
      break;

    case STCastlingRemoveNonReachers:
      result = castling_remove_non_reachers_solve(si,n);
      break;

    case STPrerequisiteOptimiser:
      result = goal_prerequisite_optimiser_solve(si,n);
      break;

    case STChess81RemoveNonReachers:
      result = chess81_remove_non_reachers_solve(si,n);
      break;

    case STCaptureRemoveNonReachers:
      result = capture_remove_non_reachers_solve(si,n);
      break;

    case STTargetRemoveNonReachers:
      result = target_remove_non_reachers_solve(si,n);
      break;

    case STMateRemoveRetractable:
      result = mate_remove_retractable_solve(si,n);
      break;

    case STOutputPlaintextGoalWriter:
      result = output_plaintext_goal_writer_solve(si,n);
      break;

    case STOutputPlaintextLineLineWriter:
      result = output_plaintext_line_line_writer_solve(si,n);
      break;

    case STBGLFilter:
      result = bgl_filter_solve(si,n);
      break;

    case STMasandRecolorer:
      result = masand_recolorer_solve(si,n);
      break;

    case STActuatedRevolvingCentre:
      result = actuated_revolving_centre_solve(si,n);
      break;

    case STActuatedRevolvingBoard:
      result = actuated_revolving_board_solve(si,n);
      break;

    case STRepublicanKingPlacer:
      result = republican_king_placer_solve(si,n);
      break;

    case STRepublicanType1DeadEnd:
      result = republican_type1_dead_end_solve(si,n);
      break;

    case STCirceCaptureFork:
      result = circe_capture_fork_solve(si,n);
      break;

    case STCirceDetermineRebornPiece:
      result = circe_determine_reborn_piece_solve(si,n);
      break;

    case STCirceCloneDetermineRebornPiece:
      result = circe_clone_determine_reborn_piece_solve(si,n);
      break;

    case STCirceDoubleAgentsAdaptRebornPiece:
      result = circe_doubleagents_adapt_reborn_piece_solve(si,n);
      break;

    case STChameleonCirceAdaptRebornPiece:
      result = chameleon_circe_adapt_reborn_piece_solve(si,n);
      break;

    case STAntiEinsteinDetermineRebornPiece:
      result = anti_einstein_determine_reborn_piece_solve(si,n);
      break;

    case STCirceDetermineRelevantPiece:
      result = circe_determine_relevant_piece_solve(si,n);
      break;

    case STCirceCouscousDetermineRelevantPiece:
      result = circe_couscous_determine_relevant_piece_solve(si,n);
      break;

    case STCirceDetermineRebirthSquare:
      result = circe_determine_rebirth_square_solve(si,n);
      break;

    case STTakeMakeCirceDetermineRebirthSquares:
      result = take_make_circe_determine_rebirth_squares_solve(si,n);
      break;

    case STTakeMakeCirceCollectRebirthSquares:
      result = take_make_circe_collect_rebirth_squares_solve(si,n);
      break;

    case STContactGridAvoidCirceRebirth:
      result = contact_grid_avoid_circe_rebirth(si,n);
      break;

    case STCircePlaceReborn:
      result = circe_place_reborn_solve(si,n);
      break;

    case STAprilAprilFork:
      result = april_chess_fork_solve(si,n);
      break;

    case STSuperCirceNoRebirthFork:
      result = supercirce_no_rebirth_fork_solve(si,n);
      break;

    case STSuperCirceRebirthHandler:
      result = supercirce_rebirth_handler_solve(si,n);
      break;

    case STCirceRebirthPromoter:
      result = circe_promoter_solve(si,n);
      break;

    case STCirceVolageRecolorer:
      result = circe_volage_recolorer_solve(si,n);
      break;

    case STCirceParrainDetermineRebirth:
      result = circe_parrain_determine_rebirth_solve(si,n);
      break;

    case STAnticircePlaceRebornRelaxed:
      result = anticirce_place_reborn_relaxed_solve(si,n);
      break;

    case STCirceCageNoCageFork:
      result = circe_cage_no_cage_fork_solve(si,n);
      break;

    case STCirceCageCageTester:
      result = circe_cage_cage_tester_solve(si,n);
      break;

    case STSentinellesInserter:
      result = sentinelles_inserter_solve(si,n);
      break;

    case STMagicViewsInitialiser:
      result = magic_views_initialiser_solve(si,n);
      break;

    case STMagicPiecesRecolorer:
      result = magic_pieces_recolorer_solve(si,n);
      break;

    case STHauntedChessGhostSummoner:
      result = haunted_chess_ghost_summoner_solve(si,n);
      break;

    case STHauntedChessGhostRememberer:
      result = haunted_chess_ghost_rememberer_solve(si,n);
      break;

    case STGhostChessGhostRememberer:
      result = ghost_chess_ghost_rememberer_solve(si,n);
      break;

    case STAndernachSideChanger:
      result = andernach_side_changer_solve(si,n);
      break;

    case STAntiAndernachSideChanger:
      result = antiandernach_side_changer_solve(si,n);
      break;

    case STChameleonPursuitSideChanger:
      result = chameleon_pursuit_side_changer_solve(si,n);
      break;

    case STNorskRemoveIllegalCaptures:
      result = norsk_remove_illegal_captures_solve(si,n);
      break;

    case STNorskArrivingAdjuster:
      result = norsk_arriving_adjuster_solve(si,n);
      break;

    case STProteanPawnAdjuster:
      result = protean_pawn_adjuster_solve(si,n);
      break;

    case STEinsteinArrivingAdjuster:
      result = einstein_moving_adjuster_solve(si,n);
      break;

    case STReverseEinsteinArrivingAdjuster:
      result = reverse_einstein_moving_adjuster_solve(si,n);
      break;

    case STAntiEinsteinArrivingAdjuster:
      result = anti_einstein_moving_adjuster_solve(si,n);
      break;

    case STTraitorSideChanger:
      result = traitor_side_changer_solve(si,n);
      break;

    case STVolageSideChanger:
      result = volage_side_changer_solve(si,n);
      break;

    case STMagicSquareSideChanger:
    case STMagicSquareType2SideChanger:
      result = magic_square_side_changer_solve(si,n);
      break;

    case STMagicSquareType2AnticirceRelevantSideAdapter:
      result = magic_square_anticirce_relevant_side_adapter_solve(si,n);
      break;

    case STCirceTraitorSideChanger:
      result = circe_turncoats_side_changer_solve(si,n);
      break;

    case STTibetSideChanger:
      result = tibet_solve(si,n);
      break;

    case STDoubleTibetSideChanger:
      result = double_tibet_solve(si,n);
      break;

    case STDegradierungDegrader:
      result = degradierung_degrader_solve(si,n);
      break;

    case STPromoteMovingIntoChameleon:
      result = chameleon_promote_moving_into_solve(si,n);
      break;

    case STPromoteCirceRebornIntoChameleon:
      result = chameleon_promote_circe_reborn_into_solve(si,n);
      break;

    case STPromoteAnticirceRebornIntoChameleon:
      result = chameleon_promote_anticirce_reborn_into_solve(si,n);
      break;

    case STChameleonArrivingAdjuster:
      result = chameleon_arriving_adjuster_solve(si,n);
      break;

    case STChameleonChessArrivingAdjuster:
      result = chameleon_chess_arriving_adjuster_solve(si,n);
      break;

    case STLineChameleonArrivingAdjuster:
      result = line_chameleon_arriving_adjuster_solve(si,n);
      break;

    case STFrischaufPromoteeMarker:
      result = frischauf_promotee_marker_solve(si,n);
      break;

    case STPiecesHalfNeutralRecolorer:
      result = half_neutral_recolorer_solve(si,n);
      break;

    case STKobulKingSubstitutor:
      result = kobul_king_substitutor_solve(si,n);
      break;

    case STDuellistsRememberDuellist:
      result = duellists_remember_duellist_solve(si,n);
      break;

    case STSingleboxType2LatentPawnSelector:
      result = singlebox_type2_latent_pawn_selector_solve(si,n);
      break;

    case STSingleboxType2LatentPawnPromoter:
      result = singlebox_type2_latent_pawn_promoter_solve(si,n);
      break;

    case STAnticirceCaptureFork:
      result = anticirce_capture_fork_solve(si,n);
      break;

    case STCirceKamikazeCaptureFork:
      result = circe_kamikaze_capture_fork_solve(si,n);
      break;

    case STAnticirceDetermineRebornPiece:
      result = anticirce_determine_reborn_piece_solve(si,n);
      break;

    case STAnticirceDetermineRevelantPiece:
      result = anticirce_determine_relevant_piece_solve(si,n);
      break;

    case STAnticirceCouscousDetermineRevelantPiece:
      result = anticirce_couscous_determine_relevant_piece_solve(si,n);
      break;

    case STAnticirceDetermineRebirthSquare:
      result = anticirce_determine_rebirth_square_solve(si,n);
      break;

    case STAnticirceCheylanFilter:
      result = anticirce_cheylan_filter_solve(si,n);
      break;

    case STAnticircePlaceRebornStrict:
      result = anticirce_place_reborn_strict_solve(si,n);
      break;

    case STAnticirceRebornPromoter:
      result = anticirce_reborn_promoter_solve(si,n);
      break;

    case STAntisupercirceDetermineRebirthSquare:
      result = antisupercirce_determine_rebirth_square_solve(si,n);
      break;

    case STFootballChessSubsitutor:
      result = football_chess_substitutor_solve(si,n);
      break;

    case STRefutationsCollector:
      result = refutations_collector_solve(si,n);
      break;

    case STMinLengthGuard:
      result = min_length_guard_solve(si,n);
      break;

    case STDegenerateTree:
      result = degenerate_tree_solve(si,n);
      break;

    case STMaxNrNonTrivialCounter:
      result = max_nr_nontrivial_counter_solve(si,n);
      break;

    case STKillerDefenseCollector:
      result = killer_defense_collector_solve(si,n);
      break;

    case STFindByIncreasingLength:
      result = find_by_increasing_length_solve(si,n);
      break;

    case STHelpMovePlayed:
      result = help_move_played_solve(si,n);
      break;

    case STIntelligentMovesLeftInitialiser:
      result = intelligent_moves_left_initialiser_solve(si,n);
      break;

    case STRestartGuardIntelligent:
      result = restart_guard_intelligent_solve(si,n);
      break;

    case STIntelligentTargetCounter:
      result = intelligent_target_counter_solve(si,n);
      break;

    case STIntelligentMateFilter:
      result = intelligent_mate_filter_solve(si,n);
      break;

    case STIntelligentStalemateFilter:
      result = intelligent_stalemate_filter_solve(si,n);
      break;

    case STIntelligentProof:
      result = intelligent_proof_solve(si,n);
      break;

    case STIntelligentLimitNrSolutionsPerTargetPos:
      result = intelligent_limit_nr_solutions_per_target_position_solve(si,n);
      break;

    case STGoalReachableGuardFilterMate:
      result = goalreachable_guard_mate_solve(si,n);
      break;

    case STGoalReachableGuardFilterStalemate:
      result = goalreachable_guard_stalemate_solve(si,n);
      break;

    case STGoalReachableGuardFilterProof:
      result = goalreachable_guard_proofgame_solve(si,n);
      break;

    case STGoalReachableGuardFilterProofFairy:
      result = goalreachable_guard_proofgame_fairy_solve(si,n);
      break;

    case STRestartGuard:
      result = restart_guard_solve(si,n);
      break;

    case STMaxTimeGuard:
      result = maxtime_guard_solve(si,n);
      break;

    case STMaxSolutionsCounter:
      result = maxsolutions_counter_solve(si,n);
      break;

    case STMaxSolutionsGuard:
      result = maxsolutions_guard_solve(si,n);
      break;

    case STStopOnShortSolutionsFilter:
      result = stoponshortsolutions_solve(si,n);
      break;

    case STIfThenElse:
      result = if_then_else_solve(si,n);
      break;

    case STFlightsquaresCounter:
      result = flightsquares_counter_solve(si,n);
      break;

    case STKingMoveGenerator:
      result = king_move_generator_solve(si,n);
      break;

    case STNonKingMoveGenerator:
      result = non_king_move_generator_solve(si,n);
      break;

    case STUltraMummerMeasurerDeadend:
      result = ultra_mummer_measurer_deadend_solve(si,n);
      break;

    case STLegalAttackCounter:
    case STAnyMoveCounter:
      result = legal_attack_counter_solve(si,n);
      break;

    case STLegalDefenseCounter:
      result = legal_defense_counter_solve(si,n);
      break;

    case STCaptureCounter:
      result = capture_counter_solve(si,n);
      break;

    case STOhneschachStopIfCheck:
      result = ohneschach_stop_if_check_solve(si,n);
      break;

    case STOhneschachStopIfCheckAndNotMate:
      result = ohneschach_stop_if_check_and_not_mate_solve(si,n);
      break;

    case STOhneschachDetectUndecidableGoal:
      result = ohneschach_detect_undecidable_goal_solve(si,n);
      break;

    case STRecursionStopper:
      result = recursion_stopper_solve(si,n);
      break;

    case STSinglePieceMoveGenerator:
      result = single_piece_move_generator_solve(si,n);
      break;

    case STCastlingIntermediateMoveGenerator:
      result = castling_intermediate_move_generator_solve(si,n);
      break;

    case STCastlingRightsAdjuster:
      result = castling_rights_adjuster_solve(si,n);
      break;

    case STSingleMoveGenerator:
      result = single_move_generator_solve(si,n);
      break;

    case STOpponentMovesCounter:
      result = opponent_moves_counter_solve(si,n);
      break;

    case STOpponentMovesFewMovesPrioriser:
      result = opponent_moves_few_moves_prioriser_solve(si,n);
      break;

    case STIntelligentImmobilisationCounter:
      result = intelligent_immobilisation_counter_solve(si,n);
      break;

    case STIntelligentDuplicateAvoider:
      result = intelligent_duplicate_avoider_solve(si,n);
      break;

    case STIntelligentSolutionsPerTargetPosCounter:
      result = intelligent_nr_solutions_per_target_position_counter_solve(si,n);
      break;

    case STSetplayFork:
      result = setplay_fork_solve(si,n);
      break;

    case STAttackAdapter:
      result = attack_adapter_solve(si,n);
      break;

    case STDefenseAdapter:
      result = defense_adapter_solve(si,n);
      break;

    case STHelpAdapter:
      result = help_adapter_solve(si,n);
      break;

    case STAnd:
      result = and_solve(si,n);
      break;

    case STNot:
      result = not_solve(si,n);
      break;

    case STMoveInverter:
      result = move_inverter_solve(si,n);
      break;

    case STMaxSolutionsInitialiser:
      result = maxsolutions_initialiser_solve(si,n);
      break;

    case STStopOnShortSolutionsInitialiser:
      result = stoponshortsolutions_initialiser_solve(si,n);
      break;

    case STIllegalSelfcheckWriter:
      result = illegal_selfcheck_writer_solve(si,n);
      break;

    case STEndOfPhaseWriter:
      result = end_of_phase_writer_solve(si,n);
      break;

    case STOutputPlaintextMoveInversionCounter:
      result = output_plaintext_move_inversion_counter_solve(si,n);
      break;

    case STOutputPlaintextLineEndOfIntroSeriesMarker:
      result = output_plaintext_line_end_of_intro_series_marker_solve(si,n);
      break;

    case STPiecesParalysingMateFilter:
      result = paralysing_mate_filter_solve(si,n);
      break;

    case STPiecesParalysingStalemateSpecial:
      result = paralysing_stalemate_special_solve(si,n);
      break;

    case STPiecesParalysingRemoveCaptures:
      result = paralysing_remove_captures_solve(si,n);
      break;

    case STAmuMateFilter:
      result = amu_mate_filter_solve(si,n);
      break;

    case STCirceSteingewinnFilter:
      result = circe_steingewinn_filter_solve(si,n);
      break;

    case STCirceCircuitSpecial:
      result = circe_circuit_special_solve(si,n);
      break;

    case STCirceExchangeSpecial:
      result = circe_exchange_special_solve(si,n);
      break;

    case STAnticirceTargetSquareFilter:
      result = anticirce_target_square_filter_solve(si,n);
      break;

    case STAnticirceCircuitSpecial:
      result = anticirce_circuit_special_solve(si,n);
      break;

    case STAnticirceExchangeSpecial:
      result = anticirce_exchange_special_solve(si,n);
      break;

    case STAnticirceExchangeFilter:
      result = anticirce_exchange_filter_solve(si,n);
      break;

    case STTemporaryHackFork:
      result = solve(slices[si].next1,n);
      break;

    case STGoalTargetReachedTester:
      result = goal_target_reached_tester_solve(si,n);
      break;

    case STGoalCheckReachedTester:
      result = goal_check_reached_tester_solve(si,n);
      break;

    case STGoalCaptureReachedTester:
      result = goal_capture_reached_tester_solve(si,n);
      break;

    case STGoalSteingewinnReachedTester:
      result = goal_steingewinn_reached_tester_solve(si,n);
      break;

    case STGoalEnpassantReachedTester:
      result = goal_enpassant_reached_tester_solve(si,n);
      break;

    case STGoalDoubleMateReachedTester:
      result = goal_doublemate_reached_tester_solve(si,n);
      break;

    case STGoalCounterMateReachedTester:
      result = goal_countermate_reached_tester_solve(si,n);
      break;

    case STGoalCastlingReachedTester:
      result = goal_castling_reached_tester_solve(si,n);
      break;

    case STGoalCircuitReachedTester:
      result = goal_circuit_reached_tester_solve(si,n);
      break;

    case STGoalExchangeReachedTester:
      result = goal_exchange_reached_tester_solve(si,n);
      break;

    case STGoalCircuitByRebirthReachedTester:
      result = goal_circuit_by_rebirth_reached_tester_solve(si,n);
      break;

    case STGoalExchangeByRebirthReachedTester:
      result = goal_exchange_by_rebirth_reached_tester_solve(si,n);
      break;

    case STGoalProofgameReachedTester:
    case STGoalAToBReachedTester:
      result = goal_proofgame_reached_tester_solve(si,n);
      break;

    case STGoalImmobileReachedTester:
      result = goal_immobile_reached_tester_solve(si,n);
      break;

    case STMaffImmobilityTesterKing:
      result = maff_immobility_tester_king_solve(si,n);
      break;

    case STOWUImmobilityTesterKing:
      result = owu_immobility_tester_king_solve(si,n);
      break;

    case STGoalNotCheckReachedTester:
      result = goal_notcheck_reached_tester_solve(si,n);
      break;

    case STGoalAnyReachedTester:
      result = goal_any_reached_tester_solve(si,n);
      break;

    case STGoalChess81ReachedTester:
      result = goal_chess81_reached_tester_solve(si,n);
      break;

    case STPiecesParalysingMateFilterTester:
      result = paralysing_mate_filter_tester_solve(si,n);
      break;

    case STBlackChecks:
      result = blackchecks_solve(si,n);
      break;

    case STSingleBoxType1LegalityTester:
      result = singlebox_type1_legality_tester_solve(si,n);
      break;

    case STSingleBoxType2LegalityTester:
      result = singlebox_type2_legality_tester_solve(si,n);
      break;

    case STSingleBoxType3LegalityTester:
      result = singlebox_type3_legality_tester_solve(si,n);
      break;

    case STSingleBoxType3PawnPromoter:
      result = singlebox_type3_pawn_promoter_solve(si,n);
      break;

    case STExclusiveChessExclusivityDetector:
      result = exclusive_chess_exclusivity_detector_solve(si,n);
      break;

    case STExclusiveChessNestedExclusivityDetector:
      result = exclusive_chess_nested_exclusivity_detector_solve(si,n);
      break;

    case STExclusiveChessLegalityTester:
      result = exclusive_chess_legality_tester_solve(si,n);
      break;

    case STExclusiveChessGoalReachingMoveCounter:
      result = exclusive_chess_goal_reaching_move_counter_solve(si,n);
      break;

    case STExclusiveChessUndecidableWriterTree:
      result = exclusive_chess_undecidable_writer_tree_solve(si,n);
      break;

    case STExclusiveChessUndecidableWriterLine:
      result = exclusive_chess_undecidable_writer_line_solve(si,n);
      break;

    case STUltraschachzwangLegalityTester:
      result = ultraschachzwang_legality_tester_solve(si,n);
      break;

    case STIsardamLegalityTester:
      result = isardam_legality_tester_solve(si,n);
      break;

    case STCirceAssassinPlaceReborn:
      result = circe_assassin_place_reborn_solve(si,n);
      break;

    case STOwnKingCaptureAvoider:
      result = own_king_capture_avoider_solve(si,n);
      break;

    case STOpponentKingCaptureAvoider:
      result = opponent_king_capture_avoider_solve(si,n);
      break;

    case STPatienceChessLegalityTester:
      result = patience_chess_legality_tester_solve(si,n);
      break;

    case STStrictSATInitialiser:
      result = strict_sat_initialiser_solve(si,n);
      break;

    case STStrictSATUpdater:
      result = strict_sat_updater_solve(si,n);
      break;

    case STDynastyKingSquareUpdater:
      result = dynasty_king_square_updater_solve(si,n);
      break;

    case STHurdleColourChanger:
      result = hurdle_colour_changer_solve(si,n);
      break;

    case STOscillatingKingsTypeA:
      result = oscillating_kings_type_a_solve(si,n);
      break;

    case STOscillatingKingsTypeB:
      result = oscillating_kings_type_b_solve(si,n);
      break;

    case STOscillatingKingsTypeC:
      result = oscillating_kings_type_c_solve(si,n);
      break;

    case STPlaySuppressor:
      result = play_suppressor_solve(si,n);
      break;

    case STContinuationSolver:
      result = continuation_solver_solve(si,n);
      break;

    case STDefensePlayed:
      result = defense_played_solve(si,n);
      break;

    case STMaxFlightsquares:
      result = maxflight_guard_solve(si,n);
      break;

    case STMaxNrNonTrivial:
      result = max_nr_nontrivial_guard_solve(si,n);
      break;

    case STEndOfSolutionWriter:
      result = end_of_solution_writer_solve(si,n);
      break;

    case STKillerMoveFinalDefenseMove:
      result = killer_move_final_defense_move_solve(si,n);
      break;

    case STKillerMovePrioriser:
      result = killer_move_prioriser_solve(si,n);
      break;

    case STMaxThreatLength:
      result = maxthreatlength_guard_solve(si,n);
      break;

    case STKillerAttackCollector:
      result = killer_attack_collector_solve(si,n);
      break;

    case STMummerOrchestrator:
      result = mummer_orchestrator_solve(si,n);
      break;

    case STMummerBookkeeper:
      result = mummer_bookkeeper_solve(si,n);
      break;

    case STBackHomeMovesOnly:
      result = back_home_moves_only_solve(si,n);
      break;

    case STNoPromotionsRemovePromotionMoving:
      result = nopromotion_avoid_promotion_moving_solve(si,n);
      break;

    case STNoPromotionsRemovePromotionReborn:
      result = nopromotion_avoid_promotion_reborn_solve(si,n);
      break;

    case STGenevaRemoveIllegalCaptures:
      result = geneva_remove_illegal_captures_solve(si,n);
      break;

    case STNocatpureRemoveCaptures:
      result = nocapture_remove_captures_solve(si,n);
      break;

    case STWoozlesRemoveIllegalCaptures:
      result = woozles_remove_illegal_captures_solve(si,n);
      break;

    case STImmuneRemoveCapturesOfImmune:
      result = immune_remove_captures_of_immune_solve(si,n);
      break;

    case STProvocateursRemoveUnobservedCaptures:
      result = provocateurs_remove_unobserved_captures_solve(si,n);
      break;

    case STLortapRemoveSupportedCaptures:
      result = lortap_remove_supported_captures_solve(si,n);
      break;

    case STPatrolRemoveUnsupportedCaptures:
      result = patrol_remove_unsupported_captures_solve(si,n);
      break;

    case STKoekoLegalityTester:
      result = koeko_legality_tester_solve(si,n);
      break;

    case STGridContactLegalityTester:
      result = contact_grid_legality_tester_solve(si,n);
      break;

    case STAntiKoekoLegalityTester:
      result = antikoeko_legality_tester_solve(si,n);
      break;

    case STNewKoekoRememberContact:
      result = newkoeko_remember_contact_solve(si,n);
      break;

    case STNewKoekoLegalityTester:
      result = newkoeko_legality_tester_solve(si,n);
      break;

    case STJiggerLegalityTester:
      result = jigger_legality_tester_solve(si,n);
      break;

    case STMonochromeRemoveBichromeMoves:
      result = monochrome_remove_bichrome_moves_solve(si,n);
      break;

    case STBichromeRemoveMonochromeMoves:
      result = bichrome_remove_monochrome_moves_solve(si,n);
      break;

    case STEdgeMoverRemoveIllegalMoves:
      result = edgemover_remove_illegal_moves_solve(si,n);
      break;

    case STSuperguardsRemoveIllegalCaptures:
      result = superguards_remove_illegal_captures_solve(si,n);
      break;

    case STGridRemoveIllegalMoves:
      result = grid_remove_illegal_moves_solve(si,n);
      break;

    case STUncapturableRemoveCaptures:
      result = uncapturable_remove_captures_solve(si,n);
      break;

    case STMessignyRemoveIllegalSwapRepetitions:
      result = messigny_remove_illegal_swap_repetitions_solve(si,n);
      break;

    case STTakeAndMakeGenerateMake:
      result = take_and_make_generate_make_solve(si,n);
      break;

    case STTakeAndMakeAvoidPawnMakeToBaseLine:
      result = take_and_make_avoid_pawn_make_to_base_line_solve(si,n);
      break;

    case STWormholeTransferer:
      result = wormhole_transferer_solve(si,n);
      break;

    case STWormholeTransferedPromoter:
      result = wormhole_transfered_promoter_solve(si,n);
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
