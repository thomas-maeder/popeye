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
#include "conditions/circe/parrain.h"
#include "conditions/duellists.h"
#include "conditions/exclusive.h"
#include "conditions/follow_my_leader.h"
#include "conditions/football.h"
#include "conditions/forced_squares.h"
#include "conditions/imitator.h"
#include "conditions/immune.h"
#include "conditions/isardam.h"
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
#include "output/plaintext/proofgame.h"
#include "solving/castling.h"
#include "solving/has_solution_type.h"
#include "solving/move_effect_journal.h"
#include "solving/moves_traversal.h"
#include "solving/proofgames.h"
#include "solving/king_capture_avoider.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/machinery/solvers.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

twin_number_type twin_number;

static void initialise_piece_walk_caches(void)
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
    if (promonly[p] || is_football_substitute[p])
      piece_walk_may_exist[p] = true;

  if (CondFlag[protean])
    piece_walk_may_exist[ReversePawn] = true;

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
      number_of_pieces[White][p] = 0;
      number_of_pieces[Black][p] = 0;
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
        if (TSTFLAG(spec[*bnp],White))
          ++number_of_pieces[White][p];
        if (TSTFLAG(spec[*bnp],Black))
          ++number_of_pieces[Black][p];
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
      if (is_king(get_walk_of_piece_on_square(s)))
      {
        if (TSTFLAG(spec[s],side))
        {
          *location = s;
          ++nr_royals;
        }
        CLRFLAGMASK(spec[s],all_pieces_flags);
        SETFLAGMASK(spec[s],all_royals_flags|BIT(Royal));
      }
      else if (TSTFLAG(spec[s],Royal))
      {
        if (TSTFLAG(spec[s],side))
        {
          *location = s;
          ++nr_royals;
        }
        CLRFLAGMASK(spec[s],all_pieces_flags);
        SETFLAGMASK(spec[s],all_royals_flags);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",nr_royals<=1);
  TraceFunctionResultEnd();
  return nr_royals<=1;
}

static boolean locate_royals(square (*new_king_square)[nr_sides])
{
  boolean result = true;

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
        assert(!TSTFLAG(spec[s],Royal));
        if (get_walk_of_piece_on_square(s)==King)
        {
          Side const king_side = TSTFLAG(spec[s],White) ? White : Black;
          CLRFLAGMASK(spec[s],all_pieces_flags);
          SETFLAGMASK(spec[s],all_royals_flags);
          if (number_of_pieces[king_side][King]==1)
          {
            (*new_king_square)[king_side] = s;
            SETFLAG(spec[s],Royal);
          }
        }
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
        assert(!TSTFLAG(spec[*bnp],Royal));
      }
    }
  }
  else if (!locate_unique_royal(White,&(*new_king_square)[White])
           || !locate_unique_royal(Black,&(*new_king_square)[Black]))
  {
    VerifieMsg(OneKing);
    result = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean proofgame_restore_start(slice_index stipulation_root,
                                       twin_context_type context)
{
  boolean result = false;

  switch (find_unique_goal(stipulation_root).type)
  {
    case no_goal:
      VerifieMsg(MultipleGoalsWithProofGameNotAcceptable);
      break;

    case goal_proofgame:
      ProofInitialiseStartPosition();
      ProofRestoreStartPosition();
      result = true;
      break;

    case goal_atob:
      ProofRestoreStartPosition();
      if (!OptFlag[noboard] && context==twin_initial)
        ProofWriteStartPosition(stipulation_root);
      result = true;
      break;

    default:
      assert(0);
      break;
  }

  return result;
}

void initialise_piece_ids(void)
{
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  currPieceId = MinPieceId;
  for (bnp = boardnum; *bnp; ++bnp)
    if (!is_square_empty(*bnp))
    {
      assert(currPieceId<=MaxPieceId);
      SetPieceId(spec[*bnp],currPieceId++);
    }

  currPieceId = underworld_set_piece_ids(currPieceId);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void initialise_piece_flags_from_conditions(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

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

  if (CondFlag[ghostchess]  || CondFlag[hauntedchess])
    SETFLAG(some_pieces_flags,Uncapturable);

  {
    square const *bnp;
    for (bnp = boardnum; *bnp; ++bnp)
      if (!is_square_empty(*bnp))
      {
        piece_walk_type const p = get_walk_of_piece_on_square(*bnp);
        SETFLAGMASK(spec[*bnp],all_pieces_flags);

        SavePositionInDiagram(spec[*bnp],*bnp);

        if (TSTFLAG(spec[*bnp],ColourChange)
            && !(is_simplehopper(p)
                 || is_chineserider(p)
                 || is_lion(p)
                 || p==ContraGras))
        {
          /* relies on imitators already having been implemented */
          CLRFLAG(spec[*bnp],ColourChange);
          ErrorMsg(ColourChangeRestricted);
        }
      }
  }

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
  stip_moves_traversal_override_by_function(&st,
                                            slice_function_binary,
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

  (*is_restricted)[advers(slices[si].starter)] = true;

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

  (*is_restricted)[slices[si].starter] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void find_restricted_side_help(slice_index si,
                                      stip_structure_traversal *st)
{
  is_restricted_type * const is_restricted = st->param;
  stip_length_type const length = slices[si].u.branch.length;
  Side const starter = slices[si].starter;
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
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_conditional_pipe,
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
 * @return true iff the verification was successful
 */
static boolean verify_position(slice_index si)
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
  if (!CondFlag[einstein])
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

  im0 = isquare[0];
  if (! CondFlag[imitators])
    CondFlag[noiprom] = true;

  if (get_max_nr_moves(si) >= maxply-2)
  {
    VerifieMsg(BigNumMoves);
    return false;
  }

  if (circe_variant.relevant_capture==circe_relevant_capture_lastmove
      && stip_ends_in(si,goal_steingewinn))
  {
    VerifieMsg(PercentAndParrain);
    return false;
  }

  if (TSTFLAG(some_pieces_flags, HalfNeutral))
    SETFLAGMASK(some_pieces_flags,NeutralMask);

  if (CondFlag[backhome])
    SETFLAGMASK(some_pieces_flags,PieceIdMask);
  if (circe_variant.determine_rebirth_square==circe_determine_rebirth_square_diagram)
    SETFLAGMASK(some_pieces_flags,PieceIdMask);

  if (CondFlag[republican] && !republican_verifie_position(si))
    return false;

  if ((royal_square[Black]!=initsquare || royal_square[White]!=initsquare
       || CondFlag[white_oscillatingKs] || CondFlag[black_oscillatingKs]
       || circe_variant.is_rex_inclusive
       || immune_variant.is_rex_inclusive
       || TSTFLAG(some_pieces_flags,Royal))
      && (CondFlag[dynasty] || CondFlag[losingchess] || CondFlag[extinction]))
  {
    VerifieMsg(IncompatibleRoyalSettings);
    return false;
  }

  if (CondFlag[takemake])
  {
    if (CondFlag[sentinelles]
        || CondFlag[nocapture]
        || CondFlag[anticirce])
    {
      VerifieMsg(TakeMakeAndFairy);
      return false;
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
      if (piece_walk_may_exist[p] || promonly[p] || is_football_substitute[p])
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
          VerifieMsg(MagicAndFairyPieces);
          return false;
        }
        if (CondFlag[einstein])
        {
          VerifieMsg(EinsteinAndFairyPieces);
          return false;
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
      VerifieMsg(ImitWFairy);
      return false;
    }
    disable_orthodox_mating_move_optimisation(nr_sides);
  }

  if (CondFlag[leofamily])
  {
    piece_walk_type p;
    for (p = Queen; p<=Bishop; p++)
      if (number_of_pieces[White][p]+number_of_pieces[Black][p]!=0)
      {
        VerifieMsg(LeoFamAndOrtho);
        return false;
      }
  }

  if (CondFlag[chinoises])
    piece_walk_may_exist_fairy = true;

  if (CondFlag[circe])
  {
    if (piece_walk_may_exist[Dummy])
    {
      VerifieMsg(CirceAndDummy);
      return false;
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
      VerifieMsg(CantDecideOnSideWhichConditionAppliesTo);
      return false;
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
    VerifieMsg(TwoMummerCond);
    return false;
  }
  if (CondFlag[whmax] && !mummer_set_length_measurer(White,&maximummer_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }

  if (CondFlag[blmin] && !mummer_set_length_measurer(Black,&minimummer_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }
  if (CondFlag[whmin] && !mummer_set_length_measurer(White,&minimummer_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }

  if (CondFlag[blcapt] && !mummer_set_length_measurer(Black,&must_capture_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }
  if (CondFlag[whcapt] && !mummer_set_length_measurer(White,&must_capture_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }

  if (CondFlag[blfollow] && !mummer_set_length_measurer(Black,&follow_my_leader_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }
  if (CondFlag[whfollow] && !mummer_set_length_measurer(White,&follow_my_leader_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }

  if (CondFlag[duellist]
      && !(mummer_set_length_measurer(Black,&duellists_measure_length)
           && mummer_set_length_measurer(White,&duellists_measure_length)))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }

  if (CondFlag[blackalphabetic]
      && !mummer_set_length_measurer(Black,&alphabetic_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }
  if (CondFlag[whitealphabetic]
      && !mummer_set_length_measurer(White,&alphabetic_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }

  if (CondFlag[blacksynchron]
      && !mummer_set_length_measurer(Black,&synchronous_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }
  if (CondFlag[whitesynchron]
      && !mummer_set_length_measurer(White,&synchronous_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }

  if (CondFlag[blackantisynchron]
      && !mummer_set_length_measurer(Black,&antisynchronous_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }
  if (CondFlag[whiteantisynchron]
      && !mummer_set_length_measurer(White,&antisynchronous_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }

  /* the mummer logic is (ab)used to priorise transmuting king moves */
  if (CondFlag[blsupertrans_king]
      && !mummer_set_length_measurer(Black,&len_supertransmuting_kings))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }
  if (CondFlag[whsupertrans_king]
      && !mummer_set_length_measurer(White,&len_supertransmuting_kings))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }

  if (CondFlag[blforsqu]
      && !mummer_set_length_measurer(Black,&forced_squares_measure_length))
  {
    VerifieMsg(CantDecideOnSideWhichConditionAppliesTo);
    return false;
  }
  if (CondFlag[whforsqu]
      && !mummer_set_length_measurer(White,&forced_squares_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }

  if (CondFlag[blconforsqu]
      && !mummer_set_length_measurer(Black,&forced_squares_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }
  if (CondFlag[whconforsqu]
      && !mummer_set_length_measurer(White,&forced_squares_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }

  if (CondFlag[schwarzschacher]
      && !mummer_set_length_measurer(Black,&blackchecks_measure_length))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }


  if (ExtraCondFlag[ultraschachzwang])
  {
    Side const restricted_side = findRestrictedSide(si);
    if (restricted_side==no_side)
    {
      VerifieMsg(CantDecideOnSideWhichConditionAppliesTo);
      return false;
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
    if (number_of_pieces[White][Knight] + number_of_pieces[Black][Knight] > 0)
    {
      VerifieMsg(CavMajAndKnight);
      return false;
    }
    piece_walk_may_exist_fairy = true;
  }

  if (OptFlag[sansrb] && king_square[White]!=initsquare)
    OptFlag[sansrb] = false;

  if (OptFlag[sansrn] && king_square[Black]!=initsquare)
    OptFlag[sansrn] = false;

  if (king_square[White]==initsquare && number_of_pieces[White][King]==0 && !OptFlag[sansrb])
  {
    ErrorMsg(MissingKing);
    ErrorMsg(NewLine);
  }
  else if (king_square[Black]==initsquare && number_of_pieces[Black][King]==0 && !OptFlag[sansrn])
  {
    ErrorMsg(MissingKing);
    ErrorMsg(NewLine);
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
           && king_square[White]!=initsquare
           && get_walk_of_piece_on_square(king_square[White])!=standard_walks[King])
          || (!OptFlag[sansrn]
              && king_square[Black]!=initsquare
              && get_walk_of_piece_on_square(king_square[Black])!=standard_walks[King]))
      {
        VerifieMsg(RoyalPWCRexCirce);
        return false;
      }
    }
  }

  if (CondFlag[bicolores]
      || CondFlag[andernach]
      || CondFlag[antiandernach]
      || CondFlag[magicsquare]
      || CondFlag[volage]
      || CondFlag[masand]
      || CondFlag[dynasty]
      || TSTFLAG(some_pieces_flags,Magic))
  {
    if (is_piece_neutral(some_pieces_flags))
    {
      VerifieMsg(TooFairyForNeutral);
      return false;
    }
  }

  if (circe_variant.is_rex_inclusive
      || CondFlag[bicolores])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (CondFlag[backhome])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if (CondFlag[monochro] && CondFlag[bichro])
  {
    VerifieMsg(MonoAndBiChrom);
    return false;
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
    VerifieMsg(TooFairyForNeutral);
    return false;
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
    VerifieMsg(SuperCirceAndOthers);
    return false;
  }

  {
    int numsuper=0;
    if (circe_variant.determine_rebirth_square==circe_determine_rebirth_square_super) numsuper++;
    if (circe_variant.determine_rebirth_square==circe_determine_rebirth_square_cage) numsuper++;
    if (anticirce_variant.determine_rebirth_square==circe_determine_rebirth_square_super) numsuper++;
    if (numsuper>1)
    {
      VerifieMsg(SuperCirceAndOthers);
      return false;
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
    VerifieMsg(IsardamAndMadrasi);
    return false;
  }

  if (CondFlag[black_oscillatingKs] || CondFlag[white_oscillatingKs])
  {
    if (king_square[White]==initsquare || king_square[Black]==initsquare)
    {
      CondFlag[black_oscillatingKs] = false;
      CondFlag[white_oscillatingKs] = false;
    }
    else
      disable_orthodox_mating_move_optimisation(nr_sides);
  }

  if (CondFlag[mars] || CondFlag[antimars] || CondFlag[plus] || CondFlag[phantom])
  {
    if (CondFlag[mars]+CondFlag[antimars]+CondFlag[plus]+CondFlag[phantom]>1)
    {
      VerifieMsg(MarsCirceAndOthers);
      return false;
    }
    else if ((CondFlag[whvault_king] && vaulting_kings_transmuting[White])
             || (CondFlag[blvault_king] && vaulting_kings_transmuting[Black])
             || calc_reflective_king[White]
             || calc_reflective_king[Black]
             || CondFlag[bicolores]
             || CondFlag[sting])
    {
      VerifieMsg(MarsCirceAndOthers);
      return false;
    }
    else
      disable_orthodox_mating_move_optimisation(nr_sides);
  }

  if (CondFlag[madras] || CondFlag[eiffel] || CondFlag[isardam])
  {
    if ( CondFlag[imitators]|| TSTFLAG(some_pieces_flags,Paralysing))
    {
      VerifieMsg(MadrasiParaAndOthers);
      return false;
    }
  }

  if (circe_variant.on_occupied_rebirth_square==circe_on_occupied_rebirth_square_assassinate)
  {
    if (is_piece_neutral(some_pieces_flags) /* Neutrals not implemented */
        || CondFlag[bicolores])             /* others? */
    {
      VerifieMsg(AssassinandOthers);
      return false;
    }
  }

  if (circe_variant.is_rex_inclusive && immune_variant.is_rex_inclusive)
  {
    VerifieMsg(RexCirceImmun);
    return false;
  }

  if (immune_variant.is_rex_inclusive && CondFlag[anticirce])
  {
    VerifieMsg(SomeCondAndAntiCirce);
    return false;
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
      VerifieMsg(SomeCondAndAntiCirce);
      return false;
    }
  }

  if ((CondFlag[white_oscillatingKs] || CondFlag[black_oscillatingKs])
      && (OptFlag[sansrb] || OptFlag[sansrn]))
  {
    VerifieMsg(MissingKing);
    return false;
  }

  if (mummer_strictness[White]==mummer_strictness_ultra && !CondFlag[whcapt])
  {
    if (is_piece_neutral(some_pieces_flags))
    {
      VerifieMsg(TooFairyForNeutral);
      return false;
    }
  }

  if (mummer_strictness[Black]==mummer_strictness_ultra && !CondFlag[blcapt])
  {
    if (is_piece_neutral(some_pieces_flags))
    {
      VerifieMsg(TooFairyForNeutral);
      return false;
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
    VerifieMsg(SomePiecesAndHeffa);
    return false;
  }

  if (CondFlag[ghostchess] || CondFlag[hauntedchess])
  {
    if (CondFlag[circe] || CondFlag[anticirce]
        || CondFlag[haanerchess]
        || TSTFLAG(some_pieces_flags,Kamikaze)
        || (CondFlag[ghostchess] && CondFlag[hauntedchess]))
    {
      VerifieMsg(GhostHauntedChessAndCirceKamikazeHaanIncompatible);
      return false;
    }
  }

  change_moving_piece=
      TSTFLAG(some_pieces_flags, Kamikaze)
      || TSTFLAG(some_pieces_flags, Protean)
      || CondFlag[tibet]
      || CondFlag[andernach]
      || CondFlag[antiandernach]
      || CondFlag[magicsquare]
      || TSTFLAG(some_pieces_flags, Chameleon)
      || CondFlag[einstein]
      || CondFlag[reveinstein]
      || CondFlag[antieinstein]
      || CondFlag[volage]
      || TSTFLAG(some_pieces_flags, Volage)
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
      || (king_square[White]!=initsquare && get_walk_of_piece_on_square(king_square[White])!=King)
      || (king_square[Black]!=initsquare && get_walk_of_piece_on_square(king_square[Black])!=King)
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
            check_piece_index++;
          }
        }
      }

    if (CondFlag[whvault_king])
    {
      unsigned int i;
      for (i = 0; i!=nr_king_vaulters[White]; ++i)
        if (king_vaulters[White][i]>Bishop && !piece_walk_may_exist[king_vaulters[White][i]])
        {
          piece_walk_may_exist_fairy = true;
          checkpieces[check_piece_index] = king_vaulters[White][i];
          check_piece_index++;
          break;
        }
    }
    if (CondFlag[blvault_king])
    {
      unsigned int i;
      for (i = 0; i!=nr_king_vaulters[Black]; ++i)
        if (king_vaulters[Black][i]>Bishop && !piece_walk_may_exist[king_vaulters[Black][i]])
        {
          piece_walk_may_exist_fairy = true;
          checkpieces[check_piece_index] = king_vaulters[Black][i];
          check_piece_index++;
          break;
        }
    }
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
      VerifieMsg(KamikazeAndHaaner);
      return false;
    }
    if (CondFlag[circe]) {
      /* No Kamikaze and Circe with fairy pieces; taking and
         taken piece could be reborn on the same square! */
      if (piece_walk_may_exist_fairy || CondFlag[volage])
      {
        VerifieMsg(KamikazeAndSomeCond);
        return false;
      }
    }
    king_capture_avoiders_avoid_own();
  }

  if ((CondFlag[singlebox]  && SingleBoxType==ConditionType1))
  {
    if (piece_walk_may_exist_fairy)
    {
      VerifieMsg(SingleBoxAndFairyPieces);
      return false;
    }
  }

  if (CondFlag[whtrans_king] || CondFlag[whsupertrans_king] || CondFlag[whrefl_king]
      || CondFlag[bltrans_king] || CondFlag[blsupertrans_king] || CondFlag[blrefl_king])
    transmuting_kings_init_transmuters_sequence();

  if (calc_reflective_king[White] || calc_reflective_king[Black])
    disable_orthodox_mating_move_optimisation(nr_sides);

  if ((calc_reflective_king[White]
       && king_square[White]!=initsquare
       && (get_walk_of_piece_on_square(king_square[White])!=King
           || CondFlag[sting]))
      || (calc_reflective_king[Black]
          && king_square[Black]!=initsquare
          && (get_walk_of_piece_on_square(king_square[Black])!=King
              || CondFlag[sting])))
  {
    VerifieMsg(TransmRoyalPieces);
    return false;
  }

  if ((piece_walk_may_exist[Orphan] || piece_walk_may_exist[Friend])
      && is_piece_neutral(some_pieces_flags))
  {
    VerifieMsg(TooFairyForNeutral);
    return false;
  }

  if (((CondFlag[isardam] && isardam_variant==ConditionTypeA) || CondFlag[brunner])
      && CondFlag[vogt])
  {
    VerifieMsg(VogtlanderandIsardam);
    return false;
  }

  if ((CondFlag[chamchess] || CondFlag[linechamchess])
      && TSTFLAG(some_pieces_flags, Chameleon))
  {
    VerifieMsg(ChameleonPiecesAndChess);
    return false;
  }

  if (CondFlag[platzwechselrochade] && CondFlag[haanerchess])
  {
    VerifieMsg(NonsenseCombination);
    return false;
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
      VerifieMsg(AnnanChessAndConditionRecoloringPieces);
      return false;
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
      VerifieMsg(LosingChessNotInCheckOrMateStipulations);
      return false;
    }

    /* capturing moves are "longer" than non-capturing moves */
    if (!(mummer_set_length_measurer(Black,&must_capture_measure_length)
          && mummer_set_length_measurer(White,&must_capture_measure_length)))
    {
      VerifieMsg(TwoMummerCond);
      return false;
    }
  }

  /* check castling possibilities */
  CLEARFL(castling_flag);

  if ((get_walk_of_piece_on_square(square_e1)== standard_walks[King]) && TSTFLAG(spec[square_e1], White)
      && (!CondFlag[dynasty] || number_of_pieces[White][standard_walks[King]]==1))
    SETCASTLINGFLAGMASK(White,k_cancastle);
  if ((get_walk_of_piece_on_square(square_h1)== standard_walks[Rook]) && TSTFLAG(spec[square_h1], White))
    SETCASTLINGFLAGMASK(White,rh_cancastle);
  if ((get_walk_of_piece_on_square(square_a1)== standard_walks[Rook]) && TSTFLAG(spec[square_a1], White))
    SETCASTLINGFLAGMASK(White,ra_cancastle);
  if ((get_walk_of_piece_on_square(square_e8)== standard_walks[King]) && TSTFLAG(spec[square_e8], Black)
      && (!CondFlag[dynasty] || number_of_pieces[Black][standard_walks[King]]==1))
    SETCASTLINGFLAGMASK(Black,k_cancastle);
  if ((get_walk_of_piece_on_square(square_h8)== standard_walks[Rook]) && TSTFLAG(spec[square_h8], Black))
    SETCASTLINGFLAGMASK(Black,rh_cancastle);
  if ((get_walk_of_piece_on_square(square_a8)== standard_walks[Rook]) && TSTFLAG(spec[square_a8], Black))
    SETCASTLINGFLAGMASK(Black,ra_cancastle);

  castling_flag &= castling_flags_no_castling;

  /* a small hack to enable ep keys */
  trait[1] = no_side;

  if (CondFlag[exclusive] && !exclusive_verifie_position(si))
    return false;

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
      || CondFlag[mars] || CondFlag[plus]|| CondFlag[phantom]
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

  return true;
}

static void take_back_retro(void)
{
  if (CondFlag[lastcapture])
    circe_parrain_undo_retro_capture();
  else if (OptFlag[enpassant])
    en_passant_undo_multistep();
}

static void replay_retro(void)
{
  if (CondFlag[lastcapture])
    circe_parrain_redo_retro_capture();
  else
  {
    unsigned int i;
    for (i = 0; i!=move_effect_journal_index_offset_capture; ++i)
      move_effect_journal_do_null_effect();

    move_effect_journal_do_no_piece_removal();

    if (OptFlag[enpassant])
      en_passant_redo_multistep();
    else
      move_effect_journal_do_null_effect();
  }
}

static void solve_any_stipulation(slice_index stipulation_root_hook)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",stipulation_root_hook);
  TraceFunctionParamListEnd();

  if (verify_position(slices[stipulation_root_hook].next1))
  {
    move_effect_journal_reset();

    {
      slice_index const solving_machinery = build_solvers(stipulation_root_hook);
      TraceStipulation(solving_machinery);

      nextply(no_side);
      assert(nbply==ply_retro_move_takeback);

      take_back_retro();

      /* Make sure that trait is different from that of the first move (or there
       * will be no e.p. capture). */
      nextply(no_side);
      assert(nbply==ply_retro_move);

      replay_retro();

      solve(solving_machinery);

      /* undo retro replay */
      undo_move_effects();

      finply();

      /* undo retro takeback */
      undo_move_effects();

      finply();

      dealloc_slices(solving_machinery);

      slack_length = 0;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void solve_proofgame_stipulation(slice_index stipulation_root_hook,
                                        twin_context_type context)
{
  slice_index const stipulation_root = slices[stipulation_root_hook].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",stipulation_root_hook);
  TraceFunctionParam("%u",context);
  TraceFunctionParamListEnd();

  ProofSaveTargetPosition();

  if (proofgame_restore_start(stipulation_root,context))
  {
    countPieces();
    initialise_piece_ids();
    initialise_piece_flags_from_conditions();
    ProofInitialise(stipulation_root);
    if (locate_royals(&king_square))
      solve_any_stipulation(stipulation_root_hook);

    ProofRestoreTargetPosition();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void twin_solve_stipulation(slice_index stipulation_root_hook,
                                   twin_context_type context)
{
  move_effect_journal_index_type const save_king_square_horizon = king_square_horizon;
  square new_king_square[nr_sides] = { initsquare, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",stipulation_root_hook);
  TraceFunctionParam("%u",context);
  TraceFunctionParamListEnd();

  initialise_piece_walk_caches();
  countPieces();

  if (locate_royals(&new_king_square))
  {
    move_effect_journal_do_king_square_movement(move_effect_reason_diagram_setup,
                                                White,new_king_square[White]);
    move_effect_journal_do_king_square_movement(move_effect_reason_diagram_setup,
                                                Black,new_king_square[Black]);
    king_square_horizon = move_effect_journal_base[nbply+1];

    if (stip_ends_in(slices[stipulation_root_hook].next1,goal_proofgame)
        || stip_ends_in(slices[stipulation_root_hook].next1,goal_atob))
      solve_proofgame_stipulation(stipulation_root_hook,context);
    else
      solve_any_stipulation(stipulation_root_hook);
  }

  king_square_horizon = save_king_square_horizon;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
