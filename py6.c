/********************* MODIFICATIONS to py6.c **************************
 **
 ** Date       Who  What
 **
 ** 2006/05/09 SE   Bug fix: Halfneutral + AntiCirce
 **
 ** 2006/05/09 SE   New conditions: SAT, StrictSAT, SAT X Y (invented L.Salai sr.)
 **
 ** 2006/05/14 SE   New Condition: TakeMake (invented H.Laue)
 **
 ** 2006/05/17 SE   Changes to allow half-move specification for helpmates using 0.5 notation
 **                 Reset of maxsolutions changed for set play
 **                 SOme combinations with Take&MAke disallowed
 **
 ** 2006/06/30 SE   New condition: BGL (invented P.Petkov)
 **
 ** 2007/01/28 NG   New stipulation: help-reflexmate (hr)
 **
 ** 2006/07/30 SE   New condition: Schwarzschacher
 **
 ** 2006/07/30 SE   Extended movenum option to also print time when solving h#/sh# at next1 ply
 **
 ** 2007/01/28 SE   New condition: Annan Chess
 **
 ** 2007/05/01 SE   Extended Chopper types to eagles, mooses and sparrows
 **
 ** 2007/05/04 SE   Bugfix: SAT + BlackMustCapture
 **
 ** 2007/05/04 SE   Bugfix: SAT + Ultraschachzwang
 **
 ** 2007/07/04 SE   Bugfix: Ser-dia
 **
 ** 2007/11/08 SE   New conditions: Vaulting kings (invented: J.G.Ingram)
 **                 Transmuting/Reflecting Ks now take optional piece list
 **                 turning them into vaulting types
 **
 ** 2007/11/19 NG   Bugfix: intelligent mode with helpselfmate not supported
 **
 ** 2007/12/20 SE   New condition: Lortap (invented: F.H. von Meyenfeldt)
 **
 ** 2007/12/21 SE   Command-line switch: -b set low priority (Win32 only so far)
 **                 Command-line switch: -maxtime (same func as Option)
 **
 ** 2007/12/26 SE   New piece: Reverse Pawn (for below but independent)
 **                 New condition: Protean Chess
 **                 New piece type: Protean man (invent A.H.Kniest?)
 **                 (Differs from Frankfurt chess in that royal riders
 **                 are not 'non-passant'. Too hard to do but possibly
 **                 implement as an independent condition later).
 **
 ** 2008/01/02 NG   New condition: Geneva Chess
 **
 ** 2008/02/20 SE   Bugfix: Annan
 **
 ** 2008/02/10 SE   New condition: Cheameleon Pursuit (invented? : L.Grolman)
 **
 ** 2008/02/19 SE   New condition: AntiKoeko
 **
 ** 2008/02/25 SE   New piece type: Magic
 **                 Adjusted Masand code
 **
 ** 2008/03/13 SE   New condition: Castling Chess (invented: N.A.Bakke?)
 **
 ** 2009/01/03 SE   New condition: Disparate Chess (invented: R.Bedoni)
 **
 **                 New piece type: Patrol pieces
 **
 ** 2012/01/27 NG   AlphabeticChess now possible for white or black only.
 **
 ***************************** End of List ******************************/

#if defined(macintosh)    /* is always defined on macintosh's  SB */
#   define SEGM2
#   include "platform/unix/mac.h"
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#if defined(__unix)
#   include <unistd.h>        /* alarm() prototype */
#endif  /* __unix */

/* TurboC and BorlandC   TLi */
#if defined(__TURBOC__)
#   include <mem.h>
#   include <alloc.h>
#   include <conio.h>
#endif  /* __TURBOC__ */

#if defined(_WIN32)
#include <process.h>
#endif  /* _WIN32 */

#include "py.h"
#include "py1.h"
#include "pyproc.h"
#include "pydata.h"
#include "pymsg.h"
#include "DHT/dhtbcmem.h"
#include "pyproof.h"
#include "solving/selfcheck_guard.h"
#include "solving/moving_pawn_promotion.h"
#include "stipulation/pipe.h"
#include "stipulation/proxy.h"
#include "solving/solve.h"
#include "output/output.h"
#include "conditions/bgl.h"
#include "conditions/contact_grid.h"
#include "conditions/exclusive.h"
#include "conditions/republican.h"
#include "conditions/blackchecks.h"
#include "conditions/extinction.h"
#include "conditions/madrasi.h"
#include "conditions/maff/immobility_tester.h"
#include "conditions/owu/immobility_tester.h"
#include "conditions/ohneschach/immobility_tester.h"
#include "conditions/ohneschach/legality_tester.h"
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
#include "conditions/circe/cage.h"
#include "conditions/circe/chameleon.h"
#include "conditions/circe/clone.h"
#include "conditions/circe/couscous.h"
#include "conditions/circe/double_agents.h"
#include "conditions/circe/kamikaze.h"
#include "conditions/circe/parrain.h"
#include "conditions/circe/volage.h"
#include "conditions/circe/promotion.h"
#include "conditions/circe/frischauf.h"
#include "conditions/circe/super.h"
#include "conditions/circe/april.h"
#include "conditions/circe/king_rebirth_avoider.h"
#include "conditions/circe/turncoats.h"
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
#include "conditions/amu/attack_counter.h"
#include "conditions/imitator.h"
#include "conditions/football.h"
#include "conditions/castling_chess.h"
#include "platform/maxmem.h"
#include "platform/maxtime.h"
#include "platform/pytime.h"
#include "platform/priority.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/branch.h"
#include "stipulation/end_of_branch.h"
#include "stipulation/end_of_branch_tester.h"
#include "stipulation/dead_end.h"
#include "stipulation/move_inverter.h"
#include "stipulation/boolean/and.h"
#include "stipulation/boolean/or.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/constraint.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/goals/prerequisite_guards.h"
#include "stipulation/temporary_hacks.h"
#include "stipulation/goals/immobile/reached_tester.h"
#include "stipulation/goals/doublemate/king_capture_avoider.h"
#include "stipulation/temporary_hacks.h"
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
#include "solving/en_passant.h"
#include "solving/move_generator.h"
#include "solving/moving_pawn_promotion.h"
#include "solving/post_move_iteration.h"
#include "pieces/walks.h"
#include "pieces/attributes/magic.h"
#include "pieces/attributes/paralysing/paralysing.h"
#include "pieces/attributes/kamikaze/kamikaze.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "pieces/attributes/neutral/half.h"
#include "pieces/attributes/hurdle_colour_changing.h"
#include "pieces/attributes/chameleon.h"
#include "pieces/attributes/chameleon.h"
#include "conditions/amu/mate_filter.h"
#include "conditions/circe/goal_filters.h"
#include "conditions/anticirce/goal_filters.h"
#include "pieces/attributes/chameleon.h"
#include "conditions/ultraschachzwang/goal_filter.h"
#include "options/no_short_variations/no_short_variations.h"
#include "optimisations/hash.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/keepmating.h"
#include "optimisations/goals/optimisation_guards.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_move_generator.h"
#include "optimisations/killer_move/killer_move.h"
#include "optimisations/killer_move/final_defense_move.h"
#include "options/quodlibet.h"
#include "options/goal_is_end.h"
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
#include "debugging/trace.h"
#include "debugging/measure.h"
#ifdef _SE_
#include "se.h"
#endif

static boolean is_rider(PieNam p)
{
  switch (p)
  {
    case    NightRider:
    case    Amazone:
    case    Empress:
    case    Princess:
    case    Waran:
    case    Camelrider:
    case    Zebrarider:
    case    Gnurider:
    case    RookHunter:
    case    BishopHunter:
    case    Rook:
    case    Queen:
    case    Bishop:
    case    WesirRider:
    case    FersRider:
      return true;

    default:
      return false;
  }
}

static boolean is_leaper(PieNam p)
{
  switch (p)
  {
    case    Zebra:
    case    Camel:
    case    Giraffe:
    case    RootFiftyLeaper:
    case    Bucephale:
    case    Wesir:
    case    Alfil:
    case    Fers:
    case    Dabbaba:
    case    Gnu:
    case    Antilope:
    case    Squirrel:
    case    ErlKing:
    case    Okapi:
    case    Knight:
    case    Leap15:
    case    Leap16:
    case    Leap24:
    case    Leap25:
    case    Leap35:
    case    Leap37:
    case    Leap36:
      return true;
    default:  return false;
  }
} /* is_leaper */

static boolean is_simplehopper(PieNam p)
{
  switch (p)
  {
    case Grasshopper:
    case NightriderHopper:
    case CamelHopper:
    case ZebraHopper:
    case GnuHopper:
    case EquiHopper:
    case NonStopEquihopper:
    case GrassHopper2:
    case GrassHopper3:
    case KingHopper:
    case Orix:
    case NonStopOrix:
    case Elk:
    case RookMoose:
    case BishopMoose:
    case Eagle:
    case RookEagle:
    case BishopEagle:
    case Sparrow:
    case RookSparrow:
    case BishopSparrow:
      return true;

    default:
      return false;
  }
}

static boolean is_simpledecomposedleaper(PieNam p)
{
  switch (p)
  {
    case Mao:
    case Moa:
      return true;

    default:
      return false;
  }
}

static boolean is_symmetricfairy(PieNam p)
{
  /* any piece where, if p captures X is legal, then it's also legal if p and X are swapped */
  switch (p)
  {
    case Leo:
    case Vao:
    case Pao:
    case Lion:
    case RookLion:
    case BishopLion:
      return true;

    default:
      return false;
  }
}


static boolean SetKing(int *kingsquare, int square)
{
  if (*kingsquare==initsquare)
  {
    *kingsquare = square;
    return true;
  }
  else
    return false;
}

static void initPieces(void)
{
  PieNam p;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  initalise_standard_walks();

  for (p = Empty; p<PieceCount; ++p)
    exist[p] = false;

  for (p = Empty; p<=Bishop; p++)
    exist[standard_walks[p]] = true;

  if (CondFlag[sentinelles])
  {
    exist[sentinelle[Black]] = true;
    exist[sentinelle[White]] = true;
  }

  if (CondFlag[chinoises])
    for (p = Leo; p<=Vao; ++p)
      exist[p] = true;

  for (p = (CondFlag[losingchess] || CondFlag[dynasty] || CondFlag[extinction]
            ? King
            : Queen);
       p < PieceCount;
       ++p)
    if (promonly[p] || is_football_substitute[p])
      exist[p] = true;

  if (CondFlag[protean])
    exist[ReversePawn] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void countPieces(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    piece p;
    for (p = roib; p<derbla; ++p)
    {
      nbpiece[p] = 0;
      nbpiece[-p] = 0;
    }
  }

  {
    square const *bnp;
    for (bnp = boardnum; *bnp; ++bnp)
    {
      piece const p = e[*bnp];

      if (p!=vide)
      {
        exist[abs(p)] = true;
        ++nbpiece[p];
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean locateRoyal(void)
{
  king_square[White] = initsquare;
  king_square[Black] = initsquare;

  if (TSTFLAG(PieSpExFlags,Neutral))
    /* neutral king has to be white for initialisation of r[bn] */
    initialise_neutrals(White);

  if (CondFlag[dynasty])
  {
    square const *bnp;
    square s;

    OptFlag[sansrn] = true;
    OptFlag[sansrb] = true;

    if (nbpiece[roib]==1)
      for (bnp = boardnum; *bnp; bnp++)
      {
        s = *bnp;
        if (e[s]==roib)
        {
          if (!SetKing(&king_square[White],s))
          {
            VerifieMsg(OneKing);
            return false;
          }
          break;
        }
      }
    else
      king_square[White] = initsquare;

    if (nbpiece[roin]==1)
      for (bnp = boardnum; *bnp; bnp++)
      {
        s = *bnp;
        if (e[s]==roin)
        {
          if (!SetKing(&king_square[Black],s))
          {
            VerifieMsg(OneKing);
            return false;
          }
          break;
        }
      }
    else
      king_square[Black] = initsquare;
  }
  else if (CondFlag[losingchess] || CondFlag[extinction])
  {
    OptFlag[sansrn] = true;
    OptFlag[sansrb] = true;
  }
  else
  {
    square const *bnp;
    for (bnp = boardnum; *bnp; bnp++)
    {
      square s = *bnp;
      piece p = e[s];
      if (p==roib
          || (p>roib && TSTFLAG(spec[s],Royal)))
      {
        if (!SetKing(&king_square[White],s))
        {
          VerifieMsg(OneKing);
          return false;
        }
        if (TSTFLAG(spec[s],Neutral))
          SetKing(&king_square[Black],s);
      }

      if (s==royal_square[White])
      {
        if (!SetKing(&king_square[White],s))
        {
          VerifieMsg(OneKing);
          return false;
        }
      }

      if (p==roin
          || (p<roin && TSTFLAG(spec[s],Royal)))
      {
        if (!SetKing(&king_square[Black],s))
        {
          VerifieMsg(OneKing);
          return false;
        }
      }

      if (s==royal_square[Black])
      {
        if (!SetKing(&king_square[Black],s))
        {
          VerifieMsg(OneKing);
          return false;
        }
      }
    }
  }

  return true;
}

/* initialize elmt at index NullPieceId with initsquare */
square PiecePositionsInDiagram[MaxPieceId+1] = { initsquare };

static void initialise_piece_flags(void)
{
  square const *bnp;
  PieceIdType id = MinPieceId;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp; bnp++)
  {
    piece const p = e[*bnp];
    if (p!=vide)
    {
      if (CondFlag[volage] && king_square[White]!=*bnp && king_square[Black]!=*bnp)
        SETFLAG(spec[*bnp], Volage);

      assert(id<=MaxPieceId);
      SetPieceId(spec[*bnp], id);
      ++id;
      SavePositionInDiagram(spec[*bnp], *bnp);

      if (TSTFLAG(spec[*bnp],ColourChange)
          && !is_simplehopper(abs(e[*bnp])))
      {
        /* relies on imitators already having been implemented */
        CLRFLAG(spec[*bnp],ColourChange);
        ErrorMsg(ColourChangeRestricted);
      }

      if (abs(e[*bnp])==King
          && !CondFlag[losingchess]
          && !CondFlag[extinction]
          && (!CondFlag[dynasty] || nbpiece[e[*bnp]]==1))
        SETFLAG(spec[*bnp],Royal);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
  Side const restricted_side = ((length-slack_length)%2==1
                                ? advers(starter)
                                : starter);

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

/* Verify the user input and our interpretation of it
 * @param si identifies the root slice of the representation of the
 *           stipulation
 * @return true iff the verification was successful
 */
static boolean verify_position(slice_index si)
{
  square const *bnp;
  int      tp, op;
  boolean flagleofamilyonly;
  boolean flagveryfairy = false;
  boolean flagsymmetricfairy = false;
  boolean flagsimplehoppers = false;

  supergenre = false;
  reset_ortho_mating_moves_generation_obstacles();
  reset_killer_move_optimisation();
  reset_orthodox_mating_move_optimisation();

  reset_countnropponentmoves_defense_move_optimisation();

  if (CondFlag[glasgow] && CondFlag[circemalefique])
    anycirprom = true;

  /* initialize promotion squares */
  if (!CondFlag[einstein])
  {
    square i;
    if (!CondFlag[whprom_sq])
      for (i = 0; i < 8; i++)
        SETFLAG(sq_spec[CondFlag[glasgow] ? square_h7-i : square_h8-i],
                WhPromSq);

    if (!CondFlag[blprom_sq])
      for (i = 0; i < 8; i++)
        SETFLAG(sq_spec[CondFlag[glasgow] ? square_a2+i : square_a1+i],
                BlPromSq);
  }

  im0 = isquare[0];
  if (! CondFlag[imitators])
    CondFlag[noiprom] = true;

  if (get_max_nr_moves(si) >= maxply-2)
  {
    VerifieMsg(BigNumMoves);
    return false;
  }

  if (anyparrain && stip_ends_in(si,goal_steingewinn))
  {
    VerifieMsg(PercentAndParrain);
    return false;
  }

  if (TSTFLAG(PieSpExFlags, HalfNeutral))
    SETFLAG(PieSpExFlags, Neutral);

  if (CondFlag[republican] && !republican_verifie_position(si))
    return false;

  if ((royal_square[Black]!=initsquare || royal_square[White]!=initsquare
       || CondFlag[white_oscillatingKs] || CondFlag[black_oscillatingKs]
       || rex_circe
       || rex_immun)
      && (CondFlag[dynasty] || CondFlag[losingchess] || CondFlag[extinction]))
  {
    VerifieMsg(IncompatibleRoyalSettings);
    return false;
  }

  if (CondFlag[takemake])
  {
    if (CondFlag[sentinelles]
        || CondFlag[nocapture]
        || anyanticirce)
    {
      VerifieMsg(TakeMakeAndFairy);
      return false;
    }
  }

  if (CondFlag[immuncage] && !CondFlag[circecage])
  {
    VerifieMsg(NoCageImmuneWithoutCage);
    CondFlag[immuncage] = false;
  }

#ifdef _SE_DECORATE_SOLUTION_
  se_init();
#endif
  flagleofamilyonly = CondFlag[leofamily];

  {
    PieNam p;
    for (p = Bishop+1; p<PieceCount; ++p)
    {
      if (exist[p] || promonly[p] || is_football_substitute[p])
      {
        flagfee = true;
        if (is_rider(p)) {}
        else if (is_leaper(p)) {}
        else if (is_simplehopper(p))
          flagsimplehoppers = true;
        else if (is_simpledecomposedleaper(p)) {}
        else if (is_symmetricfairy(p))
          flagsymmetricfairy = true;
        else
        {
          if (!is_pawn(p) && p!=Dummy && (p<Leo || p>Vao))
            flagleofamilyonly = false;
          flagveryfairy = true;
        }
        if (TSTFLAG(PieSpExFlags,Magic) && !magic_is_piece_supported(p))
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

  /* otherwise, the optimisation would be correct, too, but we
   * wouldn't care */
  optim_neutralretractable = TSTFLAG(PieSpExFlags,Neutral);

  if (CondFlag[sting])
  {
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
    flagfee = true;
    flagsimplehoppers = true;
  }

  flaglegalsquare=
      TSTFLAG(PieSpExFlags, Jigger)
      || CondFlag[newkoeko]
      || CondFlag[gridchess] || CondFlag[koeko] || CondFlag[antikoeko]
      || CondFlag[blackedge] || CondFlag[whiteedge]
      || CondFlag[geneva];

  if (CondFlag[imitators])
  {
    if (flagveryfairy
        || flagsymmetricfairy
        || flaglegalsquare
        || CondFlag[chinoises]
        || anyimmun
        || CondFlag[haanerchess]
        || anycirce           /* rebirth square may coincide with I */
        || anyanticirce       /* rebirth square may coincide with I */
        || anyparrain) /* verkraftet nicht 2 IUW in einem Zug !!! */
    {
      VerifieMsg(ImitWFairy);
      return false;
    }
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
  }

  if (CondFlag[leofamily])
  {
    piece p;
    for (p = db; p<=fb; p++)
    {
      if (nbpiece[p]+nbpiece[-p]!=0)
      {
        VerifieMsg(LeoFamAndOrtho);
        return false;
      }
    }
  }

  if (CondFlag[chinoises])
    flagfee = true;

  if (anycirce)
  {
    if (exist[Dummy])
    {
      VerifieMsg(CirceAndDummy);
      return false;
    }
    if (TSTFLAG(PieSpExFlags, Neutral)
        || CondFlag[volage] || TSTFLAG(PieSpExFlags,Volage))
    {
      optim_neutralretractable = false;
      add_ortho_mating_moves_generation_obstacle();
    }
  }

  if (flagmaxi)
  {
    Side const restricted_side = findRestrictedSide(si);
    if (restricted_side==no_side)
    {
      VerifieMsg(CantDecideOnSideWhichConditionAppliesTo);
      return false;
    }
    else if (restricted_side==Black)
    {
      CondFlag[blmax] = true;
      CondFlag[whmax] = false;
      ultra_mummer[Black] = CondFlag[ultra];
      bl_exact = CondFlag[exact];
      measure_length[Black] = len_max;
      flagmummer[Black] = true;
      flagmummer[White] = false;
    }
    else
    {
      CondFlag[blmax] = false;
      CondFlag[whmax] = true;
      ultra_mummer[White] = CondFlag[ultra];
      wh_exact = CondFlag[exact];
      measure_length[White] = len_max;
      flagmummer[White] = true;
      flagmummer[Black] = false;
    }
  }

  if (flagultraschachzwang)
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

      optim_neutralretractable = false;
      add_ortho_mating_moves_generation_obstacle();
    }
  }

  if (CondFlag[cavaliermajeur])
  {
    if (nbpiece[cb] + nbpiece[cn] > 0)
    {
      VerifieMsg(CavMajAndKnight);
      return false;
    }
    flagfee = true;
  }

  if (OptFlag[sansrb] && king_square[White]!=initsquare)
    OptFlag[sansrb] = false;

  if (OptFlag[sansrn] && king_square[Black]!=initsquare)
    OptFlag[sansrn] = false;

  if (king_square[White]==initsquare && nbpiece[roib]==0
      && !OptFlag[sansrb])
    ErrorMsg(MissingKing);

  if (king_square[Black]==initsquare && nbpiece[roin]==0
      && !OptFlag[sansrn])
    ErrorMsg(MissingKing);

  if (rex_circe) {
    /* why not royal pieces in PWC ??? TLi */
    /* Because we loose track of the royal attribute somewhere and
       I didn't find where ... NG
    */
    if (CondFlag[circeequipollents]
        || CondFlag[circeclone]
        || CondFlag[couscous]
        || CondFlag[circeclonemalefique])
    {
      /* disallowed because of the call to (*circerenai)
         in echecc */
      /* would require knowledge of id. Other forms now allowed
       */
      if (((! OptFlag[sansrb]) && king_square[White]!=initsquare && (e[king_square[White]] != roib))
          || ((! OptFlag[sansrn]) && king_square[Black]!=initsquare && (e[king_square[Black]] != roin)))
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
      || TSTFLAG(PieSpExFlags,Magic))
  {
    if (TSTFLAG(PieSpExFlags, Neutral))
    {
      VerifieMsg(TooFairyForNeutral);
      return false;
    }
  }

  if (CondFlag[bicolores])
    add_ortho_mating_moves_generation_obstacle();

  if (CondFlag[bichro] || CondFlag[monochro])
    add_ortho_mating_moves_generation_obstacle();

  eval_2 = eval_white = eval_ortho;
  rechec[White] = &orig_rbechec;
  rechec[Black] = &orig_rnechec;

  flaglegalsquare = flaglegalsquare
      || CondFlag[bichro]
      || CondFlag[monochro];

  if (flaglegalsquare)
  {
    eval_white = legalsquare;
    eval_2 = eval_ortho;
    if (CondFlag[monochro] && CondFlag[bichro])
    {
      VerifieMsg(MonoAndBiChrom);
      return false;
    }
  }

  if ((CondFlag[koeko]
       || CondFlag[newkoeko]
       || CondFlag[antikoeko]
       || TSTFLAG(PieSpExFlags, Jigger))
      && anycirce
      && TSTFLAG(PieSpExFlags, Neutral))
  {
    VerifieMsg(TooFairyForNeutral);
    return false;
  }

  if (flaglegalsquare && CondFlag[volage])
  {
    VerifieMsg(SomeCondAndVolage);
    return false;
  }

  if (TSTFLAG(PieSpExFlags, Kamikaze))
  {
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
    if (CondFlag[haanerchess])
    {
      VerifieMsg(KamikazeAndHaaner);
      return false;
    }
    if (anycirce) {
      /* No Kamikaze and Circe with fairy pieces; taking and
         taken piece could be reborn on the same square! */
      if (flagfee || CondFlag[volage])
      {
        VerifieMsg(KamikazeAndSomeCond);
        return false;
      }
    }
  }

  if ((CondFlag[supercirce] || CondFlag[april] || CondFlag[circecage])
      && (CondFlag[koeko] || CondFlag[newkoeko] || CondFlag[antikoeko]))
  {
    VerifieMsg(SuperCirceAndOthers);
    return false;
  }

  {
    int numsuper=0;
    if (CondFlag[supercirce]) numsuper++;
    if (CondFlag[circecage]) numsuper++;
    if (CondFlag[april]) numsuper++;
    if (CondFlag[antisuper]) numsuper++;
    if (numsuper>1
        || (supergenre && numsuper>0))
    {
      VerifieMsg(SuperCirceAndOthers);
      return false;
    }
  }

  if (CondFlag[patrouille]
      || CondFlag[beamten]
      || CondFlag[provacateurs]
      || CondFlag[central]
      || TSTFLAG(PieSpExFlags, Beamtet)
      || CondFlag[ultrapatrouille]
      || CondFlag[lortap]
      || TSTFLAG(PieSpExFlags, Patrol))
  {
    eval_2 = eval_white;
    eval_white = soutenu;
    obsfriendgenre = CondFlag[patrouille] ||
                     CondFlag[central] ||
                     CondFlag[ultrapatrouille] ||
                     CondFlag[lortap];
    obsenemygenre =  CondFlag[beamten] ||
                     CondFlag[provacateurs];
    obsenemyantigenre = false;
    obsfriendantigenre = CondFlag[lortap];
    obsenemyultragenre = CondFlag[beamten];
    obsfriendultragenre = CondFlag[ultrapatrouille] || CondFlag[central];
    obspieces = TSTFLAG(PieSpExFlags, Beamtet) || TSTFLAG(PieSpExFlags, Patrol);
    obsultra = obsenemyultragenre || obsfriendultragenre || TSTFLAG(PieSpExFlags, Beamtet);
    obsgenre = true;
  }

  if (TSTFLAG(PieSpExFlags,Paralyse)
      && !obsgenre)
  {
    eval_2 = eval_white;
    eval_white = paraechecc;
  }

  if (CondFlag[disparate])
  {
    eval_white = eval_black = eval_disparate;
    add_ortho_mating_moves_generation_obstacle();
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
      CondFlag[black_oscillatingKs] = CondFlag[white_oscillatingKs] = false;
    else
    {
      optim_neutralretractable = false;
      add_ortho_mating_moves_generation_obstacle();
    }
  }
  if (CondFlag[black_oscillatingKs] && OscillatingKingsTypeC[White]
      && CondFlag[white_oscillatingKs] && OscillatingKingsTypeC[White])
    CondFlag[swappingkings] = true;

  if (anymars||anyantimars) {
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
    if (calc_transmuting_king[White]
        || calc_reflective_king[White]
        || calc_transmuting_king[Black]
        || calc_reflective_king[Black]
        || CondFlag[bicolores]
        || CondFlag[sting]
        || flagsimplehoppers
        || (flagveryfairy && !flagleofamilyonly) )
    {
      VerifieMsg(MarsCirceAndOthers);
      return false;
    }
  }

  if (CondFlag[BGL])
  {
    eval_white = eval_BGL;
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
  }

  if (CondFlag[madras] || CondFlag[eiffel] || CondFlag[isardam])
  {
    if ( CondFlag[imitators]
         || TSTFLAG(PieSpExFlags,Paralyse))
    {
      VerifieMsg(MadrasiParaAndOthers);
      return false;
    }
    if (!obsgenre)
    {
      eval_2 = eval_white;
      if (CondFlag[madras])
        eval_white = eval_madrasi;
      else if (CondFlag[eiffel])
        eval_white = eval_eiffel;
      else
        eval_white = eval_isardam;
    }
  }

  if (CondFlag[woozles]) {
    if (CondFlag[madras]
        || CondFlag[eiffel]
        || CondFlag[isardam]
        || CondFlag[imitators]
        || TSTFLAG(PieSpExFlags,Paralyse))
    {
      VerifieMsg(MadrasiParaAndOthers);
      return false;
    }
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
    eval_2 = eval_white;
    eval_white = eval_wooheff;
  }

  if (CondFlag[brunner])
    /* Isardam + Brunner may be possible!
     * in which case eval_brunner is the correct evaluation function to use!
    */
    eval_white = eval_brunner;

  if (CondFlag[isardam] && IsardamB)
    eval_white=eval_ortho;

  if (CondFlag[shieldedkings])
  {
    eval_white=eval_shielded;
    obsfriendgenre=true;
  }

  if (CondFlag[circeassassin]) {
    if (TSTFLAG(PieSpExFlags,Neutral) /* Neutrals not implemented */
        || CondFlag[bicolores])             /* others? */
    {
      VerifieMsg(AssassinandOthers);
      return false;
    }
  }
  eval_black = eval_white;
  if (rex_circe || rex_immun) {
    if (rex_circe && rex_immun)
    {
      VerifieMsg(RexCirceImmun);
      return false;
    }
    if (anyanticirce)
    {
      /* an additional pointer to evaluate-functions is
         required  TLi */
      VerifieMsg(SomeCondAndAntiCirce);
      return false;
    }
    eval_2 = eval_white;
    if (rex_circe) {
      eval_white = rbcircech;
      eval_black = rncircech;
      cirrenroib = (*circerenai)(roib, spec[king_square[White]], initsquare, initsquare, initsquare, Black);
      cirrenroin = (*circerenai)(roin, spec[king_square[Black]], initsquare, initsquare, initsquare, White);
    }
    else {
      eval_white = rbimmunech;
      eval_black = rnimmunech;
      immrenroib = (*immunrenai)(roib, spec[king_square[White]], initsquare, initsquare, initsquare, Black);
      immrenroin = (*immunrenai)(roin, spec[king_square[Black]], initsquare, initsquare, initsquare, White);
    }
  }

  if (anyanticirce) {
    if (CondFlag[couscous]
        || CondFlag[koeko]
        || CondFlag[newkoeko]
        || CondFlag[antikoeko]
        || (CondFlag[singlebox] && SingleBoxType==singlebox_type1)
        || CondFlag[geneva]
        || TSTFLAG(PieSpExFlags, Kamikaze))
    {
      VerifieMsg(SomeCondAndAntiCirce);
      return false;
    }
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
    eval_2 = eval_white;
    eval_white = rbanticircech;
    eval_black = rnanticircech;
  }

  if ((CondFlag[singlebox]  && SingleBoxType==singlebox_type1)) {
    if (flagfee)
    {
      VerifieMsg(SingleBoxAndFairyPieces);
      return false;
    }
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
    eval_2 = eval_white;
    eval_white = rbsingleboxtype1ech;
    eval_black = rnsingleboxtype1ech;
  }

  if ((CondFlag[singlebox]  && SingleBoxType==singlebox_type3)) {
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
    rechec[Black] = &singleboxtype3_rnechec;
    rechec[White] = &singleboxtype3_rbechec;
    gen_wh_piece = &singleboxtype3_gen_wh_piece;
    gen_bl_piece = &singleboxtype3_gen_bl_piece;
  }

  if ((CondFlag[white_oscillatingKs] || CondFlag[black_oscillatingKs])
      && (OptFlag[sansrb] || OptFlag[sansrn]))
  {
    VerifieMsg(MissingKing);
    return false;
  }


  if (ultra_mummer[White] && !CondFlag[whcapt]) {
    if (TSTFLAG(PieSpExFlags, Neutral))
    {
      VerifieMsg(TooFairyForNeutral);
      return false;
    }
    else
    {
      eval_2 = eval_white;
      eval_black = rnultraech;
    }
  }

  if (ultra_mummer[Black] && !CondFlag[blcapt]) {
    if (TSTFLAG(PieSpExFlags, Neutral))
    {
      VerifieMsg(TooFairyForNeutral);
      return false;
    }
    else
    {
      eval_2 = eval_white;
      eval_white = rbultraech;
    }
  }
  if (CondFlag[football])
  {
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
  }

  if ( ( CondFlag[whmin]
         + CondFlag[whmax]
         + CondFlag[whcapt]
         + (CondFlag[whforsqu] || CondFlag[whconforsqu])
         + CondFlag[whfollow]
         + CondFlag[duellist]
         + (CondFlag[alphabetic] || CondFlag[whitealphabetic])
	 + CondFlag[whitesynchron]
         + CondFlag[whiteantisynchron]) > 1
       || (CondFlag[blmin]
           + CondFlag[blmax]
           + CondFlag[blcapt]
           + (CondFlag[blforsqu] || CondFlag[blconforsqu])
           + CondFlag[blfollow]
           + CondFlag[duellist]
           + (CondFlag[alphabetic] || CondFlag[blackalphabetic])
           + CondFlag[blacksynchron]
           + CondFlag[blackantisynchron] > 1))
  {
    VerifieMsg(TwoMummerCond);
    return false;
  }
  if ((CondFlag[whmin]
       || CondFlag[blmin]
       || CondFlag[whmax]
       || CondFlag[blmax]
       || CondFlag[heffalumps]
       )
      && (exist[Rose]
          || exist[SpiralSpringer]
          || exist[UbiUbi]
          || exist[Hamster]
          || exist[Elk]
          || exist[Eagle]
          || exist[Sparrow]
          || exist[Archbishop]
          || exist[ReflectBishop]
          || exist[Cardinal]
          || exist[BoyScout]
          || exist[GirlScout]
          || exist[DiagonalSpiralSpringer]
          || exist[BouncyKnight]
          || exist[BouncyNightrider]
          || exist[CAT]
          || exist[RoseHopper]
          || exist[RoseLion]
          || exist[Rao]
          || exist[RookMoose]
          || exist[RookEagle]
          || exist[RookSparrow]
          || exist[BishopMoose]
          || exist[BishopEagle]
          || exist[BishopSparrow]
          || exist[DoubleGras]
          || exist[DoubleRookHopper]
          || exist[DoubleBishopper]))
  {
    VerifieMsg(SomePiecesAndMaxiHeffa);
    return false;
  }

  if (CondFlag[ghostchess] || CondFlag[hauntedchess])
  {
    if (anycirce || anyanticirce
        || CondFlag[haanerchess]
        || TSTFLAG(PieSpExFlags,Kamikaze)
        || (CondFlag[ghostchess] && CondFlag[hauntedchess]))
    {
      VerifieMsg(GhostHauntedChessAndCirceKamikazeHaanIncompatible);
      return false;
    }
    else
    {
      SETFLAG(PieSpExFlags,Uncapturable);
      optim_neutralretractable = false;
    }
  }

  change_moving_piece=
      TSTFLAG(PieSpExFlags, Kamikaze)
      || TSTFLAG(PieSpExFlags, Protean)
      || CondFlag[tibet]
      || CondFlag[andernach]
      || CondFlag[antiandernach]
      || CondFlag[magicsquare]
      || TSTFLAG(PieSpExFlags, Chameleon)
      || CondFlag[einstein]
      || CondFlag[reveinstein]
      || CondFlag[antieinstein]
      || CondFlag[volage]
      || TSTFLAG(PieSpExFlags, Volage)
      || CondFlag[degradierung]
      || CondFlag[norsk]
      || CondFlag[traitor]
      || CondFlag[linechamchess]
      || CondFlag[chamchess]
      || CondFlag[protean]
      || CondFlag[champursue];

  empilegenre=
      flaglegalsquare
      || obsgenre
      || CondFlag[imitators]
      || anyimmun
      || CondFlag[nocapture]
      || CondFlag[nowhcapture]
      || CondFlag[noblcapture]
      || TSTFLAG(spec[king_square[White]], Kamikaze)
      || TSTFLAG(spec[king_square[Black]], Kamikaze)
      || flagmummer[White]
      || flagmummer[Black]
      || TSTFLAG(PieSpExFlags, Paralyse)
      || CondFlag[vogt]
      || anyanticirce
      || anymars
      || anyantimars
      || (CondFlag[singlebox] && SingleBoxType==singlebox_type1)
      || CondFlag[messigny]
      || CondFlag[woozles]
      || CondFlag[nowhiteprom]
      || CondFlag[noblackprom]
      || CondFlag[antikings]
      || CondFlag[norsk]
      || CondFlag[SAT]
      || CondFlag[strictSAT]
      || CondFlag[takemake]
      || CondFlag[losingchess]
      || CondFlag[disparate]
      || CondFlag[ghostchess]
      || CondFlag[hauntedchess]
      || TSTFLAG(PieSpExFlags,Uncapturable);

  if (CondFlag[dynasty])
  {
    /* checking for TSTFLAG(spec[king_square[White]],Kamikaze) may not be sufficient
     * in dynasty */
    square s;

    for (bnp = boardnum; *bnp; bnp++)
    {
      s = *bnp;
      if (abs(e[s])==roib && TSTFLAG(spec[s],Kamikaze))
      {
        empilegenre = true;
        break;
      }
    }
  }

  if (TSTFLAG(PieSpExFlags, Jigger)
      || CondFlag[newkoeko]
      || CondFlag[koeko]
      || CondFlag[antikoeko]
      || anyparrain
      || flagmummer[White]
      || flagmummer[Black]
      || CondFlag[vogt]
      || (eval_white != eval_ortho
          && eval_white != legalsquare)
      || (king_square[White] != initsquare && abs(e[king_square[White]]) != King)
      || (king_square[Black] != initsquare && abs(e[king_square[Black]]) != King)
      || TSTFLAG(PieSpExFlags, Chameleon)
      || CondFlag[einstein]
      || CondFlag[reveinstein]
      || CondFlag[antieinstein]
      || CondFlag[degradierung]
      || CondFlag[norsk]
      || CondFlag[messigny]
      || CondFlag[linechamchess]
      || CondFlag[chamchess]
      || CondFlag[antikings]
      || TSTFLAG(PieSpExFlags, HalfNeutral)
      || CondFlag[geneva]
      || CondFlag[dynasty] /* TODO why? */
      || TSTFLAG(PieSpExFlags,Magic))
  {
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
  }

  init_promotion_pieces();

  if (CondFlag[football])
    init_football_substitutes();

  {
    PieNam p;
    PieNam firstprompiece;
    unsigned int check_piece_index = 0;

    if (CondFlag[losingchess] || CondFlag[dynasty] || CondFlag[extinction])
      firstprompiece = King;
    else if ((CondFlag[singlebox] && SingleBoxType!=singlebox_type1) || CondFlag[football])
      firstprompiece = Pawn;
    else
      firstprompiece = Queen;

    for (p = firstprompiece; p<PieceCount; ++p)
      if (exist[p])
      {
        if (p>Bishop && p!=Dummy) {
          /* only fairy pieces until now ! */
          optim_neutralretractable = false;
          add_ortho_mating_moves_generation_obstacle();
          if (p!=Hamster)
          {
            checkpieces[check_piece_index] = p;
            check_piece_index++;
          }
        }
      }

    checkpieces[check_piece_index] = Empty;
  }

  tp = 0;
  op = 0;
  {
    PieNam p;
    for (p = King; p<PieceCount; ++p) {
      if (exist[p] && p!=Dummy && p!=Hamster)
      {
        if (normaltranspieces[White])
          transmpieces[White][tp] = p;
        if (normaltranspieces[Black])
          transmpieces[Black][tp] = p;
        tp++;
        if (p!=Orphan
            && p!=Friend
            && (exist[Orphan] || exist[Friend]))
          orphanpieces[op++] = p;
      }
    }
  }

  if (normaltranspieces[White])
    transmpieces[White][tp] = Empty;
  if (normaltranspieces[Black])
    transmpieces[Black][tp] = Empty;

  if (calc_reflective_king[White] || calc_reflective_king[Black])
  {
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
  }
  orphanpieces[op] = Empty;

  if ((calc_reflective_king[White]
       && king_square[White] != initsquare
       && (e[king_square[White]] != roib || CondFlag[sting]))
      || (calc_reflective_king[Black]
          && king_square[Black] != initsquare
          && (e[king_square[Black]] != roin || CondFlag[sting])))
  {
    VerifieMsg(TransmRoyalPieces);
    return false;
  }

  if ((exist[Orphan]
       || exist[Friend]
       || calc_reflective_king[White]
       || calc_reflective_king[Black])
      && TSTFLAG(PieSpExFlags, Neutral))
  {
    VerifieMsg(TooFairyForNeutral);
    return false;
  }

  if ((eval_white==eval_isardam || eval_white==eval_brunner) && CondFlag[vogt])
  {
    VerifieMsg(VogtlanderandIsardam);
    return false;
  }

  if ((CondFlag[chamchess] || CondFlag[linechamchess])
      && TSTFLAG(PieSpExFlags, Chameleon))
  {
    VerifieMsg(ChameleonPiecesAndChess);
    return false;
  }

  if (CondFlag[platzwechselrochade] && CondFlag[haanerchess])
  {
    VerifieMsg(NonsenseCombination);
    return false;
  }

  if (TSTFLAG(PieSpExFlags, ColourChange))
  {
    checkhopim = true;
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
  }
  checkhopim |= CondFlag[imitators];

  if (CondFlag[sentinelles])
  {
    /* a nasty drawback */
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
  }

  if (CondFlag[annan])
  {
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
    rechec[White] = &annan_rbechec;
    rechec[Black] = &annan_rnechec;
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

    /* no king is ever in check */
    rechec[White] = &losingchess_rbnechec;
    rechec[Black] = &losingchess_rbnechec;

    /* capturing moves are "longer" than non-capturing moves */
    measure_length[Black] = &len_losingchess;
    measure_length[White] = &len_losingchess;
    flagmummer[White] = true;
    flagmummer[Black] = true;
  }

  /* check castling possibilities */
  CLEARFL(castling_flag[0]);
  /* castling_supported has to be adjusted if there are any problems */
  /* with castling and fairy conditions/pieces */
  castling_supported = !CondFlag[patience];

  complex_castling_through_flag = CondFlag[imitators];

  if (castling_supported) {
    if ((abs(e[square_e1])== King) && TSTFLAG(spec[square_e1], White)
        && (!CondFlag[dynasty] || nbpiece[roib]==1))
      SETCASTLINGFLAGMASK(0,White,k_cancastle);
    if ((abs(e[square_h1])== Rook) && TSTFLAG(spec[square_h1], White))
      SETCASTLINGFLAGMASK(0,White,rh_cancastle);
    if ((abs(e[square_a1])== Rook) && TSTFLAG(spec[square_a1], White))
      SETCASTLINGFLAGMASK(0,White,ra_cancastle);
    if ((abs(e[square_e8])== King) && TSTFLAG(spec[square_e8], Black)
        && (!CondFlag[dynasty] || nbpiece[roin]==1))
      SETCASTLINGFLAGMASK(0,Black,k_cancastle);
    if ((abs(e[square_h8])== Rook) && TSTFLAG(spec[square_h8], Black))
      SETCASTLINGFLAGMASK(0,Black,rh_cancastle);
    if ((abs(e[square_a8])== Rook) && TSTFLAG(spec[square_a8], Black))
      SETCASTLINGFLAGMASK(0,Black,ra_cancastle);
  }

  if (stip_ends_in(si,goal_castling) && !castling_supported)
  {
    VerifieMsg(StipNotSupported);
    return false;
  }

  castling_flag[0] &= castling_flag[castlings_flags_no_castling];
  castling_flag[1] = castling_flag[0];
  castling_flag[2] = castling_flag[0];
  /* At which ply do we begin ??  NG */

  /* a small hack to enable ep keys */
  trait[1] = no_side;

  platzwechsel_rochade_allowed[White][2]=platzwechsel_rochade_allowed[White][1]=true;
  platzwechsel_rochade_allowed[Black][2]=platzwechsel_rochade_allowed[Black][1]=true;

  if (CondFlag[exclusive] && !exclusive_verifie_position(si))
    return false;

  if (CondFlag[isardam]
      || CondFlag[ohneschach])
  {
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
  }

  supergenre = supergenre
      || CondFlag[supercirce]
      || CondFlag[circecage]
      || CondFlag[antisuper]
      || CondFlag[april];

  if (CondFlag[extinction] || CondFlag[circeassassin])
  {
    optim_neutralretractable = false; /* TODO why for extinction */
    add_ortho_mating_moves_generation_obstacle();
  }

  if (CondFlag[actrevolving] || CondFlag[arc])
  {
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
  }

  if (CondFlag[circeturncoats] || CondFlag[circedoubleagents])
  {
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
  }

  if (CondFlag[kobulkings])
  {
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
  }

  if (CondFlag[chamcirce])
    chameleon_circe_init_implicit();

  prev_king_square[White][1] = king_square[White];
  prev_king_square[Black][1] = king_square[Black];

  if (CondFlag[SAT] || CondFlag[strictSAT])
  {
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
    SATCheck = false;
    StrictSAT[White][1] = echecc(White);
    StrictSAT[Black][1] = echecc(Black);
    SATCheck = true;
    satXY = SATFlights[White] > 1 || SATFlights[Black] > 1;
  }

  if (CondFlag[schwarzschacher])
  {
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
  }

  if (flagmummer[White] /* counting opponents moves not useful */
      || TSTFLAG(PieSpExFlags, Neutral)
      || CondFlag[exclusive]
      || CondFlag[isardam]
      || CondFlag[ohneschach]
      || anymars
      || anyantimars
      || CondFlag[brunner]
      || CondFlag[blsupertrans_king]
      || CondFlag[whsupertrans_king]
      || CondFlag[takemake]
      || CondFlag[circecage]
      || CondFlag[SAT]
      || CondFlag[strictSAT]
      || CondFlag[schwarzschacher]
      || CondFlag[republican]
      || CondFlag[kobulkings]
      || exist[UbiUbi] /* sorting by nr of opponents moves doesn't work - why?? */
      || exist[Hunter0] /* ditto */
      || (CondFlag[singlebox] && SingleBoxType==singlebox_type3)) /* ditto */
    disable_countnropponentmoves_defense_move_optimisation(White);

  if (flagmummer[Black] /* counting opponents moves not useful */
      || TSTFLAG(PieSpExFlags, Neutral)
      || CondFlag[exclusive]
      || CondFlag[isardam]
      || CondFlag[ohneschach]
      || anymars
      || anyantimars
      || CondFlag[brunner]
      || CondFlag[blsupertrans_king]
      || CondFlag[whsupertrans_king]
      || CondFlag[takemake]
      || CondFlag[circecage]
      || CondFlag[SAT]
      || CondFlag[strictSAT]
      || CondFlag[schwarzschacher]
      || CondFlag[republican]
      || CondFlag[kobulkings]
      || exist[UbiUbi] /* sorting by nr of opponents moves doesn't work  - why?? */
      || exist[Hunter0] /* ditto */
      || (CondFlag[singlebox] && SingleBoxType==singlebox_type3)) /* ditto */
    disable_countnropponentmoves_defense_move_optimisation(Black);

  if (CondFlag[takemake])
  {
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
  }

  if (CondFlag[protean])
  {
    flagfee = true;
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
  }

  if (CondFlag[castlingchess] || CondFlag[platzwechselrochade])
  {
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
    castling_supported = false;
  }

  if (flagmummer[Black]
      || CondFlag[messigny]
      || (CondFlag[singlebox] && SingleBoxType==singlebox_type3)
      || CondFlag[whsupertrans_king]
      || CondFlag[blsupertrans_king]
      || CondFlag[takemake]
      || CondFlag[exclusive]
      || CondFlag[isardam]
      || CondFlag[ohneschach]
      || TSTFLAG(PieSpExFlags,ColourChange) /* killer machinery doesn't store hurdle */)
    disable_killer_move_optimisation(Black);
  if (flagmummer[White]
      || CondFlag[messigny]
      || (CondFlag[singlebox] && SingleBoxType==singlebox_type3)
      || CondFlag[whsupertrans_king]
      || CondFlag[blsupertrans_king]
      || CondFlag[takemake]
      || CondFlag[exclusive]
      || CondFlag[isardam]
      || CondFlag[ohneschach]
      || TSTFLAG(PieSpExFlags,ColourChange) /* killer machinery doesn't store hurdle */)
    disable_killer_move_optimisation(White);

  if (flagmummer[Black])
    disable_orthodox_mating_move_optimisation(Black);
  if (flagmummer[White])
    disable_orthodox_mating_move_optimisation(White);

  return true;
}

boolean WriteSpec(Flags sp, piece p, boolean printcolours)
{
  boolean ret = false;
  PieSpec spname;

  if (printcolours && !TSTFLAG(sp, Neutral))
  {
    if (areColorsSwapped)
    {
      if (TSTFLAG(sp,White))
        StdChar(tolower(*PieSpString[UserLanguage][Black]));
      if (TSTFLAG(sp,Black))
        StdChar(tolower(*PieSpString[UserLanguage][White]));
    }
    else
    {
      if (TSTFLAG(sp,White))
        StdChar(tolower(*PieSpString[UserLanguage][White]));
      if (TSTFLAG(sp,Black))
        StdChar(tolower(*PieSpString[UserLanguage][Black]));
    }
  }

  for (spname = Neutral; spname < PieSpCount; spname++) {
    if ((spname != Volage || !CondFlag[volage])
        && (spname!=Royal || abs(p)!=King)
        && TSTFLAG(sp, spname))
    {
      StdChar(tolower(*PieSpString[UserLanguage][spname]));
      ret = true;
    }
  }
  return ret;
}

static void swapcolors(void)
{
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp; bnp++)
    if (!TSTFLAG(spec[*bnp],Neutral) && e[*bnp]!=vide)
    {
      e[*bnp] = -e[*bnp];
      spec[*bnp]^= BIT(White)+BIT(Black);
    }

  ProofStartSwapColors();

  areColorsSwapped = !areColorsSwapped;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void reflectboard(void)
{
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp < (square_a1+square_h8)/2; bnp++)
  {
    square const sq_reflected = transformSquare(*bnp,mirra1a8);

    piece const p = e[sq_reflected];
    Flags const sp = spec[sq_reflected];

    e[sq_reflected] = e[*bnp];
    spec[sq_reflected] = spec[*bnp];

    e[*bnp] = p;
    spec[*bnp] = sp;
  }

  ProofStartReflectboard();

  isBoardReflected = !isBoardReflected;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve "half a duplex" (in non-duplex problems, that's the entire
 * problem/twin)
 * @param si identifies the root slice of the stipulation
 */
static void solveHalfADuplex(slice_index si)
{
  inithash(si);
  solve(si,length_unspecified);
  closehash();

#ifdef _SE_DECORATE_SOLUTION_
  se_end_half_duplex();
#endif
}

typedef enum
{
  whitetoplay_means_shorten,
  whitetoplay_means_change_colors
} meaning_of_whitetoplay;

static void remember_color_change(slice_index si, stip_structure_traversal *st)
{
  meaning_of_whitetoplay * const result = st->param;
  *result = whitetoplay_means_change_colors;
}

static meaning_of_whitetoplay detect_meaning_of_whitetoplay(slice_index si)
{
  stip_structure_traversal st;
  meaning_of_whitetoplay result = whitetoplay_means_shorten;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&result);
  stip_structure_traversal_override_single(&st,
                                           STGoalAToBReachedTester,
                                           &remember_color_change);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Apply the option White to play
 * @return true iff the option is applicable (and was applied)
 */
static boolean apply_whitetoplay(slice_index proxy)
{
  slice_index next = slices[proxy].next1;
  boolean result = false;
  meaning_of_whitetoplay meaning;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParamListEnd();

  TraceStipulation(proxy);

  meaning = detect_meaning_of_whitetoplay(next);

  while (slices[next].type==STProxy || slices[next].type==STOutputModeSelector)
    next = slices[next].next1;

  TraceEnumerator(slice_type,slices[next].type,"\n");
  switch (slices[next].type)
  {
    case STHelpAdapter:
    {
      if (meaning==whitetoplay_means_shorten)
      {
        slice_index const prototype = alloc_move_inverter_slice();
        branch_insert_slices(proxy,&prototype,1);
        help_branch_shorten(next);
      }
      else
      {
        stip_detect_starter(proxy);
        stip_impose_starter(proxy,advers(slices[proxy].starter));
      }
      result = true;
      break;
    }

    case STMoveInverter:
    {
      /* starting side is already inverted - just allow color change
       * by removing the inverter
       */
      if (meaning==whitetoplay_means_change_colors)
      {
        pipe_remove(next);
        result = true;
      }
      break;
    }

    default:
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

int parseCommandlineOptions(int argc, char *argv[])
{
  int idx = 1;

  while (idx<argc)
  {
    if (idx+1<argc && strcmp(argv[idx], "-maxpos")==0)
    {
      char *end;
      idx++;
      MaxPositions = strtoul(argv[idx], &end, 10);
      if (argv[idx]==end)
      {
        /* conversion failure
         * -> set to 0 now and to default value later */
        MaxPositions = 0;
      }
      idx++;
      continue;
    }
    else if (idx+1<argc && strcmp(argv[idx], "-maxtime")==0)
    {
      char *end;
      maxtime_type value;
      idx++;
      value = strtoul(argv[idx], &end, 10);
      if (argv[idx]==end)
        ; /* conversion failure -> assume no max time */
      else
        setCommandlineMaxtime(value);

      idx++;
      continue;
    }
    else if (idx+1<argc && strcmp(argv[idx],"-maxmem")==0)
    {
      readMaxmem(argv[idx+1]);
      idx += 2;
      continue;
    }
    else if (strcmp(argv[idx], "-regression")==0)
    {
      flag_regression = true;
      idx++;
      continue;
    }
    else if (strcmp(argv[idx], "-maxtrace")==0)
    {
      trace_level max_trace_level;
      char *end;

      idx++;
      if (idx<argc)
      {
        max_trace_level = strtoul(argv[idx], &end, 10);
#if defined(DOTRACE)
        if (*end==0)
          TraceSetMaxLevel(max_trace_level);
        else
        {
          /* conversion failure  - ignore option */
        }
#else
      /* ignore */
#endif
      }

      idx++;
      continue;
    }
    else
      break;
  }

  return idx;
}

/* Perform initialisations for solving a duplex if we are solving in
 * intelligent mode
 * @param si identifies the slice currently being visited
 * @param st points to the structure holding the state of the traversal
 */
static void intelligent_init_duplex(slice_index si,
                                    stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_impose_starter_nested(si,advers(slices[si].starter),st);
  swapcolors();
  reflectboard();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Perform initialisations for solving a duplex
 * @param si identifies the root slice of the stipulation
 */
static void init_duplex(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STIntelligentMateFilter,
                                           &intelligent_init_duplex);
  stip_structure_traversal_override_single(&st,
                                           STIntelligentStalemateFilter,
                                           &intelligent_init_duplex);
  stip_structure_traversal_override_single(&st,
                                           STIntelligentProof,
                                           &intelligent_init_duplex);
  stip_traverse_structure(si,&st);
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Un-initialise from solving a duplex
 * @param si identifies the slice currently being visited
 * @param st points to the structure holding the state of the traversal
 */
static void intelligent_fini_duplex(slice_index si,
                                    stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (isBoardReflected)
  {
    reflectboard();
    swapcolors();
    stip_impose_starter_nested(si,advers(slices[si].starter),st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Un-initialise from solving a duplex
 * @param si identifies the root slice of the stipulation
 */
static void fini_duplex(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STIntelligentMateFilter,
                                           &intelligent_fini_duplex);
  stip_structure_traversal_override_single(&st,
                                           STIntelligentStalemateFilter,
                                           &intelligent_fini_duplex);
  stip_structure_traversal_override_single(&st,
                                           STIntelligentProof,
                                           &intelligent_fini_duplex);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Perform verifications and initialisations before solving a twin
 * (which may be the only twin of the problem)
 * @param si identifies the root slice of the stipulation
 */
static boolean initialise_verify_twin(slice_index si)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  initPieces();

  if (stip_ends_in(si,goal_proofgame) || stip_ends_in(si,goal_atob))
  {
    Goal const unique_goal = find_unique_goal(si);
    if (unique_goal.type==no_goal)
      VerifieMsg(MultipleGoalsWithProogGameNotAcceptable);
    else
    {
      assert(unique_goal.type==goal_proofgame || unique_goal.type==goal_atob);

      countPieces();
      if (locateRoyal())
      {
        ProofSaveTargetPosition();

        if (stip_ends_in(si,goal_proofgame))
          ProofInitialiseStartPosition();

        ProofRestoreStartPosition();

        countPieces();
        if (locateRoyal() && verify_position(si))
        {
          ProofSaveStartPosition();
          ProofRestoreTargetPosition();

          ProofInitialise(si);

          if (!OptFlag[noboard])
            WritePosition();
          initialise_piece_flags();

          ProofRestoreStartPosition();
          if (unique_goal.type==goal_atob && !OptFlag[noboard])
            ProofWriteStartPosition(si);
          initialise_piece_flags();

          result = true;
        }
      }
    }
  }
  else
  {
    countPieces();
    if (locateRoyal() && verify_position(si))
    {
      if (!OptFlag[noboard])
        WritePosition();
      initialise_piece_flags();

      result = true;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a twin (maybe the only one of a problem)
 * @param si identifies the root slice of the stipulation
 * @param twin_index 0 for first, 1 for second ...; if the problem has
 *                   a zero position, solve_twin() is invoked with
 *                   1, 2, ... but not with 0
 * @param end_of_twin_token token that ended this twin
 */
static void solve_twin(slice_index si,
                       unsigned int twin_index, Token end_of_twin_token)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",twin_index);
  TraceFunctionParamListEnd();

  if (twin_index==0)
  {
    if (LaTeXout)
      LaTeXBeginDiagram();

    if (end_of_twin_token==TwinProblem)
    {
      Message(NewLine);

      if (LaTeXout)
      {
        LaTeXout = false;
        WriteTwinNumber();
        LaTeXout = true;
      }
      else
        WriteTwinNumber();

      Message(NewLine);
    }
  }

  if (!OptFlag[halfduplex])
    solveHalfADuplex(si);

  if (OptFlag[halfduplex] || OptFlag[duplex])
  {
    /* Set next side to calculate for duplex "twin" */
    stip_impose_starter(si,advers(slices[si].starter));
    temporary_hacks_swap_colors();
    TraceStipulation(si);

    init_duplex(si);

    if (locateRoyal() && verify_position(si))
      solveHalfADuplex(si);

    fini_duplex(si);

    temporary_hacks_swap_colors();
    stip_impose_starter(si,advers(slices[si].starter));
  }

  Message(NewLine);

  WRITE_COUNTER(empile);
  WRITE_COUNTER(play_move);
  WRITE_COUNTER(orig_rbechec);
  WRITE_COUNTER(orig_rnechec);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_index build_solvers(slice_index stipulation_root_hook)
{
  slice_index result = stip_deep_copy(stipulation_root_hook);
  slice_index const stipulation_root = slices[stipulation_root_hook].next1;
  Side const starter = slices[stipulation_root].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",stipulation_root_hook);
  TraceFunctionParamListEnd();

  stip_impose_starter(result,starter);

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
  if (TSTFLAG(PieSpExFlags,Kamikaze))
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

  /* must come here because in conditions like Ohneschach, we are going
   * to tampter with the slices inserted here
   */
  stip_insert_selfcheck_guards(result);
  stip_insert_move_generators(result);

  if (OptFlag[keepmating])
    stip_insert_keepmating_filters(result);

  if (CondFlag[amu])
    stip_insert_amu_mate_filters(result);

  if (CondFlag[whiteultraschachzwang]
      || CondFlag[blackultraschachzwang])
    stip_insert_ultraschachzwang_goal_filters(result);

  if (CondFlag[ohneschach])
    ohneschach_replace_immobility_testers(result);
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
    stip_insert_extinction_chess(result);

  if (CondFlag[singlebox])
    switch (SingleBoxType)
    {
      case singlebox_type1:
        stip_insert_singlebox_type1(result);
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
    stip_insert_exclusive_chess_legality_testers(result);

  if (CondFlag[ohneschach])
    stip_insert_ohneschach_legality_testers(result);

  stip_insert_king_capture_avoiders(result);

  if (CondFlag[isardam])
    stip_insert_isardam_legality_testers(result);

  if (CondFlag[patience])
    stip_insert_patience_chess(result);

  if (TSTFLAG(PieSpExFlags,Paralyse))
    stip_insert_paralysing_goal_filters(result);

  if (TSTFLAG(PieSpExFlags,Neutral))
    stip_insert_neutral_initialisers(result);

  if (CondFlag[SAT] || CondFlag[strictSAT])
    stip_substitute_sat_king_flight_generators(result);

  if (CondFlag[strictSAT])
    stip_insert_strict_sat(result);

  if (CondFlag[schwarzschacher])
    stip_insert_blackchecks(result);

  if (CondFlag[masand])
    stip_insert_masand(result);

  if (CondFlag[dynasty])
    stip_insert_dynasty(result);

  if (TSTFLAG(PieSpExFlags,ColourChange))
    stip_insert_hurdle_colour_changers(result);

  stip_insert_king_oscillators(result);

  if (CondFlag[messigny])
    stip_insert_messigny(result);

  if (CondFlag[arc])
    stip_insert_actuated_revolving_centre(result);

  if (CondFlag[actrevolving])
    stip_insert_actuated_revolving_board(result);

  if (anyparrain)
    stip_insert_circe_parrain_rebirth_handlers(result);
  else if (CondFlag[supercirce])
    stip_insert_supercirce_rebirth_handlers(result);
  else if (CondFlag[april])
    stip_insert_april_chess(result);
  else if (CondFlag[circecage])
    stip_insert_circe_cage(result);
  else if (CondFlag[circeassassin])
    stip_insert_circe_assassin(result);
  else if (anycirce)
  {
    stip_insert_circe(result);
    if (CondFlag[circedoubleagents])
      stip_insert_circe_double_agents(result);
    if (CondFlag[chamcirce])
      stip_insert_chameleon_circe(result);
    if (anyclone)
      stip_insert_circe_clone(result);
    if (TSTFLAG(PieSpExFlags,Kamikaze))
    {
      stip_insert_anticirce_relaxed(result);
      stip_insert_circe_kamikaze_rebirth_handlers(result);
      if (CondFlag[couscous])
        stip_insert_anticirce_couscous(result);
    }
    if (CondFlag[couscous])
      stip_insert_couscous_circe(result);
  }

  if (CondFlag[contactgrid])
    stip_insert_contact_grid(result);

  if (anycirce && !rex_circe)
    stip_insert_circe_king_rebirth_avoiders(result);

  if (anycirce)
  {
    if  (TSTFLAG(PieSpExFlags,Volage) || CondFlag[volage])
      stip_insert_circe_volage_recolorers(result);
    if  (anycirprom)
      stip_insert_circe_promoters(result);
    if (CondFlag[circeturncoats])
      stip_insert_circe_turncoats_side_changers(result);
  }

  if (CondFlag[sentinelles])
    stip_insert_sentinelles_inserters(result);

  if (TSTFLAG(PieSpExFlags,Magic))
    stip_insert_magic_pieces_recolorers(result);

  if (CondFlag[antisuper])
  {
    stip_insert_anticirce_strict(result);
    if (AntiCirCheylan)
      stip_insert_anticirce_cheylan(result);
    stip_insert_antisupercirce(result);
    stip_insert_anticirce_promotion(result);
  }
  else if (anyanticirce)
  {
    if (AntiCirCheylan)
      stip_insert_anticirce_cheylan(result);
    stip_insert_anticirce_strict(result);
    if (CondFlag[magicsquare] && magic_square_type==magic_square_type2)
      stip_insert_magic_square_type2(result);
    stip_insert_anticirce(result);
    if (anyanticirprom)
      stip_insert_anticirce_promotion(result);
    if (CondFlag[couscous])
      stip_insert_anticirce_couscous(result);
  }

  if (CondFlag[duellist])
    stip_insert_duellists(result);

  if (CondFlag[hauntedchess])
    stip_insert_haunted_chess(result);

  if (CondFlag[ghostchess])
    stip_insert_ghost_chess(result);

  if (kobulking[White] || kobulking[Black])
    stip_insert_kobul_king_substitutors(result);

  if (TSTFLAG(PieSpExFlags,HalfNeutral))
    stip_insert_half_neutral_recolorers(result);

  if (CondFlag[andernach])
    stip_insert_andernach(result);

  if (CondFlag[antiandernach])
    stip_insert_antiandernach(result);

  if (CondFlag[champursue])
    stip_insert_chameleon_pursuit(result);

  if (CondFlag[norsk])
    stip_insert_norsk_chess(result);

  if (CondFlag[protean] || TSTFLAG(PieSpExFlags,Protean))
    stip_insert_protean_chess(result);

  if (castling_supported)
    stip_insert_castling(result);

  if (CondFlag[einstein])
    stip_insert_einstein_moving_adjusters(result);

  if (CondFlag[reveinstein])
    stip_insert_reverse_einstein_moving_adjusters(result);

  if (CondFlag[antieinstein])
    stip_insert_anti_einstein_moving_adjusters(result);

  if (CondFlag[traitor])
    stip_insert_traitor_side_changers(result);

  if (CondFlag[volage] || TSTFLAG(PieSpExFlags,Volage))
    stip_insert_volage_side_changers(result);

  if (CondFlag[magicsquare])
    stip_insert_magic_square(result);

  if (CondFlag[dbltibet])
    stip_insert_double_tibet(result);
  else if (CondFlag[tibet])
    stip_insert_tibet(result);

  if (CondFlag[degradierung])
    stip_insert_degradierung(result);

  if (TSTFLAG(PieSpExFlags,Chameleon))
    stip_insert_chameleon(result);

  if (CondFlag[frischauf])
    stip_insert_frischauf_promotee_markers(result);

  if (CondFlag[phantom])
    stip_insert_phantom_en_passant_adjusters(result);
  else if (anyantimars)
    stip_insert_antimars_en_passant_adjusters(result);
  else
    stip_insert_en_passant_adjusters(result);

  if (CondFlag[linechamchess])
    stip_insert_line_chameleon_chess(result);

  stip_insert_moving_pawn_promoters(result);

  if (CondFlag[haanerchess])
    stip_insert_haan_chess(result);

  if (CondFlag[castlingchess])
    stip_insert_castling_chess(result);

  if (CondFlag[blsupertrans_king] || CondFlag[whsupertrans_king])
    stip_insert_supertransmuting_kings(result);

  if (CondFlag[amu])
    stip_insert_amu_attack_counter(result);

  if (OptFlag[mutuallyexclusivecastling])
    stip_insert_mutual_castling_rights_adjusters(result);

  if (CondFlag[imitators])
    stip_insert_imitator(result);

  if (CondFlag[football])
    stip_insert_football_chess(result);

  if (CondFlag[platzwechselrochade])
    stip_insert_exchange_castling(result);

  stip_insert_post_move_iteration(result);

  if (dealWithMaxtime())
    stip_insert_maxtime_guards(result);

  if (CondFlag[BGL])
    stip_insert_bgl_filters(result);

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

  if (!OptFlag[solvariantes])
    stip_insert_play_suppressors(result);

  if (OptFlag[solvariantes] && !OptFlag[nothreat])
    stip_insert_threat_boundaries(result);

  stip_spin_off_testers(result);

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

  stip_insert_output_slices(result);

  stip_optimise_with_killer_moves(result);

  if (OptFlag[solmenaces]
      && !stip_insert_maxthreatlength_guards(result))
    Message(ThreatOptionAndExactStipulationIncompatible);

  if (CondFlag[republican])
    stip_insert_republican_king_placers(result);

  and_enable_shortcut_logic(result);

  stip_insert_avoid_unsolvable_forks(result);

  stip_insert_move_iterators(result);

#if defined(DOTRACE)
  stip_insert_move_tracers(result);
#endif

#if defined(DOMEASURE)
  stip_insert_move_counters(result);
#endif

  stip_impose_starter(result,slices[result].starter);

  resolve_proxies(&result);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Iterate over the twins of a problem
 * @prev_token token that ended the previous twin
 * @return token that ended the current twin
 */
static Token iterate_twins(Token prev_token)
{
  unsigned int twin_index = 0;
  slice_index stipulation_root_hook;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",prev_token);
  TraceFunctionParamListEnd();

  stipulation_root_hook = alloc_proxy_slice();

  do
  {
    InitAlways();

    prev_token = ReadTwin(prev_token,stipulation_root_hook);

    if (twin_index==0)
      /* Set the timer for real calculation time */
      StartTimer();

    if (prev_token==ZeroPosition)
    {
      if (!OptFlag[noboard])
        WritePosition();

      prev_token = ReadTwin(prev_token,stipulation_root_hook);
      if (LaTeXout)
        LaTeXBeginDiagram();

      ++twin_index;
    }

    if (slices[stipulation_root_hook].starter==no_side)
    {
      if (OptFlag[quodlibet] && OptFlag[goal_is_end])
        VerifieMsg(GoalIsEndAndQuodlibetIncompatible);
      else if (OptFlag[quodlibet])
      {
        if (!transform_to_quodlibet(stipulation_root_hook))
          Message(QuodlibetNotApplicable);
      }
      else if (OptFlag[goal_is_end])
      {
        if (!stip_insert_goal_is_end_testers(stipulation_root_hook))
          Message(GoalIsEndNotApplicable);
      }

      if (OptFlag[whitetoplay] && !apply_whitetoplay(stipulation_root_hook))
        Message(WhiteToPlayNotApplicable);

      stip_insert_goal_prerequisite_guards(stipulation_root_hook);

      if (OptFlag[postkeyplay] && !battle_branch_apply_postkeyplay(stipulation_root_hook))
        Message(PostKeyPlayNotApplicable);

      stip_detect_starter(stipulation_root_hook);
    }

    TraceStipulation(stipulation_root_hook);

    if (slices[slices[stipulation_root_hook].next1].starter==no_side)
      VerifieMsg(CantDecideWhoIsAtTheMove);
    else
    {
      stip_impose_starter(stipulation_root_hook,
                          slices[stipulation_root_hook].starter);

      if (initialise_verify_twin(slices[stipulation_root_hook].next1))
      {
        slice_index const root_slice = build_solvers(stipulation_root_hook);
        TraceStipulation(root_slice);
        solve_twin(root_slice,twin_index,prev_token);
        dealloc_slices(root_slice);
      }
    }

    ++twin_index;
  } while (prev_token==TwinProblem);

  dealloc_slices(stipulation_root_hook);

  assert_no_leaked_slices();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",prev_token);
  TraceFunctionResultEnd();
  return prev_token;
}

/* Iterate over the problems read from standard input or the input
 * file indicated in the command line options
 */
void iterate_problems(void)
{
  Token prev_token = BeginProblem;

  do
  {
    InitBoard();
    InitCond();
    InitOpt();

    reset_max_solutions();
    reset_was_max_nr_solutions_per_target_position_reached();
    reset_short_solution_found_in_problem();

    prev_token = iterate_twins(prev_token);

    if (max_solutions_reached()
        || was_max_nr_solutions_per_target_position_reached()
        || has_short_solution_been_found_in_problem()
        || hasMaxtimeElapsed())
      StdString(GetMsgString(InterMessage));
    else
      StdString(GetMsgString(FinishProblem));

    StdString(" ");
    PrintTime();
    StdString("\n\n\n");

    if (LaTeXout)
      LaTeXEndDiagram();
  } while (prev_token==NextProblem);
}

#if defined(NOMEMSET)
void memset(char *poi, char val, int len)
{
  while (len--)
    *poi++ = val;
}
#endif
