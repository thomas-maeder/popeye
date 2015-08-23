#include "solving/machinery/twin.h"
#include "pieces/pieces.h"
#include "pieces/attributes/magic.h"
#include "pieces/attributes/neutral/neutral.h"
#include "pieces/walks/walks.h"
#include "pieces/walks/classification.h"
#include "pieces/walks/pawns/promotee_sequence.h"
#include "pieces/walks/pawns/en_passant.h"
#include "pieces/walks/orphan.h"
#include "position/underworld.h"
#include "conditions/conditions.h"
#include "conditions/alphabetic.h"
#include "conditions/anticirce/anticirce.h"
#include "conditions/blackchecks.h"
#include "conditions/circe/circe.h"
#include "conditions/duellists.h"
#include "conditions/exclusive.h"
#include "conditions/follow_my_leader.h"
#include "conditions/football.h"
#include "conditions/forced_squares.h"
#include "conditions/geneva.h"
#include "conditions/imitator.h"
#include "conditions/immune.h"
#include "conditions/isardam.h"
#include "conditions/marscirce/marscirce.h"
#include "conditions/marscirce/anti.h"
#include "conditions/marscirce/phantom.h"
#include "conditions/mummer.h"
#include "conditions/must_capture.h"
#include "conditions/republican.h"
#include "conditions/sentinelles.h"
#include "conditions/singlebox/type1.h"
#include "conditions/synchronous.h"
#include "conditions/transmuting_kings/super.h"
#include "conditions/transmuting_kings/transmuting_kings.h"
#include "conditions/transmuting_kings/vaulting_kings.h"
#include "options/options.h"
#include "optimisations/count_nr_opponent_moves/move_generator.h"
#include "optimisations/killer_move/killer_move.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_moves_generation.h"
#include "output/plaintext/message.h"
#include "solving/castling.h"
#include "solving/has_solution_type.h"
#include "solving/moves_traversal.h"
#include "solving/proofgames.h"
#include "solving/king_capture_avoider.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/pipe.h"
#include "stipulation/proxy.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

/* Sequence number of the current twin (and its duplex if any).
 * Mainly used for boolean twin-related flags: flag==twin_id means true,
 * and everything else means false. This allows us to not reset the flag at the
 * beginning (or end) of a twin.
 * We start at 1 so that all these flags (initialised with 0) are initially
 * false.
 */
twin_id_type twin_id = 1;

twin_stage_type twin_stage;
twin_duplex_type_type twin_duplex_type;
unsigned int twin_number;
boolean twin_is_continued;

/* Initialise the piece walk caches.
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
void piece_walk_caches_initialiser_solve(slice_index si)
{
  piece_walk_type p;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  initalise_standard_walks();

  for (p = Empty; p<nr_piece_walks; ++p)
    piece_walk_may_exist[p] = false;

  for (p = Empty; p<=Bishop; p++)
    piece_walk_may_exist[standard_walks[p]] = true;

  if (CondFlag[sentinelles])
    piece_walk_may_exist[sentinelle_walk] = true;

  if (CondFlag[chinoises])
    for (p = Leo; p<=Vao; ++p)
      piece_walk_may_exist[p] = true;

  for (p = (CondFlag[losingchess] || CondFlag[dynasty] || CondFlag[extinction]
            ? King
            : Queen);
       p < nr_piece_walks;
       ++p)
    if ((CondFlag[promotiononly] && promonly[p])
        || (CondFlag[football] && is_football_substitute[p]))
      piece_walk_may_exist[p] = true;

  if (CondFlag[protean])
    piece_walk_may_exist[ReversePawn] = true;

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void countPieces(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    piece_walk_type p;
    for (p = Empty; p<nr_piece_walks; ++p)
      piece_walk_exists[p] = false;
  }

  {
    piece_walk_type p;
    for (p = King; p<nr_piece_walks; ++p)
    {
      being_solved.number_of_pieces[White][p] = 0;
      being_solved.number_of_pieces[Black][p] = 0;
    }
  }

  {
    square const *bnp;
    for (bnp = boardnum; *bnp; ++bnp)
      if (!is_square_empty(*bnp))
      {
        piece_walk_type const p = get_walk_of_piece_on_square(*bnp);
        piece_walk_exists[p] = true;
        piece_walk_may_exist[p] = true;
        if (TSTFLAG(being_solved.spec[*bnp],White))
          ++being_solved.number_of_pieces[White][p];
        if (TSTFLAG(being_solved.spec[*bnp],Black))
          ++being_solved.number_of_pieces[Black][p];
      }
  }

  if (piece_walk_exists[MarinePawn]>0)
  {
    piece_walk_may_exist[Sirene] = true;
    piece_walk_may_exist[Triton] = true;
    piece_walk_may_exist[Nereide] = true;
    piece_walk_may_exist[MarineKnight] = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean locate_unique_royal(Side side, square *location)
{
  unsigned int nr_royals = 0;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  if (royal_square[side]!=initsquare)
  {
    *location = royal_square[side];
    ++nr_royals;
  }

  {
    square const *bnp;
    for (bnp = boardnum; *bnp; ++bnp)
    {
      square const s = *bnp;
      if ((is_king(get_walk_of_piece_on_square(s))
           || TSTFLAG(being_solved.spec[s],Royal))
           && TSTFLAG(being_solved.spec[s],side))
      {
        *location = s;
        ++nr_royals;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",nr_royals<=1);
  TraceFunctionResultEnd();
  return nr_royals<=1;
}

static boolean locate_royals(void)
{
  boolean result = true;
  square new_king_square[nr_sides] = { initsquare, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (CondFlag[dynasty])
  {
    assert(royal_square[White]==initsquare);
    assert(royal_square[Black]==initsquare);

    {
      square const *bnp;
      for (bnp = boardnum; *bnp; ++bnp)
      {
        square const s = *bnp;
        Side const king_side = TSTFLAG(being_solved.spec[s],White) ? White : Black;
        CLRFLAG(being_solved.spec[s],Royal); /* piece may be royal from previous twin */
        if (get_walk_of_piece_on_square(s)==King
            && being_solved.number_of_pieces[king_side][King]==1)
          new_king_square[king_side] = s;
      }
    }
  }
  else if (CondFlag[losingchess] || CondFlag[extinction])
  {
    assert(royal_square[White]==initsquare);
    assert(royal_square[Black]==initsquare);

    {
      square const *bnp;
      for (bnp = boardnum; *bnp; ++bnp)
      {
        assert(!TSTFLAG(being_solved.spec[*bnp],Royal));
      }
    }
  }
  else if (!locate_unique_royal(White,&new_king_square[White])
           || !locate_unique_royal(Black,&new_king_square[Black]))
  {
    output_plaintext_verifie_message(OneKing);
    result = false;
  }

  if (result)
  {
    move_effect_journal_do_king_square_movement(move_effect_reason_diagram_setup,
                                                White,new_king_square[White]);
    move_effect_journal_do_king_square_movement(move_effect_reason_diagram_setup,
                                                Black,new_king_square[Black]);
    king_square_horizon = move_effect_journal_base[nbply+1];
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

void initialise_piece_ids(void)
{
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  currPieceId = MinPieceId;
  for (bnp = boardnum; *bnp; ++bnp)
    if (!is_square_empty(*bnp) && !is_square_blocked(*bnp))
    {
      assert(currPieceId<=MaxPieceId);
      SetPieceId(being_solved.spec[*bnp],currPieceId++);
    }

  currPieceId = underworld_set_piece_ids(currPieceId);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise piece flags from conditions, the pieces themselve etc.
 */
static void initialise_piece_flags(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  CLEARFL(some_pieces_flags);
  CLEARFL(all_pieces_flags);
  CLEARFL(all_royals_flags);

  {
    square const *bnp;
    for (bnp = boardnum; *bnp; ++bnp)
      if (!is_square_empty(*bnp))
      {
        if (is_piece_neutral(being_solved.spec[*bnp]))
          SETFLAGMASK(some_pieces_flags,NeutralMask);

        if (TSTFLAG(being_solved.spec[*bnp],Royal)
            && !is_king(get_walk_of_piece_on_square(*bnp)))
          SETFLAG(some_pieces_flags,Royal);

        some_pieces_flags |= being_solved.spec[*bnp] & ~NeutralMask & ~BIT(Royal);
      }
  }

  if (TSTFLAG(some_pieces_flags,HalfNeutral))
    SETFLAGMASK(some_pieces_flags,NeutralMask);

  if (stipulation_are_pieceids_required()
      || CondFlag[backhome]
      || circe_variant.determine_rebirth_square==circe_determine_rebirth_square_diagram)
    SETFLAGMASK(some_pieces_flags,PieceIdMask);
  else
    CLRFLAGMASK(some_pieces_flags,PieceIdMask);

  if (CondFlag[volage])
  {
    SETFLAG(some_pieces_flags,Volage);
    SETFLAG(all_pieces_flags,Volage);
  }

  if (CondFlag[patrouille])
  {
    SETFLAG(some_pieces_flags,Patrol);
    SETFLAG(all_pieces_flags,Patrol);
    SETFLAG(all_royals_flags,Patrol);
  }

  if (CondFlag[beamten])
  {
    SETFLAG(some_pieces_flags,Beamtet);
    SETFLAG(all_pieces_flags,Beamtet);
    SETFLAG(all_royals_flags,Beamtet);
  }

  if (CondFlag[ghostchess] || CondFlag[hauntedchess])
    SETFLAG(some_pieces_flags,Uncapturable);

  {
    square const *bnp;
    for (bnp = boardnum; *bnp; ++bnp)
      if (!is_square_empty(*bnp) && !is_square_blocked(*bnp))
      {
        piece_walk_type const p = get_walk_of_piece_on_square(*bnp);
        Flags spec = being_solved.spec[*bnp];
        SETFLAGMASK(spec,all_pieces_flags);
        move_effect_journal_do_flags_change(move_effect_reason_diagram_setup,*bnp,spec);

        SavePositionInDiagram(being_solved.spec[*bnp],*bnp);

        if (TSTFLAG(spec,ColourChange)
            && !(is_simplehopper(p)
                 || is_chineserider(p)
                 || is_lion(p)
                 || p==ContraGras))
        {
          /* relies on imitators already having been implemented */
          CLRFLAG(spec,ColourChange);
          move_effect_journal_do_flags_change(move_effect_reason_diagram_setup,*bnp,spec);
          output_plaintext_error_message(ColourChangeRestricted);
        }
      }
  }

  if (CondFlag[dynasty])
  {
    {
      Flags spec = being_solved.spec[being_solved.king_square[White]];
      CLRFLAGMASK(spec,all_pieces_flags);
      SETFLAGMASK(spec,all_royals_flags);
      if (being_solved.number_of_pieces[White][King]==1)
        SETFLAG(spec,Royal);
      move_effect_journal_do_flags_change(move_effect_reason_diagram_setup,
                                          being_solved.king_square[White],
                                          spec);
    }
    {
      Flags spec = being_solved.spec[being_solved.king_square[Black]];
      CLRFLAGMASK(spec,all_pieces_flags);
      SETFLAGMASK(spec,all_royals_flags);
      if (being_solved.number_of_pieces[Black][King]==1)
        SETFLAG(spec,Royal);
      move_effect_journal_do_flags_change(move_effect_reason_diagram_setup,
                                          being_solved.king_square[Black],
                                          spec);
    }
  }
  else if (CondFlag[losingchess] || CondFlag[extinction])
  {
    /* nothing */
  }
  else
  {
    if (being_solved.king_square[White]!=initsquare)
    {
      Flags spec = being_solved.spec[being_solved.king_square[White]];
      CLRFLAGMASK(spec,all_pieces_flags);
      if (is_king(get_walk_of_piece_on_square(being_solved.king_square[White])))
        SETFLAGMASK(spec,all_royals_flags|BIT(Royal));
      else
        SETFLAGMASK(spec,all_royals_flags);
      move_effect_journal_do_flags_change(move_effect_reason_diagram_setup,
                                          being_solved.king_square[White],
                                          spec);
    }
    if (being_solved.king_square[Black]!=initsquare)
    {
      Flags spec = being_solved.spec[being_solved.king_square[Black]];
      CLRFLAGMASK(spec,all_pieces_flags);
      if (is_king(get_walk_of_piece_on_square(being_solved.king_square[Black])))
        SETFLAGMASK(spec,all_royals_flags|BIT(Royal));
      else
        SETFLAGMASK(spec,all_royals_flags);
      move_effect_journal_do_flags_change(move_effect_reason_diagram_setup,
                                          being_solved.king_square[Black],
                                          spec);
    }
  }

  king_square_horizon = move_effect_journal_base[nbply+1];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine contribution of slice subtree to maximum number of moves
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void get_max_nr_moves_binary(slice_index si, stip_moves_traversal *st)
{
  stip_length_type * const result = st->param;
  stip_length_type const save_result = *result;
  stip_length_type result1;
  stip_length_type result2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_binary_operand1(si,st);
  result1 = *result;
  TraceValue("%u\n",result1);

  *result = save_result;
  stip_traverse_moves_binary_operand2(si,st);
  result2 = *result;
  TraceValue("%u\n",result2);

  if (result1>result2)
    *result = result1;
  TraceValue("%u\n",*result);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine contribution of slice subtree to maximum number of moves
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void get_max_nr_moves_move(slice_index si, stip_moves_traversal *st)
{
  stip_length_type * const result = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  ++*result;
  TraceValue("%u\n",*result);

  stip_traverse_moves_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine the maximally possible number of half-moves until the
 * goal has to be reached.
 * @param si root of subtree
 * @param maximally possible number of half-moves
 */
static stip_length_type get_max_nr_moves(slice_index si)
{
  stip_moves_traversal st;
  stip_length_type result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  stip_moves_traversal_init(&st,&result);
  stip_moves_traversal_override_by_contextual(&st,
                                              slice_contextual_binary,
                                              &get_max_nr_moves_binary);
  stip_moves_traversal_override_single(&st,STMove,&get_max_nr_moves_move);
  stip_traverse_moves(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

typedef boolean is_restricted_type[nr_sides];

static void find_restricted_side_attack(slice_index si,
                                        stip_structure_traversal *st)
{
  is_restricted_type * const is_restricted = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  (*is_restricted)[advers(SLICE_STARTER(si))] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void find_restricted_side_defense(slice_index si,
                                         stip_structure_traversal *st)
{
  is_restricted_type * const is_restricted = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  (*is_restricted)[SLICE_STARTER(si)] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void find_restricted_side_help(slice_index si,
                                      stip_structure_traversal *st)
{
  is_restricted_type * const is_restricted = st->param;
  stip_length_type const length = SLICE_U(si).branch.length;
  Side const starter = SLICE_STARTER(si);
  Side const restricted_side = length%2==1 ? advers(starter) : starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,restricted_side,"\n");
  (*is_restricted)[restricted_side] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor restricted_side_finders[] =
{
  { STAttackAdapter,   &find_restricted_side_attack  },
  { STDefenseAdapter , &find_restricted_side_defense },
  { STReadyForDefense, &find_restricted_side_defense },
  { STHelpAdapter,     &find_restricted_side_help    }
};

enum
{
  nr_restricted_side_finders = (sizeof restricted_side_finders
                                / sizeof restricted_side_finders[0])
};

/* Find the side restricted by some condition (e.g. maximummer without
 * indication of the side)
 * @param si identifies the slice where to start looking for the
 *           restricted side
 * @return the restricted side; no_side if the restricted side can't
 *         be determined
 */
static Side findRestrictedSide(slice_index si)
{
  stip_structure_traversal st;
  is_restricted_type is_restricted = { false, false };
  Side result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&is_restricted);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_conditional_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override(&st,
                                    restricted_side_finders,
                                    nr_restricted_side_finders);
  stip_traverse_structure(si,&st);

  if (is_restricted[White] && !is_restricted[Black])
    result = White;
  else if (is_restricted[Black] && !is_restricted[White])
    result = Black;
  else
    result = no_side;

  TraceFunctionExit(__func__);
  TraceEnumerator(Side,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Verify the user input and our interpretation of it
 * @param si identifies the root slice of the representation of the
 *           stipulation
 */
void verify_position(slice_index si)
{
  boolean flagveryfairy = false;
  boolean flagsymmetricfairy = false;

  reset_killer_move_optimisation();
  reset_orthodox_mating_move_optimisation();

  reset_countnropponentmoves_defense_move_optimisation();

  if (CondFlag[glasgow]
      && CondFlag[circe]
      && circe_variant.relevant_side_overrider==circe_relevant_side_overrider_mirror)
    circe_variant.is_promotion_possible = true;

  /* initialize promotion squares */
  if (!(CondFlag[einstein] || CondFlag[reveinstein]))
  {
    square i;
    if (!CondFlag[whprom_sq])
      for (i = 0; i<nr_files_on_board; ++i)
        SETFLAG(sq_spec[CondFlag[glasgow] ? square_h7-i : square_h8-i],
                WhPromSq);

    if (!CondFlag[blprom_sq])
      for (i = 0; i<nr_files_on_board; ++i)
        SETFLAG(sq_spec[CondFlag[glasgow] ? square_a2+i : square_a1+i],
                BlPromSq);
  }

  {
    unsigned int i;
    for (i = 0; i<nr_files_on_board; i++)
    {
      SETFLAG(sq_spec[square_a1+i*dir_right],WhBaseSq);
      SETFLAG(sq_spec[square_a2+i*dir_right],WhPawnDoublestepSq);
      SETFLAG(sq_spec[square_a7+i*dir_right],BlPawnDoublestepSq);
      SETFLAG(sq_spec[square_a8+i*dir_right],BlBaseSq);

      SETFLAG(sq_spec[square_a3+i*dir_right],CapturableByWhPawnSq);
      SETFLAG(sq_spec[square_a4+i*dir_right],CapturableByWhPawnSq);
      SETFLAG(sq_spec[square_a5+i*dir_right],CapturableByWhPawnSq);
      SETFLAG(sq_spec[square_a6+i*dir_right],CapturableByWhPawnSq);
      SETFLAG(sq_spec[square_a7+i*dir_right],CapturableByWhPawnSq);
      SETFLAG(sq_spec[square_a8+i*dir_right],CapturableByWhPawnSq);

      SETFLAG(sq_spec[square_a1+i*dir_right],CapturableByBlPawnSq);
      SETFLAG(sq_spec[square_a2+i*dir_right],CapturableByBlPawnSq);
      SETFLAG(sq_spec[square_a3+i*dir_right],CapturableByBlPawnSq);
      SETFLAG(sq_spec[square_a4+i*dir_right],CapturableByBlPawnSq);
      SETFLAG(sq_spec[square_a5+i*dir_right],CapturableByBlPawnSq);
      SETFLAG(sq_spec[square_a6+i*dir_right],CapturableByBlPawnSq);

      if (circe_variant.determine_rebirth_square==circe_determine_rebirth_square_equipollents
          || CondFlag[normalp]
          || CondFlag[einstein]
          || circe_variant.determine_rebirth_square==circe_determine_rebirth_square_cage
          || CondFlag[wormholes])
      {
        SETFLAG(sq_spec[square_a2+i*dir_right],CapturableByWhPawnSq);
        SETFLAG(sq_spec[square_a7+i*dir_right],CapturableByBlPawnSq);
      }
    }
  }

  im0 = being_solved.isquare[0];
  if (! CondFlag[imitators])
    CondFlag[noiprom] = true;

  if (get_max_nr_moves(si) >= maxply-2)
  {
    output_plaintext_verifie_message(BigNumMoves);
    return;
  }

  if (circe_variant.relevant_capture==circe_relevant_capture_lastmove
      && stip_ends_in(si,goal_steingewinn))
  {
    output_plaintext_verifie_message(PercentAndParrain);
    return;
  }

  if (CondFlag[republican] && !republican_verifie_position(si))
    return;

  if ((royal_square[Black]!=initsquare || royal_square[White]!=initsquare
       || CondFlag[white_oscillatingKs] || CondFlag[black_oscillatingKs]
       || circe_variant.is_rex_inclusive
       || immune_variant.is_rex_inclusive
       || TSTFLAG(some_pieces_flags,Royal))
      && (CondFlag[dynasty] || CondFlag[losingchess] || CondFlag[extinction]))
  {
    output_plaintext_verifie_message(IncompatibleRoyalSettings);
    return;
  }

  if (CondFlag[takemake])
  {
    if (CondFlag[sentinelles]
        || CondFlag[nocapture]
        || CondFlag[anticirce])
    {
      output_plaintext_verifie_message(TakeMakeAndFairy);
      return;
    }
  }

#ifdef _SE_DECORATE_SOLUTION_
  se_init();
#endif

  piece_walk_may_exist_fairy = false;

  {
    piece_walk_type p;
    for (p = Bishop+1; p<nr_piece_walks; ++p)
    {
      if (piece_walk_may_exist[p]
          || (CondFlag[promotiononly] && promonly[p])
          || (CondFlag[football] && is_football_substitute[p]))
      {
        piece_walk_may_exist_fairy = true;
        if (is_rider(p)) {}
        else if (is_leaper(p)) {}
        else if (is_simplehopper(p)) {}
        else if (is_simpledecomposedleaper(p)) {}
        else if (is_symmetricfairy(p))
          flagsymmetricfairy = true;
        else
          flagveryfairy = true;

        if (TSTFLAG(some_pieces_flags,Magic) && !magic_is_piece_supported(p))
        {
          output_plaintext_verifie_message(MagicAndFairyPieces);
          return;
        }
        if (CondFlag[einstein])
        {
          output_plaintext_verifie_message(EinsteinAndFairyPieces);
          return;
        }
      }
    }
  }

  if (CondFlag[sting])
  {
    disable_orthodox_mating_move_optimisation(nr_sides);
    piece_walk_may_exist_fairy = true;
  }

  if (CondFlag[immun] && immune_variant.is_rex_inclusive)
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (CondFlag[imitators])
  {
    if (flagveryfairy
        || flagsymmetricfairy
        || TSTFLAG(some_pieces_flags, Jigger)
        || CondFlag[annan]
        || CondFlag[newkoeko]
        || CondFlag[gridchess] || CondFlag[koeko] || CondFlag[antikoeko]
        || CondFlag[blackedge] || CondFlag[whiteedge]
        || CondFlag[geneva]
        || CondFlag[chinoises]
        || CondFlag[immun]
        || CondFlag[haanerchess]
        || CondFlag[sting])
    {
      output_plaintext_verifie_message(ImitWFairy);
      return;
    }
    disable_orthodox_mating_move_optimisation(nr_sides);
  }

  if (CondFlag[leofamily])
  {
    piece_walk_type p;
    for (p = Queen; p<=Bishop; p++)
      if (being_solved.number_of_pieces[White][p]+being_solved.number_of_pieces[Black][p]!=0)
      {
        output_plaintext_verifie_message(LeoFamAndOrtho);
        return;
      }
  }

  if (CondFlag[chinoises])
    piece_walk_may_exist_fairy = true;

  if (CondFlag[circe])
  {
    if (piece_walk_may_exist[Dummy])
    {
      output_plaintext_verifie_message(CirceAndDummy);
      return;
    }
    if (is_piece_neutral(some_pieces_flags)
        || CondFlag[volage] || TSTFLAG(some_pieces_flags,Volage)
        || circe_variant.on_occupied_rebirth_square==circe_on_occupied_rebirth_square_volcanic
        || circe_variant.on_occupied_rebirth_square==circe_on_occupied_rebirth_square_parachute)
      disable_orthodox_mating_move_optimisation(nr_sides);
  }

  mummer_reset_length_measurers();

  if (ExtraCondFlag[maxi])
  {
    Side const restricted_side = findRestrictedSide(si);
    if (restricted_side==no_side)
    {
      output_plaintext_verifie_message(CantDecideOnSideWhichConditionAppliesTo);
      return;
    }
    else
    {
      mummer_strictness[restricted_side] = mummer_strictness_default_side;
      CondFlag[blmax] = restricted_side==Black;
      CondFlag[whmax] = restricted_side==White;
    }
  }

  if (CondFlag[alphabetic])
  {
    CondFlag[blackalphabetic] = true;
    CondFlag[whitealphabetic] = true;
  }

  if (CondFlag[blmax] && !mummer_set_length_measurer(Black,&maximummer_measure_length))
  {
    output_plaintext_verifie_message(TwoMummerCond);
    return;
  }
  if (CondFlag[whmax] && !mummer_set_length_measurer(White,&maximummer_measure_length))
  {
    output_plaintext_verifie_message(TwoMummerCond);
    return;
  }

  if (CondFlag[blmin] && !mummer_set_length_measurer(Black,&minimummer_measure_length))
  {
    output_plaintext_verifie_message(TwoMummerCond);
    return;
  }
  if (CondFlag[whmin] && !mummer_set_length_measurer(White,&minimummer_measure_length))
  {
    output_plaintext_verifie_message(TwoMummerCond);
    return;
  }

  if (CondFlag[blcapt] && !mummer_set_length_measurer(Black,&must_capture_measure_length))
  {
    output_plaintext_verifie_message(TwoMummerCond);
    return;
  }
  if (CondFlag[whcapt] && !mummer_set_length_measurer(White,&must_capture_measure_length))
  {
    output_plaintext_verifie_message(TwoMummerCond);
    return;
  }

  if (CondFlag[blfollow] && !mummer_set_length_measurer(Black,&follow_my_leader_measure_length))
  {
    output_plaintext_verifie_message(TwoMummerCond);
    return;
  }
  if (CondFlag[whfollow] && !mummer_set_length_measurer(White,&follow_my_leader_measure_length))
  {
    output_plaintext_verifie_message(TwoMummerCond);
    return;
  }

  if (CondFlag[duellist]
      && !(mummer_set_length_measurer(Black,&duellists_measure_length)
           && mummer_set_length_measurer(White,&duellists_measure_length)))
  {
    output_plaintext_verifie_message(TwoMummerCond);
    return;
  }

  if (CondFlag[blackalphabetic]
      && !mummer_set_length_measurer(Black,&alphabetic_measure_length))
  {
    output_plaintext_verifie_message(TwoMummerCond);
    return;
  }
  if (CondFlag[whitealphabetic]
      && !mummer_set_length_measurer(White,&alphabetic_measure_length))
  {
    output_plaintext_verifie_message(TwoMummerCond);
    return;
  }

  if (CondFlag[blacksynchron]
      && !mummer_set_length_measurer(Black,&synchronous_measure_length))
  {
    output_plaintext_verifie_message(TwoMummerCond);
    return;
  }
  if (CondFlag[whitesynchron]
      && !mummer_set_length_measurer(White,&synchronous_measure_length))
  {
    output_plaintext_verifie_message(TwoMummerCond);
    return;
  }

  if (CondFlag[blackantisynchron]
      && !mummer_set_length_measurer(Black,&antisynchronous_measure_length))
  {
    output_plaintext_verifie_message(TwoMummerCond);
    return;
  }
  if (CondFlag[whiteantisynchron]
      && !mummer_set_length_measurer(White,&antisynchronous_measure_length))
  {
    output_plaintext_verifie_message(TwoMummerCond);
    return;
  }

  /* the mummer logic is (ab)used to priorise transmuting king moves */
  if (CondFlag[blsupertrans_king]
      && !mummer_set_length_measurer(Black,&len_supertransmuting_kings))
  {
    output_plaintext_verifie_message(TwoMummerCond);
    return;
  }
  if (CondFlag[whsupertrans_king]
      && !mummer_set_length_measurer(White,&len_supertransmuting_kings))
  {
    output_plaintext_verifie_message(TwoMummerCond);
    return;
  }

  if (CondFlag[blforsqu]
      && !mummer_set_length_measurer(Black,&forced_squares_measure_length))
  {
    output_plaintext_verifie_message(CantDecideOnSideWhichConditionAppliesTo);
    return;
  }
  if (CondFlag[whforsqu]
      && !mummer_set_length_measurer(White,&forced_squares_measure_length))
  {
    output_plaintext_verifie_message(TwoMummerCond);
    return;
  }

  if (CondFlag[blconforsqu]
      && !mummer_set_length_measurer(Black,&forced_squares_measure_length))
  {
    output_plaintext_verifie_message(TwoMummerCond);
    return;
  }
  if (CondFlag[whconforsqu]
      && !mummer_set_length_measurer(White,&forced_squares_measure_length))
  {
    output_plaintext_verifie_message(TwoMummerCond);
    return;
  }

  if (CondFlag[schwarzschacher]
      && !mummer_set_length_measurer(Black,&blackchecks_measure_length))
  {
    output_plaintext_verifie_message(TwoMummerCond);
    return;
  }

  if (CondFlag[snekchess] && CondFlag[snekcirclechess])
  {
    output_plaintext_verifie_message(NonsenseCombination);
    return;
  }


  if (ExtraCondFlag[ultraschachzwang])
  {
    Side const restricted_side = findRestrictedSide(si);
    if (restricted_side==no_side)
    {
      output_plaintext_verifie_message(CantDecideOnSideWhichConditionAppliesTo);
      return;
    }
    else
    {
      CondFlag[blackultraschachzwang] = restricted_side==Black;
      CondFlag[whiteultraschachzwang] = restricted_side==White;

      disable_orthodox_mating_move_optimisation(nr_sides);
    }
  }

  if (CondFlag[cavaliermajeur])
  {
    if (being_solved.number_of_pieces[White][Knight] + being_solved.number_of_pieces[Black][Knight] > 0)
    {
      output_plaintext_verifie_message(CavMajAndKnight);
      return;
    }
    piece_walk_may_exist_fairy = true;
  }

  if (OptFlag[sansrb] && being_solved.king_square[White]!=initsquare)
    OptFlag[sansrb] = false;

  if (OptFlag[sansrn] && being_solved.king_square[Black]!=initsquare)
    OptFlag[sansrn] = false;

  if (being_solved.king_square[White]==initsquare && being_solved.number_of_pieces[White][King]==0 && !OptFlag[sansrb])
  {
    output_plaintext_error_message(MissingKing);
    output_plaintext_error_message(NewLine);
  }
  else if (being_solved.king_square[Black]==initsquare && being_solved.number_of_pieces[Black][King]==0 && !OptFlag[sansrn])
  {
    output_plaintext_error_message(MissingKing);
    output_plaintext_error_message(NewLine);
  }

  if (circe_variant.is_rex_inclusive)
  {
    if (circe_variant.determine_rebirth_square==circe_determine_rebirth_square_equipollents
        || circe_variant.reborn_walk_adapter==circe_reborn_walk_adapter_clone
        || circe_variant.actual_relevant_piece==circe_relevant_piece_capturer)
    {
      /* disallowed because determining the rebirth square while detecting check
       * would require knowledge of the departure square. Other forms now allowed
       */
      if ((!OptFlag[sansrb]
           && being_solved.king_square[White]!=initsquare
           && get_walk_of_piece_on_square(being_solved.king_square[White])!=standard_walks[King])
          || (!OptFlag[sansrn]
              && being_solved.king_square[Black]!=initsquare
              && get_walk_of_piece_on_square(being_solved.king_square[Black])!=standard_walks[King]))
      {
        output_plaintext_verifie_message(RoyalPWCRexCirce);
        return;
      }
    }
  }

  if (CondFlag[bicolores]
      || CondFlag[magicsquare]
      || CondFlag[volage]
      || CondFlag[masand]
      || CondFlag[dynasty]
      || TSTFLAG(some_pieces_flags,Magic))
  {
    if (is_piece_neutral(some_pieces_flags))
    {
      output_plaintext_verifie_message(TooFairyForNeutral);
      return;
    }
  }

  if (circe_variant.is_rex_inclusive
      || CondFlag[bicolores])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (CondFlag[backhome])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (CondFlag[monochro] && CondFlag[bichro])
  {
    output_plaintext_verifie_message(MonoAndBiChrom);
    return;
  }

  if (CondFlag[bichro] || CondFlag[monochro])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if ((CondFlag[koeko]
       || CondFlag[newkoeko]
       || CondFlag[antikoeko]
       || TSTFLAG(some_pieces_flags, Jigger))
      && CondFlag[circe]
      && is_piece_neutral(some_pieces_flags))
  {
    output_plaintext_verifie_message(TooFairyForNeutral);
    return;
  }

  if (CondFlag[protean])
  {
    piece_walk_may_exist_fairy = true;
    disable_orthodox_mating_move_optimisation(nr_sides);
  }

  if ((circe_variant.determine_rebirth_square==circe_determine_rebirth_square_super
       || circe_variant.determine_rebirth_square==circe_determine_rebirth_square_cage)
      && (CondFlag[koeko] || CondFlag[newkoeko] || CondFlag[antikoeko]))
  {
    output_plaintext_verifie_message(SuperCirceAndOthers);
    return;
  }

  {
    int numsuper=0;
    if (circe_variant.determine_rebirth_square==circe_determine_rebirth_square_super) numsuper++;
    if (circe_variant.determine_rebirth_square==circe_determine_rebirth_square_cage) numsuper++;
    if (anticirce_variant.determine_rebirth_square==circe_determine_rebirth_square_super) numsuper++;
    if (numsuper>1)
    {
      output_plaintext_verifie_message(SuperCirceAndOthers);
      return;
    }
  }

  if (CondFlag[nowhcapture] && CondFlag[noblcapture])
  {
    CondFlag[nocapture] = true;
    if (CondFlag[nocapture])
    {
      CondFlag[nowhcapture] = false;
      CondFlag[noblcapture] = false;
    }
  }

  if (CondFlag[isardam]+CondFlag[madras]+CondFlag[eiffel]>1)
  {
    output_plaintext_verifie_message(IsardamAndMadrasi);
    return;
  }

  if (CondFlag[black_oscillatingKs] || CondFlag[white_oscillatingKs])
  {
    if (being_solved.king_square[White]==initsquare || being_solved.king_square[Black]==initsquare)
    {
      CondFlag[black_oscillatingKs] = false;
      CondFlag[white_oscillatingKs] = false;
    }
    else
      disable_orthodox_mating_move_optimisation(nr_sides);
  }

  if (CondFlag[mars] || CondFlag[antimars] || CondFlag[plus] || CondFlag[phantom])
  {
    if ((CondFlag[mars]||CondFlag[antimars])+CondFlag[plus]+CondFlag[phantom]>1)
    {
      output_plaintext_verifie_message(MarsCirceAndOthers);
      return;
    }
    else if ((CondFlag[whvault_king] && vaulting_kings_transmuting[White])
             || (CondFlag[blvault_king] && vaulting_kings_transmuting[Black])
             || calc_reflective_king[White]
             || calc_reflective_king[Black]
             || CondFlag[bicolores]
             || CondFlag[sting])
    {
      output_plaintext_verifie_message(MarsCirceAndOthers);
      return;
    }
    else
      disable_orthodox_mating_move_optimisation(nr_sides);
  }

  if ((CondFlag[mars] && !marscirce_is_variant_consistent(&marscirce_variant))
      || (CondFlag[antimars] && !marscirce_is_variant_consistent(&antimars_variant))
      || (CondFlag[phantom] && !marscirce_is_variant_consistent(&phantom_variant))
      || (CondFlag[geneva] && !geneva_is_variant_consistent(&geneva_variant)))
  {
    output_plaintext_verifie_message(NonsenseCombination);
    return;
  }

  if (CondFlag[madras] || CondFlag[eiffel] || CondFlag[isardam])
  {
    if ( CondFlag[imitators]|| TSTFLAG(some_pieces_flags,Paralysing))
    {
      output_plaintext_verifie_message(MadrasiParaAndOthers);
      return;
    }
  }

  if (circe_variant.on_occupied_rebirth_square==circe_on_occupied_rebirth_square_assassinate)
  {
    if (is_piece_neutral(some_pieces_flags) /* Neutrals not implemented */
        || CondFlag[bicolores])             /* others? */
    {
      output_plaintext_verifie_message(AssassinandOthers);
      return;
    }
  }

  if (circe_variant.is_rex_inclusive && immune_variant.is_rex_inclusive)
  {
    output_plaintext_verifie_message(RexCirceImmun);
    return;
  }

  if (immune_variant.is_rex_inclusive && CondFlag[anticirce])
  {
    output_plaintext_verifie_message(SomeCondAndAntiCirce);
    return;
  }

  if (CondFlag[anticirce])
  {
    if (circe_variant.actual_relevant_piece==circe_relevant_piece_capturer
        || CondFlag[koeko]
        || CondFlag[newkoeko]
        || CondFlag[antikoeko]
        || (CondFlag[singlebox] && SingleBoxType==ConditionType1)
        || CondFlag[geneva]
        || TSTFLAG(some_pieces_flags, Kamikaze))
    {
      output_plaintext_verifie_message(SomeCondAndAntiCirce);
      return;
    }
  }

  if ((CondFlag[white_oscillatingKs] || CondFlag[black_oscillatingKs])
      && (OptFlag[sansrb] || OptFlag[sansrn]))
  {
    output_plaintext_verifie_message(MissingKing);
    return;
  }

  if (mummer_strictness[White]==mummer_strictness_ultra && !CondFlag[whcapt])
  {
    if (is_piece_neutral(some_pieces_flags))
    {
      output_plaintext_verifie_message(TooFairyForNeutral);
      return;
    }
  }

  if (mummer_strictness[Black]==mummer_strictness_ultra && !CondFlag[blcapt])
  {
    if (is_piece_neutral(some_pieces_flags))
    {
      output_plaintext_verifie_message(TooFairyForNeutral);
      return;
    }
  }

  if ((CondFlag[heffalumps] || CondFlag[biheffalumps])
      && (piece_walk_may_exist[Rose]
          || piece_walk_may_exist[SpiralSpringer]
          || piece_walk_may_exist[UbiUbi]
          || piece_walk_may_exist[Hamster]
          || piece_walk_may_exist[Elk]
          || piece_walk_may_exist[Eagle]
          || piece_walk_may_exist[Sparrow]
          || piece_walk_may_exist[Archbishop]
          || piece_walk_may_exist[ReflectBishop]
          || piece_walk_may_exist[Cardinal]
          || piece_walk_may_exist[BoyScout]
          || piece_walk_may_exist[GirlScout]
          || piece_walk_may_exist[DiagonalSpiralSpringer]
          || piece_walk_may_exist[BouncyKnight]
          || piece_walk_may_exist[BouncyNightrider]
          || piece_walk_may_exist[CAT]
          || piece_walk_may_exist[RoseHopper]
          || piece_walk_may_exist[RoseLion]
          || piece_walk_may_exist[Rao]
          || piece_walk_may_exist[RookMoose]
          || piece_walk_may_exist[RookEagle]
          || piece_walk_may_exist[RookSparrow]
          || piece_walk_may_exist[BishopMoose]
          || piece_walk_may_exist[BishopEagle]
          || piece_walk_may_exist[BishopSparrow]
          || piece_walk_may_exist[DoubleGras]
          || piece_walk_may_exist[DoubleRookHopper]
          || piece_walk_may_exist[DoubleBishopper]))
  {
    output_plaintext_verifie_message(SomePiecesAndHeffa);
    return;
  }

  if (CondFlag[ghostchess] || CondFlag[hauntedchess])
  {
    if (CondFlag[circe] || CondFlag[anticirce]
        || CondFlag[haanerchess]
        || TSTFLAG(some_pieces_flags,Kamikaze)
        || (CondFlag[ghostchess] && CondFlag[hauntedchess]))
    {
      output_plaintext_verifie_message(GhostHauntedChessAndCirceKamikazeHaanIncompatible);
      return;
    }
  }

  if (en_passant_nr_retro_squares>=en_passant_retro_min_squares
      && !en_passant_are_retro_squares_consistent())
  {
    output_plaintext_verifie_message(InconsistentRetroInformation);
    return;
  }

  change_moving_piece=
      TSTFLAG(some_pieces_flags, Kamikaze)
      || TSTFLAG(some_pieces_flags, Protean)
      || CondFlag[tibet]
      || CondFlag[andernach]
      || CondFlag[antiandernach]
      || CondFlag[magicsquare]
      || TSTFLAG(some_pieces_flags,Chameleon)
      || CondFlag[einstein]
      || CondFlag[reveinstein]
      || CondFlag[antieinstein]
      || CondFlag[volage] || TSTFLAG(some_pieces_flags, Volage)
      || CondFlag[degradierung]
      || CondFlag[norsk]
      || CondFlag[traitor]
      || CondFlag[linechamchess]
      || CondFlag[chamchess]
      || CondFlag[protean]
      || CondFlag[champursue];

  if (CondFlag[vogt]
      || CondFlag[antikings]
      || CondFlag[SAT]
      || CondFlag[strictSAT]
      || CondFlag[shieldedkings])
    king_capture_avoiders_avoid_opponent();

  if (TSTFLAG(some_pieces_flags, Jigger)
      || CondFlag[newkoeko]
      || CondFlag[koeko]
      || CondFlag[antikoeko]
      || circe_variant.relevant_capture==circe_relevant_capture_lastmove
      || CondFlag[anticirce]
      || mummer_strictness[White]!=mummer_strictness_none
      || mummer_strictness[Black]!=mummer_strictness_none
      || CondFlag[vogt]
      || CondFlag[central]
      || TSTFLAG(some_pieces_flags,Beamtet)
      || TSTFLAG(some_pieces_flags,Patrol)
      || CondFlag[provocateurs]
      || CondFlag[ultrapatrouille]
      || CondFlag[lortap]
      || CondFlag[shieldedkings]
      || TSTFLAG(some_pieces_flags,Paralysing)
      || CondFlag[madras] || CondFlag[eiffel]
      || CondFlag[brunner]
      || (being_solved.king_square[White]!=initsquare && get_walk_of_piece_on_square(being_solved.king_square[White])!=King)
      || (being_solved.king_square[Black]!=initsquare && get_walk_of_piece_on_square(being_solved.king_square[Black])!=King)
      || TSTFLAG(some_pieces_flags, Chameleon)
      || CondFlag[einstein]
      || CondFlag[reveinstein]
      || CondFlag[antieinstein]
      || CondFlag[degradierung]
      || CondFlag[norsk]
      || CondFlag[messigny]
      || CondFlag[linechamchess]
      || CondFlag[chamchess]
      || CondFlag[antikings]
      || TSTFLAG(some_pieces_flags, HalfNeutral)
      || CondFlag[geneva]
      || CondFlag[disparate]
      || CondFlag[BGL]
      || CondFlag[dynasty]
      || TSTFLAG(some_pieces_flags,Magic)
      || CondFlag[woozles] || CondFlag[biwoozles]
      || CondFlag[heffalumps] || CondFlag[biheffalumps]
      || (CondFlag[singlebox] && SingleBoxType==ConditionType3)
      || CondFlag[football]
      || CondFlag[wormholes])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (CondFlag[superguards])
    disable_orthodox_mating_move_optimisation(nr_sides);

  pieces_pawns_init_promotees();

  {
    piece_walk_type p;
    piece_walk_type firstprompiece;
    unsigned int check_piece_index = 0;
    boolean is_check_piece[nr_piece_walks] = { false };

    if (CondFlag[losingchess] || CondFlag[dynasty] || CondFlag[extinction])
      firstprompiece = King;
    else if ((CondFlag[singlebox] && SingleBoxType!=ConditionType1) || CondFlag[football])
      firstprompiece = Pawn;
    else
      firstprompiece = Queen;

    for (p = firstprompiece; p<nr_piece_walks; ++p)
      if (piece_walk_may_exist[p])
      {
        if (p>Bishop && p!=Dummy)
        {
          /* only fairy pieces until now ! */
          disable_orthodox_mating_move_optimisation(nr_sides);
          if (p!=Hamster)
          {
            checkpieces[check_piece_index] = p;
            is_check_piece[p] = true;
            check_piece_index++;
          }
        }
      }

    if (CondFlag[whvault_king])
    {
      unsigned int i;
      for (i = 0; i!=nr_king_vaulters[White]; ++i)
        if (king_vaulters[White][i]>Bishop
            && !piece_walk_may_exist[king_vaulters[White][i]]
            && !is_check_piece[king_vaulters[White][i]])
        {
          piece_walk_may_exist_fairy = true;
          checkpieces[check_piece_index] = king_vaulters[White][i];
          is_check_piece[king_vaulters[White][i]] = true;
          check_piece_index++;
          break;
        }
    }
    if (CondFlag[blvault_king])
    {
      unsigned int i;
      for (i = 0; i!=nr_king_vaulters[Black]; ++i)
        if (king_vaulters[Black][i]>Bishop
            && !piece_walk_may_exist[king_vaulters[Black][i]]
            && !is_check_piece[king_vaulters[Black][i]])
        {
          piece_walk_may_exist_fairy = true;
          checkpieces[check_piece_index] = king_vaulters[Black][i];
          is_check_piece[king_vaulters[Black][i]] = true;
          check_piece_index++;
          break;
        }
    }

    checkpieces[check_piece_index] = 0;
  }

  {
    unsigned int op = 0;
    piece_walk_type p;
    for (p = King; p<nr_piece_walks; ++p) {
      if (piece_walk_may_exist[p] && p!=Dummy && p!=Hamster)
      {
        if (p!=Orphan && p!=Friend
            && (piece_walk_may_exist[Orphan] || piece_walk_may_exist[Friend]))
          orphanpieces[op++] = p;
      }
    }
    orphanpieces[op] = Empty;
  }

  if (TSTFLAG(some_pieces_flags, Kamikaze))
  {
    disable_orthodox_mating_move_optimisation(nr_sides);
    if (CondFlag[haanerchess])
    {
      output_plaintext_verifie_message(KamikazeAndHaaner);
      return;
    }
    if (CondFlag[circe]) {
      /* No Kamikaze and Circe with fairy pieces; taking and
         taken piece could be reborn on the same square! */
      if (piece_walk_may_exist_fairy || CondFlag[volage])
      {
        output_plaintext_verifie_message(KamikazeAndSomeCond);
        return;
      }
    }
    king_capture_avoiders_avoid_own();
  }

  if ((CondFlag[singlebox]  && SingleBoxType==ConditionType1))
  {
    if (piece_walk_may_exist_fairy)
    {
      output_plaintext_verifie_message(SingleBoxAndFairyPieces);
      return;
    }
  }

  if (CondFlag[whtrans_king] || CondFlag[whsupertrans_king] || CondFlag[whrefl_king]
      || CondFlag[bltrans_king] || CondFlag[blsupertrans_king] || CondFlag[blrefl_king])
    transmuting_kings_init_transmuters_sequence();

  if (calc_reflective_king[White] || calc_reflective_king[Black])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if ((calc_reflective_king[White]
       && being_solved.king_square[White]!=initsquare
       && (get_walk_of_piece_on_square(being_solved.king_square[White])!=King
           || CondFlag[sting]))
      || (calc_reflective_king[Black]
          && being_solved.king_square[Black]!=initsquare
          && (get_walk_of_piece_on_square(being_solved.king_square[Black])!=King
              || CondFlag[sting])))
  {
    output_plaintext_verifie_message(TransmRoyalPieces);
    return;
  }

  if ((piece_walk_may_exist[Orphan] || piece_walk_may_exist[Friend])
      && is_piece_neutral(some_pieces_flags))
  {
    output_plaintext_verifie_message(TooFairyForNeutral);
    return;
  }

  if (((CondFlag[isardam] && isardam_variant==ConditionTypeA) || CondFlag[brunner])
      && CondFlag[vogt])
  {
    output_plaintext_verifie_message(VogtlanderandIsardam);
    return;
  }

  if ((CondFlag[chamchess] || CondFlag[linechamchess])
      && TSTFLAG(some_pieces_flags, Chameleon))
  {
    output_plaintext_verifie_message(ChameleonPiecesAndChess);
    return;
  }

  if (CondFlag[platzwechselrochade] && CondFlag[haanerchess])
  {
    output_plaintext_verifie_message(NonsenseCombination);
    return;
  }

  if (TSTFLAG(some_pieces_flags, ColourChange))
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (CondFlag[sentinelles])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (CondFlag[annan]
      || CondFlag[facetoface] || CondFlag[backtoback] || CondFlag[cheektocheek])
  {
    if (CondFlag[masand] || TSTFLAG(some_pieces_flags, Magic))
    {
      output_plaintext_verifie_message(AnnanChessAndConditionRecoloringPieces);
      return;
    }
    else
      disable_orthodox_mating_move_optimisation(nr_sides);
  }

  if (CondFlag[losingchess])
  {
    if (stip_ends_in(si,goal_mate)
        || stip_ends_in(si,goal_check)
        || stip_ends_in(si,goal_mate_or_stale))
    {
      output_plaintext_verifie_message(LosingChessNotInCheckOrMateStipulations);
      return;
    }

    /* capturing moves are "longer" than non-capturing moves */
    if (!(mummer_set_length_measurer(Black,&must_capture_measure_length)
          && mummer_set_length_measurer(White,&must_capture_measure_length)))
    {
      output_plaintext_verifie_message(TwoMummerCond);
      return;
    }
  }

  /* check castling possibilities */
  CLEARFL(being_solved.castling_rights);

  if ((get_walk_of_piece_on_square(square_e1)== standard_walks[King]) && TSTFLAG(being_solved.spec[square_e1], White)
      && (!CondFlag[dynasty] || being_solved.number_of_pieces[White][standard_walks[King]]==1))
    SETCASTLINGFLAGMASK(White,k_cancastle);
  if ((get_walk_of_piece_on_square(square_h1)== standard_walks[Rook]) && TSTFLAG(being_solved.spec[square_h1], White))
    SETCASTLINGFLAGMASK(White,rh_cancastle);
  if ((get_walk_of_piece_on_square(square_a1)== standard_walks[Rook]) && TSTFLAG(being_solved.spec[square_a1], White))
    SETCASTLINGFLAGMASK(White,ra_cancastle);
  if ((get_walk_of_piece_on_square(square_e8)== standard_walks[King]) && TSTFLAG(being_solved.spec[square_e8], Black)
      && (!CondFlag[dynasty] || being_solved.number_of_pieces[Black][standard_walks[King]]==1))
    SETCASTLINGFLAGMASK(Black,k_cancastle);
  if ((get_walk_of_piece_on_square(square_h8)== standard_walks[Rook]) && TSTFLAG(being_solved.spec[square_h8], Black))
    SETCASTLINGFLAGMASK(Black,rh_cancastle);
  if ((get_walk_of_piece_on_square(square_a8)== standard_walks[Rook]) && TSTFLAG(being_solved.spec[square_a8], Black))
    SETCASTLINGFLAGMASK(Black,ra_cancastle);

  being_solved.castling_rights &= castling_flags_no_castling;

  /* a small hack to enable ep keys */
  trait[1] = no_side;

  if (CondFlag[exclusive] && !exclusive_verifie_position(si))
    return;

  if (CondFlag[isardam]
      || CondFlag[ohneschach])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (CondFlag[extinction]
      || circe_variant.on_occupied_rebirth_square==circe_on_occupied_rebirth_square_assassinate)
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (CondFlag[actrevolving] || CondFlag[arc])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (circe_variant.is_turncoat)
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (CondFlag[kobulkings])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (CondFlag[SAT] || CondFlag[strictSAT])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (CondFlag[schwarzschacher])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (mummer_strictness[White]!=mummer_strictness_none /* counting opponents moves not useful */
      || is_piece_neutral(some_pieces_flags)
      || CondFlag[exclusive]
      || CondFlag[isardam]
      || CondFlag[ohneschach]
      || CondFlag[mars] || CondFlag[plus] || CondFlag[phantom]
      || CondFlag[antimars]
      || CondFlag[brunner]
      || CondFlag[blsupertrans_king]
      || CondFlag[whsupertrans_king]
      || CondFlag[takemake]
      || circe_variant.determine_rebirth_square==circe_determine_rebirth_square_cage
      || CondFlag[SAT]
      || CondFlag[strictSAT]
      || CondFlag[schwarzschacher]
      || CondFlag[republican]
      || CondFlag[kobulkings]
      || piece_walk_may_exist[UbiUbi] /* sorting by nr of opponents moves doesn't work - why?? */
      || piece_walk_may_exist[Hunter0] /* ditto */
      || (CondFlag[singlebox] && SingleBoxType==ConditionType3)) /* ditto */
    disable_countnropponentmoves_defense_move_optimisation(White);

  if (mummer_strictness[Black]!=mummer_strictness_none /* counting opponents moves not useful */
      || is_piece_neutral(some_pieces_flags)
      || CondFlag[exclusive]
      || CondFlag[isardam]
      || CondFlag[ohneschach]
      || CondFlag[mars] || CondFlag[plus] || CondFlag[phantom]
      || CondFlag[antimars]
      || CondFlag[brunner]
      || CondFlag[blsupertrans_king]
      || CondFlag[whsupertrans_king]
      || CondFlag[takemake]
      || circe_variant.determine_rebirth_square==circe_determine_rebirth_square_cage
      || CondFlag[SAT]
      || CondFlag[strictSAT]
      || CondFlag[schwarzschacher]
      || CondFlag[republican]
      || CondFlag[kobulkings]
      || piece_walk_may_exist[UbiUbi] /* sorting by nr of opponents moves doesn't work  - why?? */
      || piece_walk_may_exist[Hunter0] /* ditto */
      || (CondFlag[singlebox] && SingleBoxType==ConditionType3)) /* ditto */
    disable_countnropponentmoves_defense_move_optimisation(Black);

  if (CondFlag[takemake])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (CondFlag[castlingchess] || CondFlag[platzwechselrochade])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (mummer_strictness[Black]!=mummer_strictness_none
      || CondFlag[messigny]
      || (CondFlag[singlebox] && SingleBoxType==ConditionType3)
      || CondFlag[whsupertrans_king]
      || CondFlag[blsupertrans_king]
      || CondFlag[takemake]
      || CondFlag[exclusive]
      || CondFlag[isardam]
      || CondFlag[ohneschach]
      || TSTFLAG(some_pieces_flags,ColourChange) /* killer machinery doesn't store hurdle */)
    disable_killer_move_optimisation(Black);
  if (mummer_strictness[White]!=mummer_strictness_none
      || CondFlag[messigny]
      || (CondFlag[singlebox] && SingleBoxType==ConditionType3)
      || CondFlag[whsupertrans_king]
      || CondFlag[blsupertrans_king]
      || CondFlag[takemake]
      || CondFlag[exclusive]
      || CondFlag[isardam]
      || CondFlag[ohneschach]
      || TSTFLAG(some_pieces_flags,ColourChange) /* killer machinery doesn't store hurdle */)
    disable_killer_move_optimisation(White);

  pipe_solve_delegate(si);
}

void royals_locator_solve(slice_index si)
{
  move_effect_journal_index_type const save_king_square_horizon = king_square_horizon;

  if (locate_royals())
    pipe_solve_delegate(si);

  king_square_horizon = save_king_square_horizon;
}

void initialise_piece_flags_solve(slice_index si)
{
  initialise_piece_flags();
  pipe_solve_delegate(si);
}

void pieces_counter_solve(slice_index si)
{
  countPieces();
  pipe_solve_delegate(si);
}

void create_builder_setup_ply_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",solving_machinery);
  TraceFunctionParamListEnd();

  nextply(no_side);
  assert(nbply==ply_setup_solving);

  pipe_solve_delegate(si);

  undo_move_effects();
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Validate whether shifting the entire position would move >=1 piece off board
 * @return true iff it doesn't
 */
boolean twin_twinning_shift_validate(square from, square to)
{
  int const vector = to-from;

  square const *bnp;
  for (bnp = boardnum; *bnp; bnp++)
    if (!is_square_empty(*bnp) && is_square_blocked(*bnp+vector))
      return false;

  return true;
}
