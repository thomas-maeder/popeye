#include "solving/machinery/dispatch.h"
#include "conditions/amu/mate_filter.h"
#include "conditions/amu/attack_counter.h"
#include "conditions/anticirce/cheylan.h"
#include "conditions/anticirce/magic_square.h"
#include "conditions/anticirce/clone.h"
#include "conditions/anticirce/couscous.h"
#include "conditions/bgl.h"
#include "conditions/blackchecks.h"
#include "conditions/koeko/koeko.h"
#include "conditions/koeko/contact_grid.h"
#include "conditions/koeko/anti.h"
#include "conditions/koeko/new.h"
#include "conditions/circe/antipodes.h"
#include "conditions/circe/chameleon.h"
#include "conditions/circe/diametral.h"
#include "conditions/circe/verticalmirror.h"
#include "conditions/circe/circuit_by_rebirth_special.h"
#include "conditions/circe/clone.h"
#include "conditions/circe/couscous.h"
#include "conditions/circe/diagram.h"
#include "conditions/circe/einstein.h"
#include "conditions/circe/exchange_by_rebirth_special.h"
#include "conditions/circe/file.h"
#include "conditions/circe/frischauf.h"
#include "conditions/circe/glasgow.h"
#include "conditions/circe/steingewinn_filter.h"
#include "conditions/circe/assassin.h"
#include "conditions/circe/frischauf.h"
#include "conditions/circe/super.h"
#include "conditions/circe/april.h"
#include "conditions/circe/takemake.h"
#include "conditions/circe/mirror.h"
#include "conditions/circe/pwc.h"
#include "conditions/circe/rank.h"
#include "conditions/circe/symmetry.h"
#include "conditions/circe/rex_inclusive.h"
#include "conditions/circe/parachute.h"
#include "conditions/circe/rebirth_square_occupied.h"
#include "conditions/circe/relevant_piece.h"
#include "conditions/exclusive.h"
#include "conditions/extinction.h"
#include "conditions/ohneschach.h"
#include "conditions/maff/immobility_tester.h"
#include "conditions/owu/immobility_tester.h"
#include "conditions/provocateurs.h"
#include "conditions/lortap.h"
#include "conditions/patrol.h"
#include "conditions/monochrome.h"
#include "conditions/bichrome.h"
#include "conditions/ultraschachzwang/legality_tester.h"
#include "conditions/singlebox/type1.h"
#include "conditions/singlebox/type2.h"
#include "conditions/singlebox/type3.h"
#include "conditions/snek.h"
#include "conditions/patience.h"
#include "conditions/isardam.h"
#include "conditions/sat.h"
#include "conditions/dynasty.h"
#include "conditions/masand.h"
#include "conditions/messigny.h"
#include "conditions/oscillating_kings.h"
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
#include "conditions/anticirce/anticirce.h"
#include "conditions/sentinelles.h"
#include "conditions/duellists.h"
#include "conditions/haunted_chess.h"
#include "conditions/ghost_chess.h"
#include "conditions/kobul.h"
#include "conditions/andernach.h"
#include "conditions/antiandernach.h"
#include "conditions/bicolores.h"
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
#include "conditions/marscirce/marscirce.h"
#include "conditions/marscirce/phantom.h"
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
#include "conditions/shielded_kings.h"
#include "conditions/annan.h"
#include "conditions/beamten.h"
#include "conditions/central.h"
#include "conditions/disparate.h"
#include "conditions/eiffel.h"
#include "conditions/facetoface.h"
#include "conditions/madrasi.h"
#include "conditions/disparate.h"
#include "conditions/marscirce/marscirce.h"
#include "conditions/marscirce/plus.h"
#include "conditions/messigny.h"
#include "conditions/patrol.h"
#include "conditions/marscirce/phantom.h"
#include "conditions/singlebox/type3.h"
#include "conditions/castling_chess.h"
#include "conditions/exchange_castling.h"
#include "conditions/transmuting_kings/transmuting_kings.h"
#include "conditions/transmuting_kings/super.h"
#include "conditions/transmuting_kings/reflective_kings.h"
#include "conditions/transmuting_kings/vaulting_kings.h"
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
#include "optimisations/detect_retraction.h"
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
#include "optimisations/orthodox_square_observation.h"
#include "optimisations/ohneschach/non_checking_first.h"
#include "optimisations/ohneschach/stop_if_check.h"
#include "optimisations/observation.h"
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
#include "output/output.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/end_of_phase_writer.h"
#include "output/plaintext/ohneschach_detect_undecidable_goal.h"
#include "output/plaintext/illegal_selfcheck_writer.h"
#include "output/plaintext/goal_writer.h"
#include "output/plaintext/move_inversion_counter.h"
#include "output/plaintext/line/line_writer.h"
#include "output/plaintext/line/end_of_intro_series_marker.h"
#include "output/plaintext/line/refuting_variation_writer.h"
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
#include "output/plaintext/twinning.h"
#include "output/latex/twinning.h"
#include "output/latex/line/line_writer.h"
#include "output/latex/goal_writer.h"
#include "output/latex/tree/end_of_solution_writer.h"
#include "output/latex/tree/check_writer.h"
#include "output/latex/tree/key_writer.h"
#include "output/latex/tree/move_writer.h"
#include "output/latex/tree/refutation_writer.h"
#include "output/latex/tree/refuting_variation_writer.h"
#include "output/latex/tree/threat_writer.h"
#include "output/latex/tree/try_writer.h"
#include "output/latex/tree/zugzwang_writer.h"
#include "pieces/attributes/paralysing/paralysing.h"
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
#include "pieces/walks/generate_moves.h"
#include "pieces/walks/pawns/en_passant.h"
#include "pieces/walks/pawns/promotion.h"
#include "retro/retro.h"
#include "stipulation/proxy.h"
#include "solving/machinery/slack_length.h"
#include "solving/avoid_unsolvable.h"
#include "solving/battle_play/attack_adapter.h"
#include "solving/battle_play/defense_adapter.h"
#include "solving/battle_play/continuation.h"
#include "solving/battle_play/min_length_guard.h"
#include "solving/battle_play/min_length_optimiser.h"
#include "solving/battle_play/threat.h"
#include "solving/battle_play/try.h"
#include "solving/battle_play/threat.h"
#include "solving/help_play/adapter.h"
#include "solving/boolean/or.h"
#include "solving/boolean/and.h"
#include "solving/boolean/not.h"
#include "solving/castling.h"
#include "solving/capture_counter.h"
#include "solving/find_by_increasing_length.h"
#include "solving/find_move.h"
#include "solving/find_shortest.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/for_each_move.h"
#include "solving/fork_on_remaining.h"
#include "solving/if_then_else.h"
#include "solving/king_move_generator.h"
#include "solving/legal_move_counter.h"
#include "solving/move_generator.h"
#include "solving/move_inverter.h"
#include "solving/move_played.h"
#include "solving/move_player.h"
#include "solving/non_king_move_generator.h"
#include "solving/play_suppressor.h"
#include "solving/single_piece_move_generator.h"
#include "solving/trivial_end_filter.h"
#include "solving/selfcheck_guard.h"
#include "solving/post_move_iteration.h"
#include "solving/move_effect_journal.h"
#include "solving/king_capture_avoider.h"
#include "solving/recursion_stopper.h"
#include "solving/proofgames.h"
#include "solving/constraint.h"
#include "solving/dead_end.h"
#include "solving/dummy_move.h"
#include "solving/end_of_branch_goal.h"
#include "solving/end_of_branch.h"
#include "solving/setplay_fork.h"
#include "solving/has_solution_type.h"
#include "solving/goals/any.h"
#include "solving/goals/capture.h"
#include "solving/goals/castling.h"
#include "solving/goals/chess81.h"
#include "solving/goals/check.h"
#include "solving/goals/circuit.h"
#include "solving/goals/circuit_by_rebirth.h"
#include "solving/goals/countermate.h"
#include "solving/goals/doublemate.h"
#include "solving/goals/enpassant.h"
#include "solving/goals/exchange.h"
#include "solving/goals/exchange_by_rebirth.h"
#include "solving/goals/immobile.h"
#include "solving/goals/king_capture.h"
#include "solving/goals/kiss.h"
#include "solving/goals/notcheck.h"
#include "solving/goals/proofgame.h"
#include "solving/goals/steingewinn.h"
#include "solving/goals/target.h"
#include "solving/goals/prerequisite_optimiser.h"
#include "solving/goals/reached_tester.h"
#include "solving/pipe.h"
#include "solving/machinery/solve.h"
#include "solving/machinery/solvers.h"
#include "debugging/trace.h"
#include "debugging/measure.h"
#include "debugging/assert.h"

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void dispatch(slice_index si)
{
  TraceEnumerator(slice_type,SLICE_TYPE(si),"\n");
  switch (SLICE_TYPE(si))
  {
    case STSolvingMachineryBuilder:
      build_solving_machinery(si);
      break;

    case STStartOfSolvingEnvironment:
    case STStartOfSolvingMachinery:
      solve(SLICE_NEXT1(si));
      break;

    case STDuplexSolver:
    {
      void duplex_solve(slice_index si);
      duplex_solve(si);
      break;
    }

    case STHalfDuplexSolver:
    {
      void half_duplex_solve(slice_index si);
      half_duplex_solve(si);
      break;
    }

    case STCreateBuilderSetupPly:
      create_builder_setup_ply_solve(si);
      break;

    case STPieceWalkCacheInitialiser:
      piece_walk_caches_initialiser_solve(si);
      break;

    case STPiecesCounter:
    {
      void pieces_counter_solve(slice_index si);
      pieces_counter_solve(si);
      break;
    }

    case STPiecesFlagsInitialiser:
    {
      void initialise_piece_flags_solve(slice_index si);
      initialise_piece_flags_solve(si);
      break;
    }

    case STRoyalsLocator:
    {
      void royals_locator_solve(slice_index si);
      royals_locator_solve(si);
      break;
    }

    case STProofgameVerifyUniqueGoal:
      proof_verify_unique_goal_solve(si);
      break;

    case STProofgameInitialiser:
      proof_solve(si);
      break;

    case STAToBInitialiser:
      atob_solve(si);
      break;

    case STInputVerification:
    {
      void verify_position(slice_index si);
      verify_position(si);
      break;
    }

    case STMoveEffectsJournalReset:
      move_effect_journal_reset(si);
      break;

    case STSolversBuilder:
      build_solvers(si);
      break;

    case STProxyResolver:
      proxies_resolve(si);
      break;

    case STSlackLengthAdjuster:
      adjust_slack_length(si);
      break;

    case STThreatSolver:
      threat_solver_solve(si);
      break;

    case STDummyMove:
      dummy_move_solve(si);
      break;

    case STThreatCollector:
      threat_collector_solve(si);
      break;

    case STThreatEnforcer:
      threat_enforcer_solve(si);
      break;

    case STThreatDefeatedTester:
      threat_defeated_tester_solve(si);
      break;

    case STOutputPlainTextThreatWriter:
      output_plaintext_tree_threat_writer_solve(si);
      break;

    case STOutputPlainTextZugzwangWriter:
      output_plaintext_tree_zugzwang_writer_solve(si);
      break;

    case STOutputPlainTextKeyWriter:
      output_plaintext_tree_key_writer_solve(si);
      break;

    case STOutputPlainTextTryWriter:
      output_plaintext_tree_try_writer_solve(si);
      break;

    case STRefutationsAllocator:
      refutations_allocator_solve(si);
      break;

    case STRefutationsSolver:
      refutations_solver_solve(si);
      break;

    case STOutputPlainTextRefutationsIntroWriter:
      output_plaintext_tree_refutations_intro_writer_solve(si);
      break;

    case STRefutationsAvoider:
      refutations_avoider_solve(si);
      break;

    case STRefutationsFilter:
      refutations_filter_solve(si);
      break;

    case STOutputPlaintextTreeRefutingVariationWriter:
      output_plaintext_tree_refuting_variation_writer_solve(si);
      break;

    case STOutputPlaintextLineRefutingVariationWriter:
      output_plaintext_line_refuting_variation_writer_solve(si);
      break;

    case STOutputPlainTextMoveWriter:
      output_plaintext_tree_move_writer_solve(si);
      break;

    case STTrivialEndFilter:
      trivial_end_filter_solve(si);
      break;

    case STNoShortVariations:
      no_short_variations_solve(si);
      break;

    case STOr:
      or_solve(si);
      break;

    case STFindShortest:
      find_shortest_solve(si);
      break;

    case STMoveGenerator:
      move_generator_solve(si);
      break;

    case STForEachAttack:
      for_each_attack_solve(si);
      break;

    case STFindAttack:
      find_attack_solve(si);
      break;

    case STForEachDefense:
      for_each_defense_solve(si);
      break;

    case STFindDefense:
      find_defense_solve(si);
      break;

    case STNullMovePlayer:
      null_move_player_solve(si);
      break;

    case STPostMoveIterationInitialiser:
      move_execution_post_move_iterator_solve(si);
      break;

    case STMoveEffectJournalUndoer:
      move_effect_journal_undoer_solve(si);
      break;

    case STMessignyMovePlayer:
      messigny_move_player_solve(si);
      break;

    case STCastlingPlayer:
      castling_player_solve(si);
      break;

    case STMovePlayer:
      move_player_solve(si);
      break;

    case STEnPassantAdjuster:
      en_passant_adjuster_solve(si);
      break;

    case STEinsteinEnPassantAdjuster:
      einstein_en_passant_adjuster_solve(si);
      break;

    case STPawnPromoter:
      pawn_promoter_solve(si);
      break;

    case STMarsCirceMoveToRebirthSquare:
      marscirce_move_to_rebirth_square_solve(si);
      break;

    case STKamikazeCapturingPieceRemover:
      kamikaze_capturing_piece_remover_solve(si);
      break;

    case STHaanChessHoleInserter:
      haan_chess_hole_inserter_solve(si);
      break;

    case STCastlingChessMovePlayer:
      castling_chess_move_player_solve(si);
      break;

    case STExchangeCastlingMovePlayer:
      exchange_castling_move_player_solve(si);
      break;

    case STSuperTransmutingKingTransmuter:
      supertransmuting_kings_transmuter_solve(si);
      break;

    case STSuperTransmutingKingMoveGenerationFilter:
      supertransmuting_kings_move_generation_filter_solve(si);
      break;

    case STAMUAttackCounter:
      amu_attack_counter_solve(si);
      break;

    case STMutualCastlingRightsAdjuster:
      mutual_castling_rights_adjuster_solve(si);
      break;

    case STImitatorMover:
      imitator_mover_solve(si);
      break;

    case STImitatorRemoveIllegalMoves:
      imitator_remove_illegal_moves_solve(si);
      break;

    case STImitatorDetectIllegalMoves:
      imitator_detect_illegal_moves_solve(si);
      break;

    case STPawnToImitatorPromoter:
      imitator_pawn_promoter_solve(si);
      break;

    case STAttackPlayed:
      attack_played_solve(si);
      break;

#if defined(DOTRACE)
    case STMoveTracer:
      move_tracer_solve(si);
      break;
#endif

#if defined(DOMEASURE)
    case STMoveCounter:
      move_counter_solve(si);
      break;
#endif

    case STOrthodoxMatingMoveGenerator:
      orthodox_mating_move_generator_solve(si);
      break;

    case STOrthodoxMatingKingContactGenerator:
      orthodox_mating_king_contact_generator_solve(si);
      break;

    case STDeadEnd:
    case STDeadEndGoal:
      dead_end_solve(si);
      break;

    case STMinLengthOptimiser:
      min_length_optimiser_solve(si);
      break;

    case STForkOnRemaining:
      fork_on_remaining_solve(si);
      break;

    case STHashOpener:
      hash_opener_solve(si);
      break;

    case STAttackHashed:
      attack_hashed_solve(si);
      break;

    case STAttackHashedTester:
      attack_hashed_tester_solve(si);
      break;

    case STHelpHashed:
      help_hashed_solve(si);
      break;

    case STHelpHashedTester:
      help_hashed_tester_solve(si);
      break;

    case STEndOfBranch:
    case STEndOfBranchForced:
    case STEndOfBranchTester:
      end_of_branch_solve(si);
      break;

    case STEndOfBranchGoal:
    case STEndOfBranchGoalImmobile:
    case STEndOfBranchGoalTester:
      end_of_branch_goal_solve(si);
      break;

    case STGoalReachedTester:
      goal_reached_tester_solve(si);
      break;

    case STAvoidUnsolvable:
      avoid_unsolvable_solve(si);
      break;

    case STResetUnsolvable:
      reset_unsolvable_solve(si);
      break;

    case STLearnUnsolvable:
      learn_unsolvable_solve(si);
      break;

    case STConstraintSolver:
    case STConstraintTester:
    case STGoalConstraintTester:
      constraint_solve(si);
      break;

    case STSelfCheckGuard:
    case STExclusiveChessGoalReachingMoveCounterSelfCheckGuard:
      selfcheck_guard_solve(si);
      break;

    case STKeepMatingFilter:
      keepmating_filter_solve(si);
      break;

    case STOutputPlaintextTreeCheckWriter:
      output_plaintext_tree_check_writer_solve(si);
      break;

    case STOutputPlainTextRefutationWriter:
      output_plaintext_tree_refutation_writer_solve(si);
      break;

    case STDoubleMateFilter:
      doublemate_filter_solve(si);
      break;

    case STCounterMateFilter:
      countermate_filter_solve(si);
      break;

    case STEnPassantFilter:
      enpassant_filter_solve(si);
      break;

    case STEnPassantRemoveNonReachers:
      enpassant_remove_non_reachers_solve(si);
      break;

    case STCastlingFilter:
      castling_filter_solve(si);
      break;

    case STCastlingRemoveNonReachers:
      castling_remove_non_reachers_solve(si);
      break;

    case STPrerequisiteOptimiser:
      goal_prerequisite_optimiser_solve(si);
      break;

    case STChess81RemoveNonReachers:
      chess81_remove_non_reachers_solve(si);
      break;

    case STCaptureRemoveNonReachers:
      capture_remove_non_reachers_solve(si);
      break;

    case STTargetRemoveNonReachers:
      target_remove_non_reachers_solve(si);
      break;

    case STDetectMoveRetracted:
      detect_move_retracted_solve(si);
      break;

    case STRetractionPrioriser:
      priorise_retraction_solve(si);
      break;

    case STOutputPlaintextGoalWriter:
      output_plaintext_goal_writer_solve(si);
      break;

    case STOutputPlaintextLineLineWriter:
      output_plaintext_line_line_writer_solve(si);
      break;

    case STOutputLaTeXGoalWriter:
      output_latex_goal_writer_solve(si);
      break;

    case STOutputLaTeXLineLineWriter:
      output_latex_line_line_writer_solve(si);
      break;

    case STOutputLaTeXTwinningWriter:
      output_latex_write_twinning(si);
      break;

    case STOutputLaTeXTreeEndOfSolutionWriter:
      output_latex_tree_end_of_solution_writer_solve(si);
      break;

    case STOutputLaTeXKeyWriter:
      output_latex_tree_key_writer_solve(si);
      break;

    case STOutputLaTeXMoveWriter:
      output_latex_tree_move_writer_solve(si);
      break;

    case STOutputLaTeXRefutationWriter:
      output_latex_tree_refutation_writer_solve(si);
      break;

    case STOutputLaTeXThreatWriter:
      output_latex_tree_threat_writer_solve(si);
      break;

    case STOutputLaTeXTreeCheckWriter:
      output_latex_tree_check_writer_solve(si);
      break;

    case STOutputLaTeXTreeRefutingVariationWriter:
      output_latex_tree_refuting_variation_writer_solve(si);
      break;

    case STOutputLaTeXTryWriter:
      output_latex_tree_try_writer_solve(si);
      break;

    case STOutputLaTeXZugzwangWriter:
      output_latex_tree_zugzwang_writer_solve(si);
      break;

    case STBGLEnforcer:
      bgl_enforcer_solve(si);
      break;

    case STBGLAdjuster:
      bgl_adjuster_solve(si);
      break;

    case STMasandRecolorer:
      masand_recolorer_solve(si);
      break;

    case STActuatedRevolvingCentre:
      actuated_revolving_centre_solve(si);
      break;

    case STActuatedRevolvingBoard:
      actuated_revolving_board_solve(si);
      break;

    case STRepublicanKingPlacer:
      republican_king_placer_solve(si);
      break;

    case STRepublicanType1DeadEnd:
      republican_type1_dead_end_solve(si);
      break;

    case STCirceCaptureFork:
      circe_capture_fork_solve(si);
      break;

    case STCircePreventKingRebirth:
      circe_prevent_king_rebirth_solve(si);
      break;

    case STCirceInitialiseRelevantFromReborn:
      circe_initialise_relevant_from_reborn(si);
      break;

    case STCirceInitialiseFromCurrentMove:
      circe_make_current_move_relevant_solve(si);
      break;

    case STCirceInitialiseFromLastMove:
      circe_make_last_move_relevant_solve(si);
      break;

    case STCirceInitialiseRebornFromCapturee:
      circe_initialise_reborn_from_capturee_solve(si);
      break;

    case STCirceCloneDetermineRebornWalk:
      circe_clone_determine_reborn_walk_solve(si);
      break;

    case STAntiCloneCirceDetermineRebornWalk:
      anti_clone_circe_determine_reborn_walk_solve(si);
      break;

    case STCirceDoubleAgentsAdaptRebornSide:
      circe_doubleagents_adapt_reborn_side_solve(si);
      break;

    case STChameleonCirceAdaptRebornWalk:
      chameleon_circe_adapt_reborn_walk_solve(si);
      break;

    case STCirceEinsteinAdjustRebornWalk:
      circe_einstein_adjust_reborn_walk_solve(si);
      break;

    case STCirceReversalEinsteinAdjustRebornWalk:
      circe_reversal_einstein_adjust_reborn_walk_solve(si);
      break;

    case STCirceCouscousMakeCapturerRelevant:
      circe_couscous_make_capturer_relevant_solve(si);
      break;

    case STAnticirceCouscousMakeCaptureeRelevant:
      anticirce_couscous_make_capturee_relevant_solve(si);
      break;

    case STMirrorCirceOverrideRelevantSide:
      mirror_circe_override_relevant_side_solve(si);
      break;

    case STCirceDetermineRebirthSquare:
      circe_determine_rebirth_square_solve(si);
      break;

    case STCirceFrischaufAdjustRebirthSquare:
      frischauf_adjust_rebirth_square_solve(si);
      break;

    case STCirceGlasgowAdjustRebirthSquare:
      circe_glasgow_adjust_rebirth_square_solve(si);
      break;

    case STFileCirceDetermineRebirthSquare:
      file_circe_determine_rebirth_square_solve(si);
      break;

    case STCirceDiametralAdjustRebirthSquare:
      circe_diametral_adjust_rebirth_square_solve(si);
      break;

    case STCirceVerticalMirrorAdjustRebirthSquare:
      circe_verticalmirror_adjust_rebirth_square_solve(si);
      break;

    case STRankCirceOverrideRelevantSide:
      rank_circe_override_relevant_side_solve(si);
      break;

    case STRankCirceProjectRebirthSquare:
      rank_circe_adjust_rebirth_square_solve(si);
      break;

    case STTakeMakeCirceDetermineRebirthSquares:
      take_make_circe_determine_rebirth_squares_solve(si);
      break;

    case STTakeMakeCirceCollectRebirthSquares:
      take_make_circe_collect_rebirth_squares_solve(si);
      break;

    case STAntipodesCirceDetermineRebirthSquare:
      antipodes_circe_determine_rebirth_square_solve(si);
      break;

    case STSymmetryCirceDetermineRebirthSquare:
      symmetry_circe_determine_rebirth_square_solve(si);
      break;

    case STPWCDetermineRebirthSquare:
      pwc_determine_rebirth_square_solve(si);
      break;

    case STDiagramCirceDetermineRebirthSquare:
      diagram_circe_determine_rebirth_square_solve(si);
      break;

    case STContactGridAvoidCirceRebirth:
      contact_grid_avoid_circe_rebirth(si);
      break;

    case STGenevaStopCaptureFromRebirthSquare:
      geneva_stop_catpure_from_rebirth_square_solve(si);
      break;

    case STCirceTestRebirthSquareEmpty:
      circe_test_rebirth_square_empty_solve(si);
      break;

    case STCircePlaceReborn:
      circe_place_reborn_solve(si);
      break;

    case STCirceDoneWithRebirth:
      circe_done_with_rebirth(si);
      break;

    case STAprilCaptureFork:
      april_chess_fork_solve(si);
      break;

    case STSuperCirceNoRebirthFork:
      supercirce_no_rebirth_fork_solve(si);
      break;

    case STSuperCirceDetermineRebirthSquare:
      supercirce_determine_rebirth_square_solve(si);
      break;

    case STSupercircePreventRebirthOnNonEmptySquare:
      supercirce_prevent_rebirth_on_non_empty_square_solve(si);
      break;

    case STCirceVolageRecolorer:
      circe_volage_recolorer_solve(si);
      break;

    case STCirceParrainThreatFork:
      circe_parrain_threat_fork_solve(si);
      break;

    case STCirceParrainDetermineRebirth:
      circe_parrain_determine_rebirth_solve(si);
      break;

    case STCirceContraparrainDetermineRebirth:
      circe_contraparrain_determine_rebirth_solve(si);
      break;

    case STCirceCageNoCageFork:
      circe_cage_no_cage_fork_solve(si);
      break;

    case STCirceCageCageTester:
      circe_cage_cage_tester_solve(si);
      break;

    case STCageCirceFutileCapturesRemover:
      circe_cage_futile_captures_remover_solve(si);
      break;

    case STSentinellesInserter:
      sentinelles_inserter_solve(si);
      break;

    case STMagicViewsInitialiser:
      magic_views_initialiser_solve(si);
      break;

    case STMagicPiecesRecolorer:
      magic_pieces_recolorer_solve(si);
      break;

    case STHauntedChessGhostSummoner:
      haunted_chess_ghost_summoner_solve(si);
      break;

    case STHauntedChessGhostRememberer:
      haunted_chess_ghost_rememberer_solve(si);
      break;

    case STGhostChessGhostRememberer:
      ghost_chess_ghost_rememberer_solve(si);
      break;

    case STAndernachSideChanger:
      andernach_side_changer_solve(si);
      break;

    case STAntiAndernachSideChanger:
      antiandernach_side_changer_solve(si);
      break;

    case STChameleonPursuitSideChanger:
      chameleon_pursuit_side_changer_solve(si);
      break;

    case STNorskRemoveIllegalCaptures:
      norsk_remove_illegal_captures_solve(si);
      break;

    case STNorskArrivingAdjuster:
      norsk_arriving_adjuster_solve(si);
      break;

    case STProteanPawnAdjuster:
      protean_pawn_adjuster_solve(si);
      break;

    case STEinsteinArrivingAdjuster:
      einstein_moving_adjuster_solve(si);
      break;

    case STReverseEinsteinArrivingAdjuster:
      reverse_einstein_moving_adjuster_solve(si);
      break;

    case STAntiEinsteinArrivingAdjuster:
      anti_einstein_moving_adjuster_solve(si);
      break;

    case STTraitorSideChanger:
      traitor_side_changer_solve(si);
      break;

    case STVolageSideChanger:
      volage_side_changer_solve(si);
      break;

    case STMagicSquareSideChanger:
    case STMagicSquareType2SideChanger:
      magic_square_side_changer_solve(si);
      break;

    case STMagicSquareType2AnticirceRelevantSideAdapter:
      magic_square_anticirce_relevant_side_adapter_solve(si);
      break;

    case STTibetSideChanger:
      tibet_solve(si);
      break;

    case STDoubleTibetSideChanger:
      double_tibet_solve(si);
      break;

    case STDegradierungDegrader:
      degradierung_degrader_solve(si);
      break;

    case STChameleonChangePromoteeInto:
      chameleon_change_promotee_into_solve(si);
      break;

    case STChameleonArrivingAdjuster:
      chameleon_arriving_adjuster_solve(si);
      break;

    case STChameleonChessArrivingAdjuster:
      chameleon_chess_arriving_adjuster_solve(si);
      break;

    case STLineChameleonArrivingAdjuster:
      line_chameleon_arriving_adjuster_solve(si);
      break;

    case STFrischaufPromoteeMarker:
      frischauf_promotee_marker_solve(si);
      break;

    case STPiecesHalfNeutralRecolorer:
      half_neutral_recolorer_solve(si);
      break;

    case STKobulKingSubstitutor:
      kobul_king_substitutor_solve(si);
      break;

    case STSnekSubstitutor:
      snek_substitutor_solve(si);
      break;

    case STSnekCircleSubstitutor:
      snek_circle_substitutor_solve(si);
      break;

    case STDuellistsRememberDuellist:
      duellists_remember_duellist_solve(si);
      break;

    case STSingleboxType2LatentPawnSelector:
      singlebox_type2_latent_pawn_selector_solve(si);
      break;

    case STSingleboxType2LatentPawnPromoter:
      singlebox_type2_latent_pawn_promoter_solve(si);
      break;

    case STCirceKamikazeCaptureFork:
      circe_kamikaze_capture_fork_solve(si);
      break;

    case STAnticirceInitialiseRebornFromCapturer:
      circe_initialise_reborn_from_capturer_solve(si);
      break;

    case STMarscirceInitialiseRebornFromGenerated:
      marscirce_initialise_reborn_from_generated_solve(si);
      break;

    case STGenevaInitialiseRebornFromCapturer:
      geneva_initialise_reborn_from_capturer_solve(si);
      break;

    case STAnticirceCheylanFilter:
      anticirce_cheylan_filter_solve(si);
      break;

    case STAnticirceRemoveCapturer:
      anticirce_remove_capturer_solve(si);
      break;

    case STMarscirceRemoveCapturer:
      marscirce_remove_capturer_solve(si);
      break;

    case STFootballChessSubsitutor:
      football_chess_substitutor_solve(si);
      break;

    case STRefutationsCollector:
      refutations_collector_solve(si);
      break;

    case STMinLengthGuard:
      min_length_guard_solve(si);
      break;

    case STDegenerateTree:
      degenerate_tree_solve(si);
      break;

    case STMaxNrNonTrivialCounter:
      max_nr_nontrivial_counter_solve(si);
      break;

    case STKillerDefenseCollector:
      killer_defense_collector_solve(si);
      break;

    case STFindByIncreasingLength:
      find_by_increasing_length_solve(si);
      break;

    case STHelpMovePlayed:
      help_move_played_solve(si);
      break;

    case STIntelligentMovesLeftInitialiser:
      intelligent_moves_left_initialiser_solve(si);
      break;

    case STRestartGuardIntelligent:
      restart_guard_intelligent_solve(si);
      break;

    case STIntelligentTargetCounter:
      intelligent_target_counter_solve(si);
      break;

    case STIntelligentMateFilter:
      intelligent_mate_filter_solve(si);
      break;

    case STIntelligentStalemateFilter:
      intelligent_stalemate_filter_solve(si);
      break;

    case STIntelligentProof:
      intelligent_proof_solve(si);
      break;

    case STIntelligentLimitNrSolutionsPerTargetPos:
      intelligent_limit_nr_solutions_per_target_position_solve(si);
      break;

    case STGoalReachableGuardFilterMate:
      goalreachable_guard_mate_solve(si);
      break;

    case STGoalReachableGuardFilterStalemate:
      goalreachable_guard_stalemate_solve(si);
      break;

    case STGoalReachableGuardFilterProof:
      goalreachable_guard_proofgame_solve(si);
      break;

    case STGoalReachableGuardFilterProofFairy:
      goalreachable_guard_proofgame_fairy_solve(si);
      break;

    case STRestartGuard:
      restart_guard_solve(si);
      break;

    case STRestartGuardNested:
      restart_guard_nested_solve(si);
      break;

    case STMaxTimeSetter:
      maxtime_set(si);
      break;

    case STMaxTimeGuard:
      maxtime_guard_solve(si);
      break;

    case STMaxSolutionsCounter:
      maxsolutions_counter_solve(si);
      break;

    case STMaxSolutionsGuard:
      maxsolutions_guard_solve(si);
      break;

    case STStopOnShortSolutionsFilter:
      stoponshortsolutions_solve(si);
      break;

    case STIfThenElse:
      if_then_else_solve(si);
      break;

    case STFlightsquaresCounter:
      flightsquares_counter_solve(si);
      break;

    case STKingMoveGenerator:
      king_move_generator_solve(si);
      break;

    case STNonKingMoveGenerator:
      non_king_move_generator_solve(si);
      break;

    case STUltraMummerMeasurerDeadend:
      ultra_mummer_measurer_deadend_solve(si);
      break;

    case STLegalAttackCounter:
    case STAnyMoveCounter:
      legal_attack_counter_solve(si);
      break;

    case STLegalDefenseCounter:
      legal_defense_counter_solve(si);
      break;

    case STCaptureCounter:
      capture_counter_solve(si);
      break;

    case STOhneschachStopIfCheck:
      ohneschach_stop_if_check_solve(si);
      break;

    case STOhneschachStopIfCheckAndNotMate:
      ohneschach_stop_if_check_and_not_mate_solve(si);
      break;

    case STOhneschachDetectUndecidableGoal:
      ohneschach_detect_undecidable_goal_solve(si);
      break;

    case STRecursionStopper:
      recursion_stopper_solve(si);
      break;

    case STSinglePieceMoveGenerator:
      single_piece_move_generator_solve(si);
      break;

    case STCastlingRightsAdjuster:
      castling_rights_adjuster_solve(si);
      break;

    case STOpponentMovesCounter:
      opponent_moves_counter_solve(si);
      break;

    case STOpponentMovesFewMovesPrioriser:
      opponent_moves_few_moves_prioriser_solve(si);
      break;

    case STIntelligentImmobilisationCounter:
      intelligent_immobilisation_counter_solve(si);
      break;

    case STIntelligentDuplicateAvoider:
      intelligent_duplicate_avoider_solve(si);
      break;

    case STIntelligentSolutionRememberer:
      intelligent_solution_rememberer_solve(si);
      break;

    case STIntelligentSolutionsPerTargetPosCounter:
      intelligent_nr_solutions_per_target_position_counter_solve(si);
      break;

    case STSetplayFork:
      setplay_fork_solve(si);
      break;

    case STAttackAdapter:
      attack_adapter_solve(si);
      break;

    case STDefenseAdapter:
      defense_adapter_solve(si);
      break;

    case STHelpAdapter:
      help_adapter_solve(si);
      break;

    case STAnd:
      and_solve(si);
      break;

    case STNot:
      not_solve(si);
      break;

    case STMoveInverter:
    case STMoveInverterSetPlay:
      move_inverter_solve(si);
      break;

    case STRetroStartRetractionPly:
      retro_start_retraction_ply(si);
      break;

    case STRetroPlayNullMove:
      retro_play_null_move(si);
      break;

    case STRetroRetractLastCapture:
      circe_parrain_undo_retro_capture(si);
      break;

    case STRetroRedoLastCapture:
      circe_parrain_redo_retro_capture(si);
      break;

    case STRetroUndoLastPawnMultistep:
      en_passant_undo_multistep(si);
      break;

    case STRetroRedoLastPawnMultistep:
      en_passant_redo_multistep(si);
      break;

    case STRetroStartRetroMovePly:
      retro_start_retro_move_ply(si);
      break;

    case STMaxSolutionsInitialiser:
      maxsolutions_initialiser_solve(si);
      break;

    case STStopOnShortSolutionsInitialiser:
      stoponshortsolutions_initialiser_solve(si);
      break;

    case STOutputPlaintextTwinIntroWriter:
      output_plaintext_write_twin_intro(si);
      break;

    case STOutputPlainTextPositionWriter:
      output_plaintext_write_position(si);
      break;

    case STOutputEndOfIntro:
      output_end_of_intro(si);
      break;

    case STIllegalSelfcheckWriter:
      illegal_selfcheck_writer_solve(si);
      break;

    case STOutputPlainTextEndOfPhaseWriter:
      output_plaintext_end_of_phase_writer_solve(si);
      break;

    case STOutputPlaintextMoveInversionCounter:
    case STOutputPlaintextMoveInversionCounterSetPlay:
      output_plaintext_move_inversion_counter_solve(si);
      break;

    case STOutputPlaintextLineEndOfIntroSeriesMarker:
      output_plaintext_line_end_of_intro_series_marker_solve(si);
      break;

    case STPiecesParalysingMateFilter:
      paralysing_mate_filter_solve(si);
      break;

    case STPiecesParalysingStalemateSpecial:
      paralysing_stalemate_special_solve(si);
      break;

    case STPiecesParalysingRemoveCaptures:
      paralysing_remove_captures_solve(si);
      break;

    case STPiecesParalysingSuffocationFinder:
      paralysing_suffocation_finder_solve(si);
      break;

    case STAmuMateFilter:
      amu_mate_filter_solve(si);
      break;

    case STCirceSteingewinnFilter:
      circe_steingewinn_filter_solve(si);
      break;

    case STCirceCircuitSpecial:
      circe_circuit_special_solve(si);
      break;

    case STCirceExchangeSpecial:
      circe_exchange_special_solve(si);
      break;

    case STTemporaryHackFork:
      pipe_solve_delegate(si);
      break;

    case STGoalTargetReachedTester:
      goal_target_reached_tester_solve(si);
      break;

    case STGoalCheckReachedTester:
      goal_check_reached_tester_solve(si);
      break;

    case STGoalCaptureReachedTester:
      goal_capture_reached_tester_solve(si);
      break;

    case STGoalSteingewinnReachedTester:
      goal_steingewinn_reached_tester_solve(si);
      break;

    case STGoalEnpassantReachedTester:
      goal_enpassant_reached_tester_solve(si);
      break;

    case STGoalDoubleMateReachedTester:
      goal_doublemate_reached_tester_solve(si);
      break;

    case STGoalCounterMateReachedTester:
      goal_countermate_reached_tester_solve(si);
      break;

    case STGoalCastlingReachedTester:
      goal_castling_reached_tester_solve(si);
      break;

    case STGoalCircuitReachedTester:
      goal_circuit_reached_tester_solve(si);
      break;

    case STGoalExchangeReachedTester:
      goal_exchange_reached_tester_solve(si);
      break;

    case STGoalCircuitByRebirthReachedTester:
      goal_circuit_by_rebirth_reached_tester_solve(si);
      break;

    case STGoalExchangeByRebirthReachedTester:
      goal_exchange_by_rebirth_reached_tester_solve(si);
      break;

    case STGoalProofgameReachedTester:
    case STGoalAToBReachedTester:
      goal_proofgame_reached_tester_solve(si);
      break;

    case STGoalImmobileReachedTester:
      goal_immobile_reached_tester_solve(si);
      break;

    case STMaffImmobilityTesterKing:
      maff_immobility_tester_king_solve(si);
      break;

    case STOWUImmobilityTesterKing:
      owu_immobility_tester_king_solve(si);
      break;

    case STGoalNotCheckReachedTester:
      goal_notcheck_reached_tester_solve(si);
      break;

    case STGoalAnyReachedTester:
      goal_any_reached_tester_solve(si);
      break;

    case STGoalChess81ReachedTester:
      goal_chess81_reached_tester_solve(si);
      break;

    case STGoalKissReachedTester:
      goal_kiss_reached_tester_solve(si);
      break;

    case STGoalKingCaptureReachedTester:
      goal_king_capture_reached_tester_solve(si);
      break;

    case STPiecesParalysingMateFilterTester:
      paralysing_mate_filter_tester_solve(si);
      break;

    case STBlackChecks:
      blackchecks_solve(si);
      break;

    case STSingleBoxType1LegalityTester:
      singlebox_type1_legality_tester_solve(si);
      break;

    case STSingleBoxType2LegalityTester:
      singlebox_type2_legality_tester_solve(si);
      break;

    case STSingleBoxType3LegalityTester:
      singlebox_type3_legality_tester_solve(si);
      break;

    case STSingleBoxType3PawnPromoter:
      singlebox_type3_pawn_promoter_solve(si);
      break;

    case STExclusiveChessExclusivityDetector:
      exclusive_chess_exclusivity_detector_solve(si);
      break;

    case STExclusiveChessNestedExclusivityDetector:
      exclusive_chess_nested_exclusivity_detector_solve(si);
      break;

    case STExclusiveChessLegalityTester:
      exclusive_chess_legality_tester_solve(si);
      break;

    case STExclusiveChessGoalReachingMoveCounter:
      exclusive_chess_goal_reaching_move_counter_solve(si);
      break;

    case STExclusiveChessUndecidableWriterTree:
      exclusive_chess_undecidable_writer_tree_solve(si);
      break;

    case STExclusiveChessUndecidableWriterLine:
      exclusive_chess_undecidable_writer_line_solve(si);
      break;

    case STUltraschachzwangLegalityTester:
      ultraschachzwang_legality_tester_solve(si);
      break;

    case STIsardamLegalityTester:
      isardam_legality_tester_solve(si);
      break;

    case STCirceAssassinAssassinate:
      circe_assassin_assassinate_solve(si);
      break;

    case STCirceParachuteRemember:
      circe_parachute_remember_solve(si);
      break;

    case STCirceParachuteUncoverer:
      circe_parachute_uncoverer_solve(si);
      break;

    case STCirceVolcanicRemember:
      circe_volcanic_remember_solve(si);
      break;

    case STCirceVolcanicSwapper:
      circe_volcanic_swapper_solve(si);
      break;

    case STKingSquareUpdater:
      king_square_updater_solve(si);
      break;

    case STOwnKingCaptureAvoider:
      own_king_capture_avoider_solve(si);
      break;

    case STOpponentKingCaptureAvoider:
      opponent_king_capture_avoider_solve(si);
      break;

    case STPatienceChessLegalityTester:
      patience_chess_legality_tester_solve(si);
      break;

    case STStrictSATInitialiser:
      strict_sat_initialiser_solve(si);
      break;

    case STStrictSATUpdater:
      strict_sat_updater_solve(si);
      break;

    case STDynastyKingSquareUpdater:
      dynasty_king_square_updater_solve(si);
      break;

    case STHurdleColourChanger:
      hurdle_colour_changer_solve(si);
      break;

    case STHurdleColourChangeInitialiser:
      hurdle_colour_change_initialiser_solve(si);
      break;

    case STHurdleColourChangerChangePromoteeInto:
      hurdle_colour_change_change_promotee_into_solve(si);
      break;

    case STOscillatingKingsTypeA:
      oscillating_kings_type_a_solve(si);
      break;

    case STOscillatingKingsTypeB:
      oscillating_kings_type_b_solve(si);
      break;

    case STOscillatingKingsTypeC:
      oscillating_kings_type_c_solve(si);
      break;

    case STPlaySuppressor:
      play_suppressor_solve(si);
      break;

    case STContinuationSolver:
      continuation_solver_solve(si);
      break;

    case STDefensePlayed:
      defense_played_solve(si);
      break;

    case STMaxFlightsquares:
      maxflight_guard_solve(si);
      break;

    case STMaxNrNonTrivial:
      max_nr_nontrivial_guard_solve(si);
      break;

    case STOutputPlainTextTreeEndOfSolutionWriter:
      output_plaintext_end_of_solution_writer_solve(si);
      break;

    case STKillerMoveFinalDefenseMove:
      killer_move_final_defense_move_solve(si);
      break;

    case STKillerMovePrioriser:
      killer_move_prioriser_solve(si);
      break;

    case STMaxThreatLength:
      maxthreatlength_guard_solve(si);
      break;

    case STKillerAttackCollector:
      killer_attack_collector_solve(si);
      break;

    case STMummerOrchestrator:
      mummer_orchestrator_solve(si);
      break;

    case STMummerBookkeeper:
      mummer_bookkeeper_solve(si);
      break;

    case STBackHomeMovesOnly:
      back_home_moves_only_solve(si);
      break;

    case STNoPromotionsRemovePromotionMoving:
      nopromotion_avoid_promotion_moving_solve(si);
      break;

    case STBackhomeExistanceTester:
      backhome_existance_tester_solve(si);
      break;

    case STBackhomeRemoveIllegalMoves:
      backhome_remove_illegal_moves_solve(si);
      break;

    case STNocaptureRemoveCaptures:
      nocapture_remove_captures_solve(si);
      break;

    case STWoozlesRemoveIllegalCaptures:
      woozles_remove_illegal_captures_solve(si);
      break;

    case STBiWoozlesRemoveIllegalCaptures:
      biwoozles_remove_illegal_captures_solve(si);
      break;

    case STHeffalumpsRemoveIllegalCaptures:
      heffalumps_remove_illegal_captures_solve(si);
      break;

    case STBiHeffalumpsRemoveIllegalCaptures:
      biheffalumps_remove_illegal_captures_solve(si);
      break;

    case STProvocateursRemoveUnobservedCaptures:
      provocateurs_remove_unobserved_captures_solve(si);
      break;

    case STLortapRemoveSupportedCaptures:
      lortap_remove_supported_captures_solve(si);
      break;

    case STPatrolRemoveUnsupportedCaptures:
      patrol_remove_unsupported_captures_solve(si);
      break;

    case STKoekoLegalityTester:
      koeko_legality_tester_solve(si);
      break;

    case STGridContactLegalityTester:
      contact_grid_legality_tester_solve(si);
      break;

    case STAntiKoekoLegalityTester:
      antikoeko_legality_tester_solve(si);
      break;

    case STNewKoekoRememberContact:
      newkoeko_remember_contact_solve(si);
      break;

    case STNewKoekoLegalityTester:
      newkoeko_legality_tester_solve(si);
      break;

    case STJiggerLegalityTester:
      jigger_legality_tester_solve(si);
      break;

    case STMonochromeRemoveBichromeMoves:
      monochrome_remove_bichrome_moves_solve(si);
      break;

    case STBichromeRemoveMonochromeMoves:
      bichrome_remove_monochrome_moves_solve(si);
      break;

    case STEdgeMoverRemoveIllegalMoves:
      edgemover_remove_illegal_moves_solve(si);
      break;

    case STShieldedKingsRemoveIllegalCaptures:
      shielded_kings_remove_illegal_captures_solve(si);
      break;

    case STSuperguardsRemoveIllegalCaptures:
      superguards_remove_illegal_captures_solve(si);
      break;

    case STGridRemoveIllegalMoves:
      grid_remove_illegal_moves_solve(si);
      break;

    case STUncapturableRemoveCaptures:
      uncapturable_remove_captures_solve(si);
      break;

    case STTakeAndMakeGenerateMake:
      take_and_make_generate_make_solve(si);
      break;

    case STTakeAndMakeAvoidPawnMakeToBaseLine:
      take_and_make_avoid_pawn_make_to_base_line_solve(si);
      break;

    case STWormholeRemoveIllegalCaptures:
      wormhole_remove_illegal_captures_solve(si);
      break;

    case STWormholeTransferer:
      wormhole_transferer_solve(si);
      break;

    case STExtinctionExtinctedTester:
      extinction_extincted_tester_solve(si);
      break;

    case STCastlingGeneratorTestDeparture:
      castling_generate_test_departure(si);
      break;

    case STBlackChecksNullMoveGenerator:
      black_checks_null_move_generator_solve(si);
      break;

    case STTrue:
      solve_result = previous_move_has_solved;
      break;

    case STFalse:
      solve_result = MOVE_HAS_NOT_SOLVED_LENGTH();
      break;

    /* move generators: */
    case STSingleBoxType3TMovesForPieceGenerator:
      singleboxtype3_generate_moves_for_piece(si);
      break;

    case STMadrasiMovesForPieceGenerator:
      madrasi_generate_moves_for_piece(si);
      break;

    case STEiffelMovesForPieceGenerator:
      eiffel_generate_moves_for_piece(si);
      break;

    case STDisparateMovesForPieceGenerator:
      disparate_generate_moves_for_piece(si);
      break;

    case STParalysingMovesForPieceGenerator:
      paralysing_generate_moves_for_piece(si);
      break;

    case STUltraPatrolMovesForPieceGenerator:
      ultrapatrol_generate_moves_for_piece(si);
      break;

    case STCentralMovesForPieceGenerator:
      central_generate_moves_for_piece(si);
      break;

    case STBeamtenMovesForPieceGenerator:
      beamten_generate_moves_for_piece(si);
      break;

    case STMarsCirceMoveGeneratorEnforceRexInclusive:
      marscirce_generate_moves_enforce_rex_exclusive(si);
      break;

    case STPhantomAvoidDuplicateMoves:
      phantom_avoid_duplicate_moves(si);
      break;

    case STPlusAdditionalCapturesForPieceGenerator:
      plus_generate_additional_captures_for_piece(si);
      break;

    case STMoveForPieceGeneratorTwoPaths:
      generate_moves_for_piece_two_paths(si);
      break;

    case STMarsCirceRememberRebirth:
      marscirce_remember_rebirth(si);
      break;

    case STMarsCirceRememberNoRebirth:
      marscirce_remember_no_rebirth(si);
      break;

    case STMarsCirceFixDeparture:
      marscirce_fix_departure(si);
      break;

    case STMarsCirceGenerateFromRebirthSquare:
      marscirce_generate_from_rebirth_square(si);
      break;

    case STMoveGeneratorRejectCaptures:
      move_generation_reject_captures(si);
      break;

    case STMoveGeneratorRejectNoncaptures:
      move_generation_reject_non_captures(si);
      break;

    case STVaultingKingsMovesForPieceGenerator:
      vaulting_kings_generate_moves_for_piece(si);
      break;

    case STTransmutingKingsMovesForPieceGenerator:
      transmuting_kings_generate_moves_for_piece(si);
      break;

    case STSuperTransmutingKingsMovesForPieceGenerator:
      supertransmuting_kings_generate_moves_for_piece(si);
      break;

    case STReflectiveKingsMovesForPieceGenerator:
      reflective_kings_generate_moves_for_piece(si);
      break;

    case STCastlingChessMovesForPieceGenerator:
      castlingchess_generate_moves_for_piece(si);
      break;

    case STPlatzwechselRochadeMovesForPieceGenerator:
      exchange_castling_generate_moves_for_piece(si);
      break;

    case STCastlingGenerator:
      castling_generator_generate_castling(si);
      break;

    case STMessignyMovesForPieceGenerator:
      messigny_generate_moves_for_piece(si);
      break;

    case STAnnanMovesForPieceGenerator:
      annan_generate_moves_for_piece(si);
      break;

    case STFaceToFaceMovesForPieceGenerator:
      facetoface_generate_moves_for_piece(si);
      break;

    case STBackToBackMovesForPieceGenerator:
      backtoback_generate_moves_for_piece(si);
      break;

    case STCheekToCheekMovesForPieceGenerator:
      cheektocheek_generate_moves_for_piece(si);
      break;

    case STMovesForPieceBasedOnWalkGenerator:
      generate_moves_for_piece_based_on_walk();
      break;

    case STMoveGenerationPostMoveIterator:
      move_generation_post_move_iterator_solve(si);
      break;

    /* square observation */
    case STIsSquareObservedOrtho:
      observation_result = is_square_observed_ortho(trait[nbply],
                                                    move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture);
      break;

    case STPlusIsSquareObserved:
      plus_is_square_observed(si);
      break;

    case STMarsIterateObservers:
      marscirce_iterate_observers(si);
      break;

    case STMarsCirceIsSquareObservedEnforceRexInclusive:
      marscirce_is_square_observed_enforce_rex_exclusive(si);
      break;

    case STMarsIsSquareObserved:
      marscirce_is_square_observed(si);
      break;

    case STVaultingKingIsSquareObserved:
      vaulting_king_is_square_observed(si);
      break;

    case STTransmutingKingIsSquareObserved:
      transmuting_king_is_square_observed(si);
      break;

    case STTransmutingKingDetectNonTransmutation:
      transmuting_king_detect_non_transmutation(si);
      break;

    case STDetermineObserverWalk:
      determine_observer_walk(si);
      break;

    case STBicoloresTryBothSides:
      bicolores_try_both_sides(si);
      break;

    case STDontTryObservingWithNonExistingWalk:
      dont_try_observing_with_non_existing_walk(si);
      break;

    case STDontTryObservingWithNonExistingWalkBothSides:
      dont_try_observing_with_non_existing_walk_both_sides(si);
      break;

    case STOptimiseObservationsByQueenInitialiser:
      optimise_away_observations_by_queen_initialise(si);
      break;

    case STOptimiseObservationsByQueen:
      optimise_away_observations_by_queen(si);
      break;

    case STTrackBackFromTargetAccordingToObserverWalk:
      track_back_from_target_according_to_observer_walk(si);
      break;

    case STIsSquareObservedTwoPaths:
      is_square_observed_two_paths(si);
      break;

    case STSquareObservationPostMoveIterator:
      square_observation_post_move_iterator_solve(si);
      break;

    default:
      assert(0);
      break;
  }
}
