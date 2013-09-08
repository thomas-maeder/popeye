#include "solving/solvers.h"
#include "pieces/pieces.h"
#include "options/options.h"
#include "conditions/conditions.h"
#include "pymsg.h"
#include "solving/selfcheck_guard.h"
#include "solving/check.h"
#include "stipulation/proxy.h"
#include "solving/observation.h"
#include "output/output.h"
#include "conditions/annan.h"
#include "conditions/bgl.h"
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
#include "conditions/circe/assassin.h"
#include "conditions/ultraschachzwang/legality_tester.h"
#include "conditions/sat.h"
#include "conditions/oscillating_kings.h"
#include "conditions/messigny.h"
#include "conditions/actuated_revolving_centre.h"
#include "conditions/actuated_revolving_board.h"
#include "conditions/circe/circe.h"
#include "conditions/circe/antipodes.h"
#include "conditions/circe/cage.h"
#include "conditions/circe/chameleon.h"
#include "conditions/circe/clone.h"
#include "conditions/circe/couscous.h"
#include "conditions/circe/diagram.h"
#include "conditions/circe/diametral.h"
#include "conditions/circe/double_agents.h"
#include "conditions/circe/equipollents.h"
#include "conditions/circe/file.h"
#include "conditions/circe/kamikaze.h"
#include "conditions/circe/parrain.h"
#include "conditions/circe/volage.h"
#include "conditions/circe/promotion.h"
#include "conditions/circe/frischauf.h"
#include "conditions/circe/mirror.h"
#include "conditions/circe/pwc.h"
#include "conditions/circe/super.h"
#include "conditions/circe/symmetry.h"
#include "conditions/circe/april.h"
#include "conditions/circe/turncoats.h"
#include "conditions/circe/takemake.h"
#include "conditions/circe/rex_inclusive.h"
#include "conditions/circe/rank.h"
#include "conditions/anticirce/cheylan.h"
#include "conditions/anticirce/couscous.h"
#include "conditions/anticirce/anticirce.h"
#include "conditions/anticirce/promotion.h"
#include "conditions/anticirce/strict.h"
#include "conditions/anticirce/relaxed.h"
#include "conditions/anticirce/super.h"
#include "conditions/anticirce/magic_square.h"
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
#include "conditions/vaulting_kings.h"
#include "conditions/backhome.h"
#include "conditions/vogtlaender.h"
#include "conditions/bicolores.h"
#include "conditions/antikings.h"
#include "conditions/sting.h"
#include "conditions/follow_my_leader.h"
#include "conditions/alphabetic.h"
#include "conditions/forced_squares.h"
#include "conditions/synchronous.h"
#include "conditions/must_capture.h"
#include "platform/maxtime.h"
#include "conditions/shielded_kings.h"
#include "stipulation/end_of_branch_tester.h"
#include "stipulation/dead_end.h"
#include "stipulation/boolean/and.h"
#include "stipulation/constraint.h"
#include "stipulation/temporary_hacks.h"
#include "stipulation/goals/immobile/reached_tester.h"
#include "solving/king_capture_avoider.h"
#include "solving/testers.h"
#include "solving/find_shortest.h"
#include "solving/for_each_move.h"
#include "solving/battle_play/try.h"
#include "solving/battle_play/threat.h"
#include "solving/battle_play/continuation.h"
#include "solving/battle_play/setplay.h"
#include "solving/battle_play/min_length_guard.h"
#include "solving/trivial_end_filter.h"
#include "solving/avoid_unsolvable.h"
#include "solving/play_suppressor.h"
#include "solving/castling.h"
#include "pieces/walks/pawns/en_passant.h"
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
#include "conditions/amu/mate_filter.h"
#include "conditions/circe/goal_filters.h"
#include "conditions/anticirce/goal_filters.h"
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
#include "debugging/trace.h"
#include "debugging/measure.h"

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

  stip_impose_starter(result,slices[stipulation_root_hook].starter);

  insert_temporary_hacks(result);

  /* must come before stip_insert_selfcheck_guards() and
   * stip_insert_move_generators() because flight counting machinery needs
   * selfcheck guards and move generators */
  if (OptFlag[solflights])
    stip_insert_maxflight_guards(result);

  /* must come before stip_insert_selfcheck_guards() because the
   * instrumentation of the goal filters inserts or slices of
   * which both branches need selfcheck guards */
  if (anycirce)
    stip_insert_circe_goal_filters(result);
  if (anyanticirce)
    stip_insert_anticirce_goal_filters(result);
  if (TSTFLAG(some_pieces_flags,Kamikaze))
    stip_insert_kamikaze(result);

  /* must come before stip_apply_setplay() */
  stip_insert_root_slices(result);
  stip_insert_intro_slices(result);

  /* must come before stip_insert_selfcheck_guards() because the set play
   * branch needs a selfcheck guard */
  if (OptFlag[solapparent] && !OptFlag[restart]
      && !stip_apply_setplay(result))
    Message(SetPlayNotApplicable);

  /* must come before stip_insert_move_generators() because immobilise_black
   * needs a move generator */
  if (!init_intelligent_mode(result))
    Message(IntelligentRestricted);

  /* must come here because we generate branches that have to be provided with
   * self-check guards and move generators
   */
  if (CondFlag[ohneschach])
    ohneschach_insert_check_guards(result);

  if (CondFlag[losingchess])
    solving_instrument_check_testing(result,STNoCheckConceptCheckTester);

  /* must come here because in conditions like MAFF, we are going to tamper with
   * the slices inserted here
   */
  stip_insert_selfcheck_guards(result);
  stip_insert_move_generators(result);

  if (OptFlag[keepmating])
    stip_insert_keepmating_filters(result);

  if (CondFlag[amu])
    stip_insert_amu_mate_filters(result);

  if (CondFlag[whiteultraschachzwang]
      || CondFlag[blackultraschachzwang])
    ultraschachzwang_initialise_solving(result);

  if (CondFlag[ohneschach])
  {
    /* prevent king first optimisation - the Ohneschach specific optimisation
     * is more effective */
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

  if (CondFlag[extinction])
    extinction_initialise_solving(result);

  if (CondFlag[singlebox])
    switch (SingleBoxType)
    {
      case singlebox_type1:
        singlebox_type1_initialise_solving(result);
        break;

      case singlebox_type2:
        stip_insert_singlebox_type2(result);
        break;

      case singlebox_type3:
        stip_insert_singlebox_type3(result);
        break;

      default:
        break;
    }

  if (CondFlag[exclusive])
    stip_insert_exclusive_chess(result);

  stip_insert_king_capture_avoiders(result);

  if (CondFlag[isardam])
    stip_insert_isardam_legality_testers(result);

  if (CondFlag[patience])
    stip_insert_patience_chess(result);

  if (TSTFLAG(some_pieces_flags,Paralysing))
    paralysing_initialise_solving(result);

  if (CondFlag[strictSAT])
    strictsat_initialise_solving(result);
  else if (CondFlag[SAT])
    sat_initialise_solving(result);

  if (CondFlag[schwarzschacher])
    blackchecks_initialise_solving(result);

  if (CondFlag[masand])
    stip_insert_masand(result);

  if (CondFlag[dynasty])
    dynasty_initialise_solving(result);

  if (TSTFLAG(some_pieces_flags,ColourChange))
    stip_insert_hurdle_colour_changers(result);

  stip_insert_king_oscillators(result);

  if (CondFlag[messigny])
    stip_insert_messigny(result);

  if (CondFlag[arc])
    stip_insert_actuated_revolving_centre(result);

  if (CondFlag[actrevolving])
    stip_insert_actuated_revolving_board(result);

  if (anyparrain)
    circe_parrain_initialise_solving(result);
  else if (CondFlag[supercirce])
    supercirce_initialise_solving(result);
  else if (CondFlag[april])
    april_chess_initialise_solving(result);
  else if (CondFlag[circecage])
    circe_cage_initialise_solving(result);
  else if (CondFlag[circeassassin])
    assassin_circe_initalise_solving(result);
  else if (CondFlag[circetakeandmake])
    circe_take_make_initialse_solving(result);
  else if (CondFlag[circeantipoden])
    antipodes_circe_initialise_solving(result);
  else if (CondFlag[pwc])
    pwc_initialise_solving(result);
  else if (CondFlag[circeequipollents])
    circe_equipollents_initialise_solving(result);
  else if (CondFlag[circefile])
    file_circe_initialise_solving(result);
  else if (CondFlag[circefilemalefique])
  {
    file_circe_initialise_solving(result);
    circe_instrument_solving(result,STSpiegelCirceDetermineRelevantSide);
  }
  else if (CondFlag[circediagramm])
    diagram_circe_initialise_solving(result);
  else if (CondFlag[circesymmetry])
    symmetry_circe_initialise_solving(result);
  else if (anycirce)
  {
    circe_initialise_solving(result);
    if (CondFlag[circedoubleagents])
      circe_double_agents_initialise_solving(result);
    if (CondFlag[chamcirce])
      chameleon_circe_initialise_solving(result);
    if (anyclone)
      circe_clone_initialise_solving(result);
    if (CondFlag[circediametral])
      diametral_circe_initialise_solving(result);
    if (CondFlag[couscousmirror]
        || CondFlag[circemalefique]
        || CondFlag[circeclonemalefique]
        || CondFlag[circedoubleagents])
      circe_instrument_solving(result,STSpiegelCirceDetermineRelevantSide);
    if (CondFlag[circerank])
      rank_circe_initialise_solving(result);
    if (TSTFLAG(some_pieces_flags,Kamikaze))
    {
      anticirce_relaxed_initialise_solving(result);
      circe_kamikaze_initialise_solving(result);
      if (CondFlag[couscous])
        anticirce_couscous_initialise_solving(result);
    }
    if (CondFlag[couscous])
      couscous_circe_initialise_solving(result);
  }

  if (anycirce)
  {
    if  (TSTFLAG(some_pieces_flags,Volage))
      circe_volage_initialise_solving(result);
    if  (anycirprom)
      stip_insert_circe_promoters(result);
    if (CondFlag[circeturncoats])
      circe_turncoats_initialise_solving(result);
  }

  if (CondFlag[sentinelles])
    stip_insert_sentinelles_inserters(result);

  if (CondFlag[antisuper])
  {
    stip_insert_anticirce_strict(result);
    antisupercirce_initialise_solving(result);
    anticirce_promotion_initialise_solving(result);
  }
  else if (anyanticirce)
  {
    stip_insert_anticirce_strict(result);
    if (CondFlag[magicsquare] && magic_square_type==magic_square_type2)
      stip_insert_magic_square_type2(result);
    anticirce_initialise_solving(result);
    if (anyanticirprom)
      anticirce_promotion_initialise_solving(result);
    if (CondFlag[couscous])
      anticirce_couscous_initialise_solving(result);
    if (CondFlag[antidiagramm])
      anticirce_solving_substitute(result,
                                   STAnticirceDetermineRebirthSquare,
                                   STDiagramCirceDetermineRebirthSquare);
    if (CondFlag[antifile])
      anticirce_solving_substitute(result,
                                   STAnticirceDetermineRebirthSquare,
                                   STFileCirceDetermineRebirthSquare);
    if (CondFlag[antisymmetrie])
      anticirce_solving_substitute(result,
                                   STAnticirceDetermineRebirthSquare,
                                   STSymmetryCirceDetermineRebirthSquare);
    if (CondFlag[antispiegelfile])
      anticirce_solving_substitute(result,
                                   STAnticirceDetermineRebirthSquare,
                                   STFileCirceDetermineRebirthSquare);
    if (CondFlag[antiantipoden])
      anticirce_solving_substitute(result,
                                   STAnticirceDetermineRebirthSquare,
                                   STAntipodesCirceDetermineRebirthSquare);
    if (CondFlag[antiequipollents])
      anticirce_solving_substitute(result,
                                   STAnticirceDetermineRebirthSquare,
                                   STCirceEquipollentsDetermineRebirthSquare);
    if (CondFlag[antispiegel]
        || CondFlag[antispiegelfile])
      anticirce_instrument_solving(result,STSpiegelCirceDetermineRelevantSide);
  }
  if (AntiCirceType==AntiCirceTypeCheylan)
    anticirce_cheylan_initialise_solving(result);

  if (CondFlag[duellist])
    stip_insert_duellists(result);

  if (CondFlag[hauntedchess])
    stip_insert_haunted_chess(result);

  if (CondFlag[ghostchess])
    stip_insert_ghost_chess(result);

  if (kobulking[White] || kobulking[Black])
    stip_insert_kobul_king_substitutors(result);

  if (TSTFLAG(some_pieces_flags,HalfNeutral))
    stip_insert_half_neutral_recolorers(result);

  if (CondFlag[andernach])
    stip_insert_andernach(result);

  if (CondFlag[antiandernach])
    stip_insert_antiandernach(result);

  if (CondFlag[champursue])
    stip_insert_chameleon_pursuit(result);

  if (CondFlag[norsk])
    stip_insert_norsk_chess(result);

  if (CondFlag[protean] || TSTFLAG(some_pieces_flags,Protean))
    stip_insert_protean_chess(result);

  solving_initialise_castling(result);

  if (CondFlag[einstein])
    stip_insert_einstein_moving_adjusters(result);

  if (CondFlag[reveinstein])
    stip_insert_reverse_einstein_moving_adjusters(result);

  if (CondFlag[antieinstein])
    stip_insert_anti_einstein_moving_adjusters(result);

  if (CondFlag[einstein] || CondFlag[antieinstein] || CondFlag[reveinstein])
    stip_insert_einstein_en_passant_adjusters(result);

  if (CondFlag[traitor])
    stip_insert_traitor_side_changers(result);

  if (TSTFLAG(some_pieces_flags,Volage))
    stip_insert_volage_side_changers(result);

  if (CondFlag[magicsquare])
    stip_insert_magic_square(result);

  if (CondFlag[wormholes])
    wormhole_initialse_solving(result);

  if (CondFlag[dbltibet])
    stip_insert_double_tibet(result);
  else if (CondFlag[tibet])
    stip_insert_tibet(result);

  if (CondFlag[degradierung])
    stip_insert_degradierung(result);

  if (CondFlag[chamchess])
    chameleon_chess_initialse_solving(result);

  if (TSTFLAG(some_pieces_flags,Chameleon))
    chameleon_initialse_solving(result);

  if (CondFlag[frischauf])
    stip_insert_frischauf_promotee_markers(result);

  if (CondFlag[phantom])
    solving_initialise_phantom(result);
  else if (anyantimars)
    solving_initialise_antimars(result);
  else
  {
    stip_insert_en_passant_adjusters(result);

    if (CondFlag[plus])
      solving_initialise_plus(result);
    else if (anymars)
      solving_initialise_marscirce(result);
  }

  if (CondFlag[linechamchess])
    stip_insert_line_chameleon_chess(result);

  stip_insert_moving_pawn_promoters(result);

  if (CondFlag[haanerchess])
    stip_insert_haan_chess(result);

  if (CondFlag[castlingchess])
    stip_insert_castling_chess(result);

  if (CondFlag[amu])
    stip_insert_amu_attack_counter(result);

  if (OptFlag[mutuallyexclusivecastling])
    stip_insert_mutual_castling_rights_adjusters(result);

  if (CondFlag[imitators])
    stip_insert_imitator(result);

  if (CondFlag[football])
    stip_insert_football_chess(result);

  if (CondFlag[platzwechselrochade])
    exchange_castling_initialise_solving(result);

  stip_insert_post_move_iteration(result);

  if (dealWithMaxtime())
    stip_insert_maxtime_guards(result);

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

  if (anyimmun)
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
    stip_insert_nocapture(result);

  if (CondFlag[nowhiteprom] || CondFlag[noblackprom])
    stip_insert_nopromotions(result);

  if (anygeneva)
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
    stip_insert_edgemover(result);

  if (CondFlag[gridchess])
    grid_initialise_solving(result);

  if (TSTFLAG(some_pieces_flags,Uncapturable))
    stip_insert_uncapturable(result);

  if (CondFlag[takemake])
    stip_insert_take_and_make(result);

  if (OptFlag[noshort])
    stip_insert_no_short_variations_filters(result);

  if (OptFlag[maxsols])
    stip_insert_maxsolutions_filters(result);

  stip_optimise_dead_end_slices(result);

  if (OptFlag[stoponshort]
      && !stip_insert_stoponshortsolutions_filters(result))
    Message(NoStopOnShortSolutions);

  stip_remove_irrelevant_constraints(result);

  if (OptFlag[movenbr])
    stip_insert_restart_guards(result);

  stip_insert_continuation_solvers(result);

  stip_insert_find_shortest_solvers(result);

  stip_optimise_with_orthodox_mating_move_generators(result);

  stip_optimise_with_goal_non_reacher_removers(result);

  if (!OptFlag[solvariantes])
    stip_insert_play_suppressors(result);

  if (OptFlag[solvariantes] && !OptFlag[nothreat])
    stip_insert_threat_boundaries(result);

  stip_spin_off_testers(result);

  mummer_initialise_solving(result);

  if (CondFlag[ohneschach])
  {
    ohneschach_optimise_away_redundant_immobility_tests(result);
    ohneschach_optimise_away_immobility_tests_help(result);
    ohneschach_optimise_immobility_testers(result);
  }

  if (is_hashtable_allocated())
    stip_insert_hash_slices(result);

  stip_instrument_help_ends_of_branches(result);

  stip_insert_setplay_solvers(result);

  if (OptFlag[soltout]) /* this includes OptFlag[solessais] */
    stip_insert_try_solvers(result);

  stip_insert_trivial_variation_filters(result);

  stip_insert_min_length(result);

  if (OptFlag[nontrivial])
    stip_insert_max_nr_nontrivial_guards(result);

  if (OptFlag[solvariantes] && !OptFlag[nothreat])
    stip_insert_threat_handlers(result);

  if (OptFlag[degeneratetree])
    stip_insert_degenerate_tree_guards(result);

  stip_impose_starter(result,slices[result].starter);
  stip_optimise_with_countnropponentmoves(result);

  stip_optimise_with_killer_moves(result);

  if (is_piece_neutral(some_pieces_flags))
    solving_optimise_by_detecting_retracted_moves(result);

  stip_insert_output_slices(result);

  if (OptFlag[solmenaces]
      && !stip_insert_maxthreatlength_guards(result))
    Message(ThreatOptionAndExactStipulationIncompatible);

  if (CondFlag[republican])
    stip_insert_republican_king_placers(result);

  and_enable_shortcut_logic(result);

  stip_insert_avoid_unsolvable_forks(result);

  stip_insert_move_iterators(result);

  if (TSTFLAG(some_pieces_flags,Magic))
    stip_insert_magic_pieces_recolorers(result);

  if (CondFlag[vogt])
    vogtlaender_initalise_solving(result);

  if (CondFlag[bicolores])
    bicolores_initalise_solving(result);

  if (CondFlag[antikings])
    antikings_initalise_solving(result);

  if (CondFlag[madras])
    madrasi_initialise_solving(result);

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

  if (CondFlag[sting])
    sting_initalise_solving(result);

  if (CondFlag[annan])
    annan_initialise_solving(result);

  if (flagfee)
    stip_instrument_is_square_observed_testing(result,
                                               nr_sides,
                                               STFindSquareObserverTrackingBackFairy);

#if defined(DOTRACE)
  stip_insert_move_tracers(result);
#endif

#if defined(DOMEASURE)
  stip_insert_move_counters(result);
#endif

  if (CondFlag[shieldedkings])
    shielded_kings_initialise_solving(result);

  if (CondFlag[brunner])
    brunner_initialise_solving(result);

  if (circe_is_rex_inclusive)
    circe_rex_inclusive_initialise_solving(result);

  if (CondFlag[backhome])
    backhome_initialise_solving(result);

  optimise_is_square_observed(result);
  optimise_is_in_check(result);

  stip_impose_starter(result,slices[result].starter);

  resolve_proxies(&result);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
