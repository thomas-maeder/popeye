#include "solving/machinery/solvers.h"
#include "solving/machinery/twin.h"
#include "pieces/pieces.h"
#include "options/options.h"
#include "conditions/conditions.h"
#include "retro/retro.h"
#include "solving/selfcheck_guard.h"
#include "solving/has_solution_type.h"
#include "solving/check.h"
#include "pieces/attributes/total_invisible.h"
#include "pieces/walks/pawns/promotion.h"
#include "stipulation/proxy.h"
#include "solving/observation.h"
#include "output/plaintext/message.h"
#include "conditions/annan.h"
#include "conditions/bgl.h"
#include "conditions/bolero.h"
#include "conditions/breton.h"
#include "conditions/dister.h"
#include "conditions/role_exchange.h"
#include "conditions/facetoface.h"
#include "conditions/koeko/contact_grid.h"
#include "conditions/koeko/koeko.h"
#include "conditions/koeko/anti.h"
#include "conditions/koeko/new.h"
#include "conditions/exclusive.h"
#include "conditions/republican.h"
#include "conditions/blackchecks.h"
#include "conditions/influencer.h"
#include "conditions/extinction.h"
#include "conditions/madrasi.h"
#include "conditions/partial_paralysis.h"
#include "conditions/maff/immobility_tester.h"
#include "conditions/owu/immobility_tester.h"
#include "conditions/ohneschach.h"
#include "conditions/pointreflection.h"
#include "conditions/singlebox/type1.h"
#include "conditions/singlebox/type2.h"
#include "conditions/singlebox/type3.h"
#include "conditions/patience.h"
#include "conditions/isardam.h"
#include "conditions/masand.h"
#include "conditions/disparate.h"
#include "conditions/dynasty.h"
#include "conditions/eiffel.h"
#include "conditions/ultraschachzwang/legality_tester.h"
#include "conditions/sat.h"
#include "conditions/oscillating_kings.h"
#include "conditions/messigny.h"
#include "conditions/actuated_revolving_centre.h"
#include "conditions/actuated_revolving_board.h"
#include "conditions/circe/circe.h"
#include "conditions/circe/rex_inclusive.h"
#include "conditions/circe/rebirth_avoider.h"
#include "conditions/circe/kamikaze.h"
#include "conditions/anticirce/anticirce.h"
#include "conditions/anticirce/magic_square.h"
#include "conditions/sentinelles.h"
#include "conditions/snek.h"
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
#include "conditions/marscirce/phantom.h"
#include "conditions/marscirce/marscirce.h"
#include "conditions/marscirce/anti.h"
#include "conditions/marscirce/plus.h"
#include "conditions/line_chameleon.h"
#include "conditions/haan.h"
#include "conditions/castling_chess.h"
#include "conditions/exchange_castling.h"
#include "conditions/transmuting_kings/super.h"
#include "conditions/transmuting_kings/reflective_kings.h"
#include "conditions/amu/attack_counter.h"
#include "conditions/imitator.h"
#include "conditions/football.h"
#include "conditions/castling_chess.h"
#include "conditions/mummer.h"
#include "conditions/woozles.h"
#include "conditions/immune.h"
#include "conditions/provocateurs.h"
#include "conditions/lortap.h"
#include "conditions/patrol.h"
#include "conditions/nocapture.h"
#include "conditions/nopromotion.h"
#include "conditions/geneva.h"
#include "conditions/monochrome.h"
#include "conditions/bichrome.h"
#include "conditions/edgemover.h"
#include "conditions/grid.h"
#include "conditions/take_and_make.h"
#include "conditions/make_and_take.h"
#include "conditions/superguards.h"
#include "conditions/central.h"
#include "conditions/beamten.h"
#include "conditions/eiffel.h"
#include "conditions/madrasi.h"
#include "conditions/brunner.h"
#include "conditions/wormhole.h"
#include "conditions/transmuting_kings/transmuting_kings.h"
#include "conditions/transmuting_kings/vaulting_kings.h"
#include "conditions/backhome.h"
#include "conditions/vogtlaender.h"
#include "conditions/bicolores.h"
#include "conditions/antikings.h"
#include "conditions/follow_my_leader.h"
#include "conditions/alphabetic.h"
#include "conditions/forced_squares.h"
#include "conditions/synchronous.h"
#include "conditions/must_capture.h"
#include "conditions/lostpieces.h"
#include "conditions/series_capture.h"
#include "platform/maxtime.h"
#include "conditions/shielded_kings.h"
#include "solving/end_of_branch_tester.h"
#include "solving/dead_end.h"
#include "stipulation/constraint.h"
#include "solving/temporary_hacks.h"
#include "stipulation/goals/immobile/reached_tester.h"
#include "stipulation/goals/kiss/reached_tester.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/fork.h"
#include "stipulation/move.h"
#include "solving/duplex.h"
#include "solving/pipe.h"
#include "solving/king_capture_avoider.h"
#include "solving/testers.h"
#include "solving/find_shortest.h"
#include "solving/for_each_move.h"
#include "solving/battle_play/try.h"
#include "solving/battle_play/threat.h"
#include "solving/battle_play/continuation.h"
#include "solving/battle_play/setplay.h"
#include "solving/battle_play/min_length_guard.h"
#include "solving/boolean/and.h"
#include "solving/goals/prerequisite_guards.h"
#include "solving/trivial_end_filter.h"
#include "solving/avoid_unsolvable.h"
#include "solving/play_suppressor.h"
#include "solving/castling.h"
#include "solving/machinery/intro.h"
#include "pieces/walks/pawns/en_passant.h"
#include "pieces/walks/pawns/promotion.h"
#include "solving/post_move_iteration.h"
#include "pieces/attributes/magic.h"
#include "pieces/attributes/paralysing/paralysing.h"
#include "pieces/attributes/kamikaze/kamikaze.h"
#include "pieces/attributes/neutral/neutral.h"
#include "pieces/attributes/neutral/half.h"
#include "pieces/attributes/hurdle_colour_changing.h"
#include "pieces/attributes/chameleon.h"
#include "pieces/attributes/chameleon.h"
#include "pieces/attributes/jigger.h"
#include "pieces/attributes/uncapturable.h"
#include "pieces/walks/hunters.h"
#include "conditions/amu/mate_filter.h"
#include "conditions/circe/goal_filters.h"
#include "pieces/attributes/chameleon.h"
#include "conditions/ultraschachzwang/goal_filter.h"
#include "options/no_short_variations/no_short_variations.h"
#include "optimisations/hash.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/keepmating.h"
#include "optimisations/killer_move/killer_move.h"
#include "optimisations/ohneschach/non_checking_first.h"
#include "optimisations/ohneschach/redundant_immobility_tests.h"
#include "optimisations/ohneschach/immobility_tests_help.h"
#include "options/maxtime.h"
#include "options/degenerate_tree.h"
#include "options/maxthreatlength.h"
#include "options/maxflightsquares.h"
#include "options/nontrivial.h"
#include "options/movenumbers.h"
#include "options/nullmoves.h"
#include "optimisations/count_nr_opponent_moves/move_generator.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_moves_generation.h"
#include "optimisations/intelligent/limit_nr_solutions_per_target.h"
#include "optimisations/goals/remove_non_reachers.h"
#include "optimisations/detect_retraction.h"
#include "optimisations/observation.h"
#include "debugging/assert.h"
#include "debugging/trace.h"
#include "debugging/measure.h"

/* Instrument the slices representing the stipulation with solving slices
 * @param solving_machinery proxy slice into the solving machinery to be built
 */
void build_solvers1(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  goal_prerequisite_guards_initialse_solving(si);

  insert_temporary_hacks(si);

  /* must come before stip_insert_selfcheck_guards() and
   * solving_insert_move_generators() because flight counting machinery needs
   * selfcheck guards and move generators */
  if (OptFlag[solflights])
    solving_insert_maxflight_guards(si);

  /* must come before stip_insert_selfcheck_guards() because the
   * instrumentation of the goal filters inserts or slices of
   * which both branches need selfcheck guards */
  if (CondFlag[circe])
    solving_insert_circe_goal_filters(si);
  if (TSTFLAG(some_pieces_flags,Kamikaze))
    solving_insert_kamikaze(si);

  if (total_invisible_number>0)
    solving_instrument_total_invisible(si);

  /* must come before solving_apply_setplay() */
  solving_insert_root_slices(si);
  solving_insert_intro_slices(si);

  /* must come before stip_insert_selfcheck_guards() because the set play
   * branch needs a selfcheck guard */
  if (OptFlag[solapparent] && !OptFlag[restart]
      && !solving_apply_setplay(si))
    output_plaintext_message(SetPlayNotApplicable);

  retro_instrument_solving_default(si);

  if (CondFlag[lastcapture])
  {
    retro_instrument_retractor(si,STRetroRetractLastCapture);
    retro_substitute_last_move_player(si,STRetroRedoLastCapture);
  }
  else if (OptFlag[enpassant])
  {
    retro_instrument_retractor(si,STRetroUndoLastPawnMultistep);
    retro_substitute_last_move_player(si,STRetroRedoLastPawnMultistep);
  }

  /* must come before stip_insert_move_generators() because immobilise_black
   * needs a move generator */
  if (!init_intelligent_mode(si))
    output_plaintext_message(IntelligentRestricted);

  /* must come here because we generate branches that have to be provided with
   * self-check guards and move generators
   */
  if (CondFlag[ohneschach])
    ohneschach_insert_check_guards(si);

  /* must come here because in conditions like MAFF, we are going to tamper with
   * the slices inserted here
   */
  solving_insert_selfcheck_guards(si);
  solving_insert_move_generators(si);

  if (OptFlag[keepmating])
    solving_insert_keepmating_filters(si);

  if (CondFlag[amu])
    solving_insert_amu_mate_filters(si);

  if (CondFlag[whiteultraschachzwang]
      || CondFlag[blackultraschachzwang])
    ultraschachzwang_initialise_solving(si);

  if (CondFlag[ohneschach])
  {
    /* prevent king first optimisation - the Ohneschach specific optimisation
     * is more effective */
  }
  else if (CondFlag[whsupertrans_king] || CondFlag[blsupertrans_king])
  {
    /* prevent king first optimisation - doesn't work */
  }
  else if (CondFlag[exclusive])
    ; /* use regular move generation to filter out non-unique mating moves */
  else if (CondFlag[MAFF])
    maff_replace_immobility_testers(si);
  else if (CondFlag[OWU])
    owu_replace_immobility_testers(si);
  else
    immobility_testers_substitute_king_first(si);

  if (CondFlag[exclusive])
    optimise_away_unnecessary_selfcheckguards(si);

  if (CondFlag[singlebox])
    switch (SingleBoxType)
    {
      case ConditionType1:
        singlebox_type1_initialise_solving(si);
        break;

      case ConditionType2:
        stip_insert_singlebox_type2(si);
        break;

      case ConditionType3:
        stip_insert_singlebox_type3(si);
        break;

      default:
        break;
    }

  if (CondFlag[exclusive])
    solving_insert_exclusive_chess(si);

  solving_insert_king_capture_avoiders(si);

  if (CondFlag[isardam])
    solving_insert_isardam_legality_testers(si);

  if (CondFlag[patience])
    solving_insert_patience_chess(si);

  if (TSTFLAG(some_pieces_flags,Paralysing))
    paralysing_initialise_solving(si);

  if (CondFlag[strictSAT])
    strictsat_initialise_solving(si);
  else if (CondFlag[SAT])
    sat_initialise_solving(si);

  if (OptFlag[nullmoves])
    nullmoves_initialise_solving(si,no_side);

  if (CondFlag[role_exchange])
    role_exchange_initialise_solving(si,no_side);

  if (CondFlag[schwarzschacher])
    blackchecks_initialise_solving(si);

  if (CondFlag[masand])
    solving_insert_masand(si);

  if (CondFlag[influencer])
    solving_insert_influencer(si);

  if (CondFlag[masand_generalised])
    solving_insert_masand_generalised(si);

  if (CondFlag[dynasty])
    dynasty_initialise_solving(si);

  solving_insert_king_oscillators(si);

  if (CondFlag[messigny])
    solving_insert_messigny(si);

  if (CondFlag[arc])
    solving_insert_actuated_revolving_centre(si);

  if (CondFlag[actrevolving])
    solving_insert_actuated_revolving_board(si);

  if (CondFlag[circe])
  {
    circe_initialise_solving(si,&circe_variant,STMove,&move_insert_slices,STCirceConsideringRebirth);

    if (circe_variant.is_rex_inclusive)
      circe_rex_inclusive_initialise_check_validation(si);

    if (TSTFLAG(some_pieces_flags,Kamikaze))
      circe_kamikaze_initialise_solving(si);
  }

  if (CondFlag[sentinelles])
    solving_insert_sentinelles_inserters(si);

  if (CondFlag[anticirce])
  {
    anticirce_initialise_solving(si);

    if (CondFlag[magicsquare] && magic_square_type==ConditionType2)
      magic_square_type2_initialise_solving(si);
  }

  if (CondFlag[duellist])
    solving_insert_duellists(si);

  if (CondFlag[hauntedchess])
    solving_insert_haunted_chess(si);

  if (CondFlag[ghostchess])
    solving_insert_ghost_chess(si);

  if (kobul_who[White] || kobul_who[Black])
    solving_insert_kobul_king_substitutors(si);

  if (CondFlag[snekchess])
    solving_insert_snek_chess(si);
  if (CondFlag[snekcirclechess])
    solving_insert_snekcircle_chess(si);

  if (TSTFLAG(some_pieces_flags,HalfNeutral))
    solving_insert_half_neutral_recolorers(si);

  if (CondFlag[andernach])
    solving_insert_andernach(si);

  if (CondFlag[antiandernach])
    solving_insert_antiandernach(si);

  if (CondFlag[breton])
    solving_insert_breton(si);

  if (CondFlag[champursue])
    solving_insert_chameleon_pursuit(si);

  if (CondFlag[norsk])
    solving_insert_norsk_chess(si);

  if (CondFlag[protean] || TSTFLAG(some_pieces_flags,Protean))
    solving_insert_protean_chess(si);

  solving_initialise_castling(si);

  if (CondFlag[extinction])
  {
    castling_generation_test_departure(si);
    extinction_initialise_solving(si);
  }

  if (CondFlag[losingchess])
  {
    castling_generation_test_departure(si);
    solving_instrument_check_testing(si,STNoCheckConceptCheckTester);
  }

  if (CondFlag[einstein])
    solving_insert_einstein_moving_adjusters(si);

  if (CondFlag[reveinstein])
    solving_insert_reverse_einstein_moving_adjusters(si);

  if (CondFlag[antieinstein])
    anti_einstein_instrument_solving(si);

  if (CondFlag[einstein] || CondFlag[antieinstein] || CondFlag[reveinstein])
    solving_insert_einstein_en_passant_adjusters(si);

  if (CondFlag[traitor])
    solving_insert_traitor_side_changers(si);

  if (TSTFLAG(some_pieces_flags,Volage))
    solving_insert_volage_side_changers(si);

  if (CondFlag[magicsquare])
    solving_insert_magic_square(si);

  if (CondFlag[wormholes])
    wormhole_initialse_solving(si);

  if (CondFlag[dbltibet])
    solving_insert_double_tibet(si);
  else if (CondFlag[tibet])
    solving_insert_tibet(si);

  if (CondFlag[degradierung])
    solving_insert_degradierung(si);

  en_passant_initialise_solving(si);

  if (CondFlag[phantom] || CondFlag[mars] || CondFlag[plus] || CondFlag[antimars])
  {
    move_generator_instrument_for_alternative_paths(si,nr_sides);

    stip_instrument_moves(si,STMarsCirceMoveToRebirthSquare);
    move_effect_journal_register_pre_capture_effect();
  }

  if (CondFlag[phantom])
    solving_initialise_phantom(si);
  else if (CondFlag[plus])
    solving_initialise_plus(si);
  else
  {
    if (CondFlag[antimars])
      solving_initialise_antimars(si);
    if (CondFlag[mars])
      solving_initialise_marscirce(si);
  }

  if (CondFlag[maketake])
    solving_insert_make_and_take(si);

  if (CondFlag[linechamchess])
    solving_insert_line_chameleon_chess(si);

  if (CondFlag[chamchess])
    chameleon_chess_initialise_solving(si);

  if (CondFlag[series_capture])
    solving_instrument_series_capture(si);

  promotion_insert_slice_sequence(si,STMove,&move_insert_slices);
  promotion_instrument_solving_default(si);

  /* this has to come after all promotion_insert_slice_sequence() invokations
   * to support for promotion into Chameleon
   */
  if (TSTFLAG(some_pieces_flags,Chameleon))
    chameleon_initialise_solving(si);

  if ((CondFlag[chamchess] || TSTFLAG(some_pieces_flags,Chameleon))
      && !CondFlag[chameleonsequence])
    chameleon_init_sequence_implicit(&chameleon_walk_sequence);

  if (TSTFLAG(some_pieces_flags,ColourChange))
    solving_insert_hurdle_colour_changers(si);

  if (CondFlag[haanerchess])
    solving_insert_haan_chess(si);

  if (CondFlag[castlingchess] || CondFlag[rokagogo])
    solving_insert_castling_chess(si,CondFlag[rokagogo]);

  if (CondFlag[amu])
    solving_insert_amu_attack_counter(si);

  if (OptFlag[mutuallyexclusivecastling])
    solving_insert_mutual_castling_rights_adjusters(si);

  if (CondFlag[imitators])
    solving_insert_imitator(si);

  if (CondFlag[football])
    solving_insert_football_chess(si);

  if (CondFlag[platzwechselrochade])
    exchange_castling_initialise_solving(si);

  solving_insert_post_move_iteration(si);

  if (CondFlag[BGL])
    bgl_initialise_solving(si);

  if (TSTFLAG(some_pieces_flags,Patrol))
    patrol_initialise_solving(si);
  if (CondFlag[ultrapatrouille])
    ultrapatrol_initialise_solving(si);

  if (CondFlag[lortap])
    lortap_initialise_solving(si);

  if (CondFlag[provocateurs])
    provocateurs_initialise_solving(si);

  if (CondFlag[immun])
    immune_initialise_solving(si);

  if (CondFlag[woozles])
    woozles_initialise_solving(si);
  if (CondFlag[biwoozles])
    biwoozles_initialise_solving(si);
  if (CondFlag[heffalumps])
    heffalumps_initialise_solving(si);
  if (CondFlag[biheffalumps])
    biheffalumps_initialise_solving(si);

  if (CondFlag[nocapture] || CondFlag[nowhcapture] || CondFlag[noblcapture])
    solving_insert_nocapture(si);

  if (CondFlag[nowhiteprom] || CondFlag[noblackprom])
    solving_insert_nopromotions(si);

  if (CondFlag[geneva])
    geneva_initialise_solving(si);

  if (CondFlag[contactgrid])
    contact_grid_initialise_solving(si);
  else if (CondFlag[koeko])
    koeko_initialise_solving(si);
  else if (CondFlag[antikoeko])
    antikoeko_initialise_solving(si);
  else if (CondFlag[newkoeko])
    newkoeko_initialise_solving(si);

  if (TSTFLAG(some_pieces_flags,Jigger))
    jigger_initialise_solving(si);

  if (CondFlag[monochro])
      monochrome_initialise_solving(si);
  if (CondFlag[bichro])
      bichrome_initialise_solving(si);

  if (CondFlag[superguards])
    superguards_initialise_solving(si);

  if (CondFlag[whiteedge] || CondFlag[blackedge])
    solving_insert_edgemover(si);

  if (CondFlag[gridchess])
    grid_initialise_solving(si);

  if (TSTFLAG(some_pieces_flags,Uncapturable))
    solving_insert_uncapturable(si);

  if (CondFlag[takemake])
    solving_insert_take_and_make(si);

  if (OptFlag[noshort])
    solving_insert_no_short_variations_filters(si);

  solving_optimise_dead_end_slices(si);

  solving_remove_irrelevant_constraints(si);

  if (OptFlag[movenbr])
    solving_insert_restart_guards(si);

  solving_insert_continuation_solvers(si);

  solving_insert_find_shortest_solvers(si);

  if (total_invisible_number==0)
  {
    solving_optimise_with_orthodox_mating_move_generators(si);
    solving_optimise_with_goal_non_reacher_removers(si);
  }

  if (!OptFlag[solvariantes])
    solving_insert_play_suppressors(si);

  if (OptFlag[solvariantes] && !OptFlag[nothreat])
    solving_insert_threat_boundaries(si);

  solving_spin_off_testers(si);

  mummer_initialise_solving(si);

  if (CondFlag[blmaxdister] || CondFlag[blmindister] || CondFlag[whmaxdister] || CondFlag[whmindister])
    dister_initialise_solving(si);

  if (CondFlag[ohneschach])
  {
    ohneschach_optimise_away_redundant_immobility_tests(si);
    ohneschach_optimise_away_immobility_tests_help(si);
    ohneschach_optimise_immobility_testers(si);
  }

  if (total_invisible_number==0)
  {
    if (is_hashtable_allocated())
      solving_insert_hashing(si);
  }

  solving_instrument_help_ends_of_branches(si);

  solving_insert_setplay_solvers(si);

  if (OptFlag[soltout]) /* this includes OptFlag[solessais] */
    solving_insert_try_solvers(si);

  solving_insert_trivial_variation_filters(si);

  solving_insert_min_length(si);

  if (OptFlag[nontrivial])
    solving_insert_max_nr_nontrivial_guards(si);

  if (OptFlag[solvariantes] && !OptFlag[nothreat])
    solving_insert_threat_handlers(si);

  if (CondFlag[lostpieces])
    solving_insert_lostpieces(si);

  if (OptFlag[degeneratetree])
    solving_insert_degenerate_tree_guards(si);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the slices representing the stipulation with solving slices
 * @param solving_machinery proxy slice into the solving machinery to be built
 */
void build_solvers2(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (OptFlag[solmenaces]
      && !solving_insert_maxthreatlength_guards(si))
    output_plaintext_message(ThreatOptionAndExactStipulationIncompatible);

  if (CondFlag[republican])
    solving_insert_republican_king_placers(si);

  and_enable_shortcut_logic(si);

  solving_insert_avoid_unsolvable_forks(si);

  solving_insert_move_iterators(si);

  stip_instrument_moves(si,STKingSquareUpdater);

  if (TSTFLAG(some_pieces_flags,Magic))
    solving_insert_magic_pieces_recolorers(si);

  if (CondFlag[vogt])
    vogtlaender_initalise_solving(si);

  if (CondFlag[bicolores])
    bicolores_initalise_solving(si);

  if (CondFlag[antikings])
    antikings_initalise_solving(si);

  if (CondFlag[madras])
    madrasi_initialise_solving(si);

  if (CondFlag[partialparalysis])
    partial_paralysis_initialise_solving(si);

  solving_initialise_hunters(si);

  if (CondFlag[eiffel])
    eiffel_initialise_solving(si);

  if (CondFlag[disparate])
    disparate_initialise_solving(si);

  if (CondFlag[central])
    central_initialise_solving(si);

  if (TSTFLAG(some_pieces_flags,Beamtet))
    beamten_initialise_solving(si);

  if (CondFlag[whvault_king])
    vaulting_kings_initalise_solving(si,White);
  if (CondFlag[blvault_king])
    vaulting_kings_initalise_solving(si,Black);

  if (CondFlag[whsupertrans_king])
    supertransmuting_kings_initialise_solving(si,White);
  if (CondFlag[blsupertrans_king])
    supertransmuting_kings_initialise_solving(si,Black);

  if (CondFlag[whtrans_king])
    transmuting_kings_initialise_solving(si,White);
  if (CondFlag[bltrans_king])
    transmuting_kings_initialise_solving(si,Black);

  if (CondFlag[whrefl_king])
    reflective_kings_initialise_solving(si,White);
  if (CondFlag[blrefl_king])
    reflective_kings_initialise_solving(si,Black);

  if (CondFlag[annan])
    annan_initialise_solving(si);
  if (CondFlag[nanna])
    nanna_initialise_solving(si);

  if (CondFlag[bolero])
    solving_initialise_bolero(si);
  if (CondFlag[bolero_inverse])
    solving_initialise_bolero_inverse(si);

  if (CondFlag[pointreflection])
    point_reflection_initialise_solving(si);

#if defined(DOTRACE)
  solving_insert_move_tracers(si);
#endif

#if defined(DOMEASURE)
  solving_insert_move_counters(si);
#endif

  if (CondFlag[shieldedkings])
    shielded_kings_initialise_solving(si);

  if (CondFlag[brunner])
    brunner_initialise_solving(si);

  if (CondFlag[backhome])
    backhome_initialise_solving(si);

  if (CondFlag[facetoface])
    facetoface_initialise_solving(si);
  if (CondFlag[backtoback])
    backtoback_initialise_solving(si);
  if (CondFlag[cheektocheek])
    cheektocheek_initialise_solving(si);

  goal_kiss_init_piece_id(si);

  solving_impose_starter(si,SLICE_STARTER(si));
  solving_optimise_with_countnropponentmoves(si);

  solving_optimise_with_killer_moves(si);

  if (is_piece_neutral(some_pieces_flags))
    solving_optimise_by_detecting_retracted_moves(si);

  optimise_is_square_observed(si);
  optimise_is_in_check(si);

  solving_impose_starter(si,SLICE_STARTER(si));

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
