#include "solving/machinery/solvers.h"
#include "solving/machinery/twin.h"
#include "pieces/pieces.h"
#include "options/options.h"
#include "conditions/conditions.h"
#include "solving/selfcheck_guard.h"
#include "solving/has_solution_type.h"
#include "solving/check.h"
#include "pieces/walks/pawns/promotion.h"
#include "stipulation/proxy.h"
#include "solving/observation.h"
#include "output/output.h"
#include "output/plaintext/message.h"
#include "conditions/annan.h"
#include "conditions/bgl.h"
#include "conditions/facetoface.h"
#include "conditions/koeko/contact_grid.h"
#include "conditions/koeko/koeko.h"
#include "conditions/koeko/anti.h"
#include "conditions/koeko/new.h"
#include "conditions/exclusive.h"
#include "conditions/republican.h"
#include "conditions/blackchecks.h"
#include "conditions/extinction.h"
#include "conditions/madrasi.h"
#include "conditions/maff/immobility_tester.h"
#include "conditions/owu/immobility_tester.h"
#include "conditions/ohneschach.h"
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
#include "options/maxsolutions/maxsolutions.h"
#include "options/stoponshortsolutions/stoponshortsolutions.h"
#include "optimisations/count_nr_opponent_moves/move_generator.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_moves_generation.h"
#include "optimisations/intelligent/limit_nr_solutions_per_target.h"
#include "optimisations/goals/remove_non_reachers.h"
#include "optimisations/detect_retraction.h"
#include "optimisations/observation.h"
#include "debugging/assert.h"
#include "debugging/trace.h"
#include "debugging/measure.h"

stip_length_type slack_length = 0;

static void adjust_branch(slice_index si, stip_structure_traversal *st)
{
  int const * const diff = st->param;

  stip_traverse_structure_children(si,st);

  SLICE_U(si).branch.length += *diff;
  SLICE_U(si).branch.min_length += *diff;
}

void adjust_slack_length(slice_index si, stip_length_type to)
{
  int diff = (int)to-slack_length;

  stip_structure_traversal st;
  stip_structure_traversal_init(&st,&diff);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_branch,
                                                 &adjust_branch);
  stip_traverse_structure(si,&st);

  slack_length = to;
}

/* Instrument the slices representing the stipulation with solving slices
 * @param stipulation_root_hook proxy slice into stipulation
 * @return a copy of the stipulation instrumented with solvers
 */
slice_index build_solvers(slice_index stipulation_root_hook)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",stipulation_root_hook);
  TraceFunctionParamListEnd();

  result = stip_deep_copy(stipulation_root_hook);

  solving_impose_starter(result,SLICE_STARTER(stipulation_root_hook));

  {
    slice_index const protos[] = {
        alloc_pipe(STRetroRetractor),
        alloc_pipe(STRetroRetractLastCapture),
        alloc_pipe(STRetroInitialiser),
        alloc_pipe(STRetroRedoLastCapture)
    };
    enum { nr_prototypes = sizeof protos / sizeof protos[0] };
    slice_insertion_insert(result,protos,nr_prototypes);
  }

  goal_prerequisite_guards_initialse_solving(result);

  if (!OptFlag[noboard] && twin_duplex_type!=twin_is_duplex)
    pipe_append(result,alloc_pipe(STOutputPlainTextPositionWriter));

  insert_temporary_hacks(result);

  /* must come before stip_insert_selfcheck_guards() and
   * stip_insert_move_generators() because flight counting machinery needs
   * selfcheck guards and move generators */
  if (OptFlag[solflights])
    solving_insert_maxflight_guards(result);

  /* must come before stip_insert_selfcheck_guards() because the
   * instrumentation of the goal filters inserts or slices of
   * which both branches need selfcheck guards */
  if (CondFlag[circe])
    solving_insert_circe_goal_filters(result);
  if (TSTFLAG(some_pieces_flags,Kamikaze))
    solving_insert_kamikaze(result);

  /* must come before solving_apply_setplay() */
  solving_insert_root_slices(result);
  solving_insert_intro_slices(result);

  /* must come before stip_insert_selfcheck_guards() because the set play
   * branch needs a selfcheck guard */
  if (OptFlag[solapparent] && !OptFlag[restart]
      && !solving_apply_setplay(result))
    output_plaintext_message(SetPlayNotApplicable);

  /* must come before stip_insert_move_generators() because immobilise_black
   * needs a move generator */
  if (!init_intelligent_mode(result))
    output_plaintext_message(IntelligentRestricted);

  /* must come here because we generate branches that have to be provided with
   * self-check guards and move generators
   */
  if (CondFlag[ohneschach])
    ohneschach_insert_check_guards(result);

  /* must come here because in conditions like MAFF, we are going to tamper with
   * the slices inserted here
   */
  solving_insert_selfcheck_guards(result);
  solving_insert_move_generators(result);

  if (OptFlag[keepmating])
    solving_insert_keepmating_filters(result);

  if (CondFlag[amu])
    solving_insert_amu_mate_filters(result);

  if (CondFlag[whiteultraschachzwang]
      || CondFlag[blackultraschachzwang])
    ultraschachzwang_initialise_solving(result);

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
    maff_replace_immobility_testers(result);
  else if (CondFlag[OWU])
    owu_replace_immobility_testers(result);
  else
    immobility_testers_substitute_king_first(result);

  if (CondFlag[exclusive])
    optimise_away_unnecessary_selfcheckguards(result);

  if (CondFlag[singlebox])
    switch (SingleBoxType)
    {
      case ConditionType1:
        singlebox_type1_initialise_solving(result);
        break;

      case ConditionType2:
        stip_insert_singlebox_type2(result);
        break;

      case ConditionType3:
        stip_insert_singlebox_type3(result);
        break;

      default:
        break;
    }

  if (CondFlag[exclusive])
    solving_insert_exclusive_chess(result);

  solving_insert_king_capture_avoiders(result);

  if (CondFlag[isardam])
    solving_insert_isardam_legality_testers(result);

  if (CondFlag[patience])
    solving_insert_patience_chess(result);

  if (TSTFLAG(some_pieces_flags,Paralysing))
    paralysing_initialise_solving(result);

  if (CondFlag[strictSAT])
    strictsat_initialise_solving(result);
  else if (CondFlag[SAT])
    sat_initialise_solving(result);

  if (CondFlag[schwarzschacher])
    blackchecks_initialise_solving(result);

  if (CondFlag[masand])
    solving_insert_masand(result);

  if (CondFlag[dynasty])
    dynasty_initialise_solving(result);

  solving_insert_king_oscillators(result);

  if (CondFlag[messigny])
    solving_insert_messigny(result);

  if (CondFlag[arc])
    solving_insert_actuated_revolving_centre(result);

  if (CondFlag[actrevolving])
    solving_insert_actuated_revolving_board(result);

  if (CondFlag[circe])
  {
    circe_initialise_solving(result,&circe_variant,STMove,&move_insert_slices,STCirceConsideringRebirth);

    if (circe_variant.is_rex_inclusive)
      circe_rex_inclusive_initialise_check_validation(result);

    if (TSTFLAG(some_pieces_flags,Kamikaze))
      circe_kamikaze_initialise_solving(result);
  }

  if (CondFlag[sentinelles])
    solving_insert_sentinelles_inserters(result);

  if (CondFlag[anticirce])
  {
    anticirce_initialise_solving(result);

    if (CondFlag[magicsquare] && magic_square_type==ConditionType2)
      magic_square_type2_initialise_solving(result);
  }

  if (CondFlag[duellist])
    solving_insert_duellists(result);

  if (CondFlag[hauntedchess])
    solving_insert_haunted_chess(result);

  if (CondFlag[ghostchess])
    solving_insert_ghost_chess(result);

  if (kobul_who[White] || kobul_who[Black])
    solving_insert_kobul_king_substitutors(result);

  if (CondFlag[snekchess])
    solving_insert_snek_chess(result);
  if (CondFlag[snekcirclechess])
    solving_insert_snekcircle_chess(result);

  if (TSTFLAG(some_pieces_flags,HalfNeutral))
    solving_insert_half_neutral_recolorers(result);

  if (CondFlag[andernach])
    solving_insert_andernach(result);

  if (CondFlag[antiandernach])
    solving_insert_antiandernach(result);

  if (CondFlag[champursue])
    solving_insert_chameleon_pursuit(result);

  if (CondFlag[norsk])
    solving_insert_norsk_chess(result);

  if (CondFlag[protean] || TSTFLAG(some_pieces_flags,Protean))
    solving_insert_protean_chess(result);

  solving_initialise_castling(result);

  if (CondFlag[extinction])
  {
    castling_generation_test_departure(result);
    extinction_initialise_solving(result);
  }

  if (CondFlag[losingchess])
  {
    castling_generation_test_departure(result);
    solving_instrument_check_testing(result,STNoCheckConceptCheckTester);
  }

  if (CondFlag[einstein])
    solving_insert_einstein_moving_adjusters(result);

  if (CondFlag[reveinstein])
    solving_insert_reverse_einstein_moving_adjusters(result);

  if (CondFlag[antieinstein])
    anti_einstein_instrument_solving(result);

  if (CondFlag[einstein] || CondFlag[antieinstein] || CondFlag[reveinstein])
    solving_insert_einstein_en_passant_adjusters(result);

  if (CondFlag[traitor])
    solving_insert_traitor_side_changers(result);

  if (TSTFLAG(some_pieces_flags,Volage))
    solving_insert_volage_side_changers(result);

  if (CondFlag[magicsquare])
    solving_insert_magic_square(result);

  if (CondFlag[wormholes])
    wormhole_initialse_solving(result);

  if (CondFlag[dbltibet])
    solving_insert_double_tibet(result);
  else if (CondFlag[tibet])
    solving_insert_tibet(result);

  if (CondFlag[degradierung])
    solving_insert_degradierung(result);

  en_passant_initialise_solving(result);

  if (CondFlag[phantom] || CondFlag[mars] || CondFlag[plus] || CondFlag[antimars])
  {
    move_generator_instrument_for_alternative_paths(result,nr_sides);

    stip_instrument_moves(result,STMarsCirceMoveToRebirthSquare);
    move_effect_journal_register_pre_capture_effect();
  }

  if (CondFlag[phantom])
    solving_initialise_phantom(result);
  else if (CondFlag[plus])
    solving_initialise_plus(result);
  else
  {
    if (CondFlag[antimars])
      solving_initialise_antimars(result);
    if (CondFlag[mars])
      solving_initialise_marscirce(result);
  }

  if (CondFlag[linechamchess])
    solving_insert_line_chameleon_chess(result);

  if (CondFlag[chamchess])
    chameleon_chess_initialise_solving(result);

  promotion_insert_slice_sequence(result,STMove,&move_insert_slices);
  promotion_instrument_solving_default(result);

  /* this has to come after all promotion_insert_slice_sequence() invokations
   * to support for promotion into Chameleon
   */
  if (TSTFLAG(some_pieces_flags,Chameleon))
    chameleon_initialise_solving(result);

  if (CondFlag[chamchess] || TSTFLAG(some_pieces_flags,Chameleon))
    chameleon_init_sequence_implicit(&chameleon_is_squence_explicit,
                                     &chameleon_walk_sequence);

  if (TSTFLAG(some_pieces_flags,ColourChange))
    solving_insert_hurdle_colour_changers(result);

  if (CondFlag[haanerchess])
    solving_insert_haan_chess(result);

  if (CondFlag[castlingchess])
    solving_insert_castling_chess(result);

  if (CondFlag[amu])
    solving_insert_amu_attack_counter(result);

  if (OptFlag[mutuallyexclusivecastling])
    solving_insert_mutual_castling_rights_adjusters(result);

  if (CondFlag[imitators])
    solving_insert_imitator(result);

  if (CondFlag[football])
    solving_insert_football_chess(result);

  if (CondFlag[platzwechselrochade])
    exchange_castling_initialise_solving(result);

  solving_insert_post_move_iteration(result);

  if (isMaxtimeSet())
    solving_insert_maxtime_guards(result);

  if (CondFlag[BGL])
    bgl_initialise_solving(result);

  if (TSTFLAG(some_pieces_flags,Patrol))
    patrol_initialise_solving(result);
  if (CondFlag[ultrapatrouille])
    ultrapatrol_initialise_solving(result);

  if (CondFlag[lortap])
    lortap_initialise_solving(result);

  if (CondFlag[provocateurs])
    provocateurs_initialise_solving(result);

  if (CondFlag[immun])
    immune_initialise_solving(result);

  if (CondFlag[woozles])
    woozles_initialise_solving(result);
  if (CondFlag[biwoozles])
    biwoozles_initialise_solving(result);
  if (CondFlag[heffalumps])
    heffalumps_initialise_solving(result);
  if (CondFlag[biheffalumps])
    biheffalumps_initialise_solving(result);

  if (CondFlag[nocapture] || CondFlag[nowhcapture] || CondFlag[noblcapture])
    solving_insert_nocapture(result);

  if (CondFlag[nowhiteprom] || CondFlag[noblackprom])
    solving_insert_nopromotions(result);

  if (CondFlag[geneva])
    geneva_initialise_solving(result);

  if (CondFlag[contactgrid])
    contact_grid_initialise_solving(result);
  else if (CondFlag[koeko])
    koeko_initialise_solving(result);
  else if (CondFlag[antikoeko])
    antikoeko_initialise_solving(result);
  else if (CondFlag[newkoeko])
    newkoeko_initialise_solving(result);

  if (TSTFLAG(some_pieces_flags,Jigger))
    jigger_initialise_solving(result);

  if (CondFlag[monochro])
      monochrome_initialise_solving(result);
  if (CondFlag[bichro])
      bichrome_initialise_solving(result);

  if (CondFlag[superguards])
    superguards_initialise_solving(result);

  if (CondFlag[whiteedge] || CondFlag[blackedge])
    solving_insert_edgemover(result);

  if (CondFlag[gridchess])
    grid_initialise_solving(result);

  if (TSTFLAG(some_pieces_flags,Uncapturable))
    solving_insert_uncapturable(result);

  if (CondFlag[takemake])
    solving_insert_take_and_make(result);

  if (OptFlag[noshort])
    solving_insert_no_short_variations_filters(result);

  if (OptFlag[maxsols])
    solving_insert_maxsolutions_filters(result);

  solving_optimise_dead_end_slices(result);

  if (OptFlag[stoponshort]
      && !solving_insert_stoponshortsolutions_filters(result))
    output_plaintext_message(NoStopOnShortSolutions);

  solving_remove_irrelevant_constraints(result);

  if (OptFlag[movenbr])
    solving_insert_restart_guards(result);

  solving_insert_continuation_solvers(result);

  solving_insert_find_shortest_solvers(result);

  solving_optimise_with_orthodox_mating_move_generators(result);

  solving_optimise_with_goal_non_reacher_removers(result);

  if (!OptFlag[solvariantes])
    solving_insert_play_suppressors(result);

  if (OptFlag[solvariantes] && !OptFlag[nothreat])
    solving_insert_threat_boundaries(result);

  solving_spin_off_testers(result);

  mummer_initialise_solving(result);

  if (CondFlag[ohneschach])
  {
    ohneschach_optimise_away_redundant_immobility_tests(result);
    ohneschach_optimise_away_immobility_tests_help(result);
    ohneschach_optimise_immobility_testers(result);
  }

  if (is_hashtable_allocated())
    solving_insert_hashing(result);

  solving_instrument_help_ends_of_branches(result);

  solving_insert_setplay_solvers(result);

  if (OptFlag[soltout]) /* this includes OptFlag[solessais] */
    solving_insert_try_solvers(result);

  solving_insert_trivial_variation_filters(result);

  solving_insert_min_length(result);

  if (OptFlag[nontrivial])
    solving_insert_max_nr_nontrivial_guards(result);

  if (OptFlag[solvariantes] && !OptFlag[nothreat])
    solving_insert_threat_handlers(result);

  if (OptFlag[degeneratetree])
    solving_insert_degenerate_tree_guards(result);

  solving_insert_output_slices(result);

  if (OptFlag[solmenaces]
      && !solving_insert_maxthreatlength_guards(result))
    output_plaintext_message(ThreatOptionAndExactStipulationIncompatible);

  if (CondFlag[republican])
    solving_insert_republican_king_placers(result);

  and_enable_shortcut_logic(result);

  solving_insert_avoid_unsolvable_forks(result);

  solving_insert_move_iterators(result);

  stip_instrument_moves(result,STKingSquareUpdater);

  if (TSTFLAG(some_pieces_flags,Magic))
    solving_insert_magic_pieces_recolorers(result);

  if (CondFlag[vogt])
    vogtlaender_initalise_solving(result);

  if (CondFlag[bicolores])
    bicolores_initalise_solving(result);

  if (CondFlag[antikings])
    antikings_initalise_solving(result);

  if (CondFlag[madras])
    madrasi_initialise_solving(result);

  solving_initialise_hunters(result);

  if (CondFlag[eiffel])
    eiffel_initialise_solving(result);

  if (CondFlag[disparate])
    disparate_initialise_solving(result);

  if (CondFlag[central])
    central_initialise_solving(result);

  if (TSTFLAG(some_pieces_flags,Beamtet))
    beamten_initialise_solving(result);

  if (CondFlag[whvault_king])
    vaulting_kings_initalise_solving(result,White);
  if (CondFlag[blvault_king])
    vaulting_kings_initalise_solving(result,Black);

  if (CondFlag[whsupertrans_king])
    supertransmuting_kings_initialise_solving(result,White);
  if (CondFlag[blsupertrans_king])
    supertransmuting_kings_initialise_solving(result,Black);

  if (CondFlag[whtrans_king])
    transmuting_kings_initialise_solving(result,White);
  if (CondFlag[bltrans_king])
    transmuting_kings_initialise_solving(result,Black);

  if (CondFlag[whrefl_king])
    reflective_kings_initialise_solving(result,White);
  if (CondFlag[blrefl_king])
    reflective_kings_initialise_solving(result,Black);

  if (CondFlag[annan])
    annan_initialise_solving(result);

#if defined(DOTRACE)
  solving_insert_move_tracers(result);
#endif

#if defined(DOMEASURE)
  solving_insert_move_counters(result);
#endif

  if (CondFlag[shieldedkings])
    shielded_kings_initialise_solving(result);

  if (CondFlag[brunner])
    brunner_initialise_solving(result);

  if (CondFlag[backhome])
    backhome_initialise_solving(result);

  if (CondFlag[facetoface])
    facetoface_initialise_solving(result);
  if (CondFlag[backtoback])
    backtoback_initialise_solving(result);
  if (CondFlag[cheektocheek])
    cheektocheek_initialise_solving(result);

  goal_kiss_init_piece_id(result);

  solving_impose_starter(result,SLICE_STARTER(result));
  solving_optimise_with_countnropponentmoves(result);

  solving_optimise_with_killer_moves(result);

  if (is_piece_neutral(some_pieces_flags))
    solving_optimise_by_detecting_retracted_moves(result);

  optimise_is_square_observed(result);
  optimise_is_in_check(result);

  solving_impose_starter(result,SLICE_STARTER(result));

  resolve_proxies(&result);

  adjust_slack_length(result,previous_move_has_solved);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
