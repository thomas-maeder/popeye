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
 ** 2006/07/30 SE   Extended movenum option to also print time when solving h#/sh# at next ply
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
 ** 2009/04/25 SE   New condition: Provacateurs
 **                 New piece type: Patrol pieces
 **
 ***************************** End of List ******************************/

#if defined(macintosh)    /* is always defined on macintosh's  SB */
#   define SEGM2
#   include "pymac.h"
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
#include "pyhash.h"
#include "pyint.h"
#include "DHT/dhtbcmem.h"
#include "pyproof.h"
#include "pymovein.h"
#include "pykeepmt.h"
#include "pyselfcg.h"
#include "pymovenb.h"
#include "pyflight.h"
#include "pydegent.h"
#include "pythreat.h"
#include "pynontrv.h"
#include "pypipe.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_moves_generation.h"
#include "stipulation/proxy.h"
#include "trace.h"
#include "measure.h"
#include "pyslice.h"
#include "pyoutput.h"
#include "conditions/exclusive.h"
#include "conditions/republican.h"
#include "platform/maxmem.h"
#include "platform/maxtime.h"
#include "platform/pytime.h"
#include "platform/priority.h"
#include "stipulation/branch.h"
#include "stipulation/dead_end.h"
#include "stipulation/boolean/or.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/continuation.h"
#include "stipulation/battle_play/try.h"
#include "stipulation/battle_play/postkeyplay.h"
#include "stipulation/battle_play/check_detector.h"
#include "stipulation/battle_play/threat.h"
#include "stipulation/help_play/find_by_increasing_length.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/goals/prerequisite_guards.h"
#include "pieces/attributes/paralysing/paralysing.h"
#include "pieces/attributes/kamikaze/kamikaze.h"
#include "conditions/amu/mate_filter.h"
#include "conditions/circe/circe.h"
#include "conditions/anticirce/anticirce.h"
#include "conditions/ultraschachzwang/goal_filter.h"
#include "options/no_short_variations/no_short_variations.h"
#include "optimisations/goals/optimisation_guards.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_move_generator.h"
#include "optimisations/killer_move/killer_move.h"
#include "optimisations/killer_move/final_defense_move.h"
#include "optimisations/end_of_branch_goal_immobile.h"
#include "options/maxtime.h"
#include "options/maxsolutions/maxsolutions.h"
#include "options/stoponshortsolutions/stoponshortsolutions.h"
#include "optimisations/count_nr_opponent_moves/move_generator.h"
#ifdef _SE_
#include "se.h"
#endif

boolean is_rider(piece p)
{
  switch (p)
  {
    case    nb:
    case    amazb:
    case    impb:
    case    princb:
    case    waranb:
    case    camridb:
    case    zebridb:
    case    gnuridb:
    case    rhuntb:
    case    bhuntb:
    case    tb:
    case    db:
    case    fb:
    case    vizridb:
    case    fersridb:
      return true;
    default:  return false;
  }
}

boolean is_leaper(piece p)
{
  switch (p)
  {
    case    zb:
    case    chb:
    case    gib:
    case    rccinqb:
    case    bub:
    case    vizirb:
    case    alfilb:
    case    fersb:
    case    dabb:
    case    gnoub:
    case    antilb:
    case    ecurb:
    case    ekingb:
    case    okapib:
    case    cb:
    case    leap15b:
    case    leap16b:
    case    leap24b:
    case    leap25b:
    case    leap35b:
    case    leap37b:
    case    leap36b:
      return true;
    default:  return false;
  }
} /* is_leaper */

boolean is_simplehopper(piece p)
{
  switch (p)
  {
    case sb:
    case nsautb:
    case camhopb:
    case zebhopb:
    case gnuhopb:
    case equib:
    case nequib:
    case g2b:
    case g3b:
    case khb:
    case orixb:

    case mooseb:
    case rookmooseb:
    case bishopmooseb:
    case eagleb:
    case rookeagleb:
    case bishopeagleb:
    case sparrb:
    case rooksparrb:
    case bishopsparrb:

      return true;
    default:  return false;
  }
}

static boolean is_simpledecomposedleaper(piece p)
{
  switch (p)
  {
    case maob:
    case moab:
      return true;
    default:
      return false;
  }
}

static boolean is_symmetricfairy(piece p)
{
  /* any piece where, if p captures X is legal, then it's also legal if p and X are swapped */
  switch (p)
  {
    case leob:
    case vaob:
    case paob:
    case lionb:
    case tlionb:
    case flionb:
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
  piece p;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (p = roib; p<=fb; p++)
    exist[p] = true;

  for (p = fb+1; p<=derbla; p++)
    exist[p] = false;

  if (CondFlag[sentinelles])
  {
    exist[sentineln] = true;
    exist[sentinelb] = true;
  }

  if (CondFlag[leofamily])
    for (p = db; p<=fb; p++)
      exist[p] = false;

  if (CondFlag[chinoises])
    for (p = leob; p <= vaob; p++)
      exist[p] = true;

  if (CondFlag[cavaliermajeur])
  {
    exist[cb] = false;
    exist[nb] = true;
  }

  for (p = (CondFlag[losingchess] || CondFlag[dynasty] || CondFlag[extinction]
            ? roib
            : db);
       p <= derbla;
       p++)
    if (promonly[p])
      exist[p] = true;

  if (CondFlag[protean])
    exist[reversepb] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void countPieces(void)
{
  square const *bnp;
  piece p;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (p = roib; p<=derbla; p++)
  {
    nbpiece[p] = 0;
    nbpiece[-p] = 0;
  }

  for (bnp = boardnum; *bnp; bnp++)
  {
    p = e[*bnp];
    if (p!=vide)
    {
      if (p<fn)
        exist[-p] = true;
      else if (p>fb)
        exist[p] = true;

      ++nbpiece[p];
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean locateRoyal(void)
{
  rb = initsquare;
  rn = initsquare;

  if (TSTFLAG(PieSpExFlags,Neutral))
    /* neutral king has to be white for initialisation of r[bn] */
    initneutre(White);

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
          if (!SetKing(&rb,s))
          {
            VerifieMsg(OneKing);
            return false;
          }
          break;
        }
      }
    else
      rb = initsquare;

    if (nbpiece[roin]==1)
      for (bnp = boardnum; *bnp; bnp++)
      {
        s = *bnp;
        if (e[s]==roin)
        {
          if (!SetKing(&rn,s))
          {
            VerifieMsg(OneKing);
            return false;
          }
          break;
        }
      }
    else
      rn = initsquare;
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
        if (!SetKing(&rb,s))
        {
          VerifieMsg(OneKing);
          return false;
        }
        if (TSTFLAG(spec[s],Neutral))
          SetKing(&rn,s);
      }

      if (s==wh_royal_sq)
      {
        if (!SetKing(&rb,s))
        {
          VerifieMsg(OneKing);
          return false;
        }
      }

      if (p==roin
          || (p<roin && TSTFLAG(spec[s],Royal)))
      {
        if (!SetKing(&rn,s))
        {
          VerifieMsg(OneKing);
          return false;
        }
      }

      if (s==bl_royal_sq)
      {
        if (!SetKing(&rn,s))
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
square DiaRenSquares[MaxPieceId+1] = { initsquare };

static void initialise_piece_flags(void)
{
  square const *bnp;
  PieceIdType id = MinPieceId;
  for (bnp = boardnum; *bnp; bnp++)
  {
    piece const p = e[*bnp];
    if (p!=vide)
    {
      if (CondFlag[volage] && rb!=*bnp && rn!=*bnp)
        SETFLAG(spec[*bnp], Volage);

      assert(id<=MaxPieceId);
      SetPieceId(spec[*bnp], id);
      ++id;
      SetDiaRen(spec[*bnp], *bnp);

      if (TSTFLAG(spec[*bnp],ColourChange)
          && !is_simplehopper(abs(e[*bnp])))
      {
        /* relies on imitators already having been implemented */
        CLRFLAG(spec[*bnp],ColourChange);
        ErrorMsg(ColourChangeRestricted);
      }

      /* known limitation: will print rK rather than just K as usual */
      if (abs(e[*bnp])==King && (CondFlag[protean] || flag_magic))
        SETFLAG(spec[*bnp],Royal);
    }
  }
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
  Side const restricted_side = ((length-slack_length_help)%2==1
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

static structure_traversers_visitors restricted_side_finders[] =
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

  stip_structure_traversal_init(&st,&is_restricted);
  stip_structure_traversal_override(&st,
                                    restricted_side_finders,
                                    nr_restricted_side_finders);
  stip_traverse_structure(si,&st);

  if (is_restricted[White] && !is_restricted[Black])
    return White;
  else if (is_restricted[Black] && !is_restricted[White])
    return Black;
  else
    return no_side;
}

/* Verify the user input and our interpretation of it
 * @param si identifies the root slice of the representation of the
 *           stipulation
 * @return true iff the verification was successful
 */
static boolean verify_position(slice_index si)
{
  square const *bnp;
  piece     p;
  ply           n;
  int      cp, pp, tp, op;

  jouegenre = false;
  jouetestgenre = false;
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

  if (CondFlag[parrain] && stip_ends_in(si,goal_steingewinn))
  {
    VerifieMsg(PercentAndParrain);
    return false;
  }

  if (TSTFLAG(PieSpExFlags, HalfNeutral))
    SETFLAG(PieSpExFlags, Neutral);

  if (CondFlag[republican] && !republican_verifie_position(si))
    return false;

  if ((bl_royal_sq!=initsquare || wh_royal_sq!=initsquare
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

  flag_magic = TSTFLAG(PieSpExFlags, Magic);
  flag_outputmultiplecolourchanges = flag_magic || CondFlag[masand];

#ifdef _SE_DECORATE_SOLUTION_
  se_init();
#endif
  flagleofamilyonly = CondFlag[leofamily] ? true : false;
  for (p = fb + 1; p <= derbla; p++)
  {
    if (exist[p] || promonly[p])
    {
      flagfee = true;
      if (is_rider(p))
        flagriders = true;
      else if (is_leaper(p))
        flagleapers = true;
      else if (is_simplehopper(p))
        flagsimplehoppers = true;
      else if (is_simpledecomposedleaper(p))
        flagsimpledecomposedleapers = true;
      else if (is_symmetricfairy(p))
        flagsymmetricfairy = true;
      else {
        if (!is_pawn(p) && p != dummyb && (p<leob || p>vaob))
          flagleofamilyonly = false;
        flagveryfairy = true;
      }
      if (flag_magic
          && attackfunctions[p]==unsupported_uncalled_attackfunction)
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

  if (!CondFlag[noiprom])
    for (n = 0; n <= maxply; n++)
      Iprom[n] = false;

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
        || CondFlag[parrain]) /* verkraftet nicht 2 IUW in einem Zug !!! */
    {
      VerifieMsg(ImitWFairy);
      return false;
    }
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
  }

  if (CondFlag[leofamily])
  {
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
    if (exist[dummyb])
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
      bl_ultra = CondFlag[ultra];
      bl_exact = CondFlag[exact];
      black_length = len_max;
      flagblackmummer = true;
      flagwhitemummer = false;
    }
    else
    {
      CondFlag[blmax] = false;
      CondFlag[whmax] = true;
      wh_ultra = CondFlag[ultra];
      wh_exact = CondFlag[exact];
      white_length = len_max;
      flagwhitemummer = true;
      flagblackmummer = false;
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

  if (OptFlag[sansrb] && rb!=initsquare)
    OptFlag[sansrb] = false;

  if (OptFlag[sansrn] && rn!=initsquare)
    OptFlag[sansrn] = false;

  if (rb==initsquare && nbpiece[roib]==0
      && !OptFlag[sansrb])
    ErrorMsg(MissingKing);

  if (rn==initsquare && nbpiece[roin]==0
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
      if (((! OptFlag[sansrb]) && rb!=initsquare && (e[rb] != roib))
          || ((! OptFlag[sansrn]) && rn!=initsquare && (e[rn] != roin)))
      {
        VerifieMsg(RoyalPWCRexCirce);
        return false;
      }
    }
  }

  if (CondFlag[bicolores])
  {
    if (TSTFLAG(PieSpExFlags, Neutral))
    {
      VerifieMsg(NeutralAndBicolor);
      return false;
    }
    else
      add_ortho_mating_moves_generation_obstacle();
  }

  if (CondFlag[bichro] || CondFlag[monochro])
    add_ortho_mating_moves_generation_obstacle();

  eval_2 = eval_white = eval_ortho;
  rbechec = &orig_rbechec;
  rnechec = &orig_rnechec;

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
    if (  (CondFlag[koeko]
           || CondFlag[newkoeko]
           || CondFlag[antikoeko]
           || TSTFLAG(PieSpExFlags, Jigger))
          && anycirce
          && TSTFLAG(PieSpExFlags, Neutral))
    {
      VerifieMsg(KoeKoCirceNeutral);
      return false;
    }
  }

  if ((flaglegalsquare || TSTFLAG(PieSpExFlags,Neutral))
      && CondFlag[volage])
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
    eval_white = eval_black = eval_disp;
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

  if (CondFlag[isardam] && flag_madrasi)
  {
    VerifieMsg(IsardamAndMadrasi);
    return false;
  }

  if (CondFlag[black_oscillatingKs] || CondFlag[white_oscillatingKs])
  {
    if (rb==initsquare || rn==initsquare)
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
    if (calc_whtrans_king
        || calc_whrefl_king
        || calc_bltrans_king
        || calc_blrefl_king
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
    BGL_whiteinfinity = BGL_white == BGL_infinity;
    BGL_blackinfinity = BGL_black == BGL_infinity;
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
  }

  if (flag_madrasi || CondFlag[isardam])
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
      eval_white = CondFlag[isardam]
          ?   eval_isardam
          : eval_madrasi;
    }
  }

  if (CondFlag[woozles]) {
    if ( flag_madrasi
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
    eval_white=eval_isardam;

  if (CondFlag[isardam] && IsardamB)
    eval_white=eval_ortho;

  if (CondFlag[shieldedkings])
  {
    eval_white=eval_shielded;
    obsfriendgenre=true;
  }

  if (flagAssassin) {
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
      cirrenroib = (*circerenai)(nbply, roib, spec[rb], initsquare, initsquare, initsquare, Black);
      cirrenroin = (*circerenai)(nbply, roin, spec[rn], initsquare, initsquare, initsquare, White);
    }
    else {
      eval_white = rbimmunech;
      eval_black = rnimmunech;
      immrenroib = (*immunrenai)(nbply, roib, spec[rb], initsquare, initsquare, initsquare, Black);
      immrenroin = (*immunrenai)(nbply, roin, spec[rn], initsquare, initsquare, initsquare, White);
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
    rnechec = &singleboxtype3_rnechec;
    rbechec = &singleboxtype3_rbechec;
    gen_wh_piece = &singleboxtype3_gen_wh_piece;
    gen_bl_piece = &singleboxtype3_gen_bl_piece;
  }

  if ((CondFlag[white_oscillatingKs] || CondFlag[black_oscillatingKs])
      && (OptFlag[sansrb] || OptFlag[sansrn]))
  {
    VerifieMsg(MissingKing);
    return false;
  }

  if (wh_ultra && !CondFlag[whcapt]) {
    eval_2 = eval_white;
    eval_black = rnultraech;
    if (TSTFLAG(PieSpExFlags, Neutral))
    {
      VerifieMsg(OthersNeutral);
      return false;
    }
  }
  if (bl_ultra && !CondFlag[blcapt]) {
    eval_2 = eval_white;
    eval_white = rbultraech;
    if (TSTFLAG(PieSpExFlags, Neutral))
    {
      VerifieMsg(OthersNeutral);
      return false;
    }
  }

  if ( ( CondFlag[whmin]
         + CondFlag[whmax]
         + CondFlag[whcapt]
         + (CondFlag[whforsqu] || CondFlag[whconforsqu])
         + CondFlag[whfollow]
         + CondFlag[duellist]
         + CondFlag[alphabetic]
         + CondFlag[whitesynchron]
         + CondFlag[whiteantisynchron]) > 1
       || (CondFlag[blmin]
           + CondFlag[blmax]
           + CondFlag[blcapt]
           + (CondFlag[blforsqu] || CondFlag[blconforsqu])
           + CondFlag[blfollow]
           + CondFlag[duellist]
           + CondFlag[alphabetic]
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
      && (exist[roseb]
          || exist[csb]
          || exist[ubib]
          || exist[hamstb]
          || exist[mooseb]
          || exist[eagleb]
          || exist[sparrb]
          || exist[archb]
          || exist[reffoub]
          || exist[cardb]
          || exist[bscoutb]
          || exist[gscoutb]
          || exist[dcsb]
          || exist[refcb]
          || exist[refnb]
          || exist[catb]
          || exist[rosehopperb]
          || exist[roselionb]
          || exist[raob]
          || exist[rookmooseb]
          || exist[rookeagleb]
          || exist[rooksparrb]
          || exist[bishopmooseb]
          || exist[bishopeagleb]
          || exist[bishopsparrb]
          || exist[doublegb]))
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

  jouegenre = jouegenre
      || CondFlag[black_oscillatingKs]
      || CondFlag[white_oscillatingKs]
      || anycirce
      || CondFlag[sentinelles]
      || anyanticirce
      || CondFlag[singlebox]
      || CondFlag[blroyalsq]
      || CondFlag[whroyalsq]
      || CondFlag[dynasty] /* TODO why? */
      || CondFlag[strictSAT]
      || CondFlag[masand]
      || CondFlag[BGL]
      || CondFlag[duellist]
      || TSTFLAG(PieSpExFlags,HalfNeutral)
      || exist[Orphan]
      || exist[Friend]
      || calc_whrefl_king || calc_blrefl_king
      || CondFlag[phantom]
      || CondFlag[extinction] /* TODO why? */
      || CondFlag[amu]
      || CondFlag[imitators]
      || CondFlag[blsupertrans_king] || CondFlag[whsupertrans_king]
      || TSTFLAG(PieSpExFlags, Magic)
      || CondFlag[ghostchess]
      || CondFlag[hauntedchess];


  change_moving_piece=
      TSTFLAG(PieSpExFlags, Kamikaze)
      || TSTFLAG(PieSpExFlags, Protean)
      || CondFlag[tibet]
      || CondFlag[andernach]
      || CondFlag[antiandernach]
      || CondFlag[magicsquare]
      || TSTFLAG(PieSpExFlags, Chameleon)
      || CondFlag[einstein]
      || CondFlag[volage]
      || TSTFLAG(PieSpExFlags, Volage)
      || CondFlag[degradierung]
      || CondFlag[norsk]
      || CondFlag[traitor]
      || CondFlag[linechamchess]
      || CondFlag[chamchess]
      || CondFlag[protean]
      || CondFlag[champursue];

  repgenre =
      CondFlag[sentinelles]
      || CondFlag[imitators]
      || anycirce
      || TSTFLAG(PieSpExFlags, Neutral)
      || (CondFlag[singlebox] && SingleBoxType==singlebox_type1)
      || anyanticirce
      || CondFlag[ghostchess]
      || CondFlag[hauntedchess];

  empilegenre=
      flaglegalsquare
      || obsgenre
      || CondFlag[imitators]
      || anyimmun
      || CondFlag[nocapture]
      || CondFlag[nowhcapture]
      || CondFlag[noblcapture]
      || TSTFLAG(spec[rb], Kamikaze)
      || TSTFLAG(spec[rn], Kamikaze)
      || flagwhitemummer
      || flagblackmummer
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

  flag_libre_on_generate = flag_madrasi || CondFlag[disparate];

  if (CondFlag[dynasty])
  {
    /* checking for TSTFLAG(spec[rb],Kamikaze) may not be sufficient
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
      || CondFlag[parrain]
      || flagwhitemummer
      || flagblackmummer
      || CondFlag[vogt]
      || (eval_white != eval_ortho
          && eval_white != legalsquare)
      || (rb != initsquare && abs(e[rb]) != King)
      || (rn != initsquare && abs(e[rn]) != King)
      || TSTFLAG(PieSpExFlags, Chameleon)
      || CondFlag[einstein]
      || CondFlag[degradierung]
      || CondFlag[norsk]
      || CondFlag[messigny]
      || CondFlag[linechamchess]
      || CondFlag[chamchess]
      || CondFlag[antikings]
      || TSTFLAG(PieSpExFlags, HalfNeutral)
      || CondFlag[geneva]
      || CondFlag[dynasty] /* TODO why? */
      || flag_magic)
  {
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
  }

  superbas = CondFlag[antisuper] ? square_a1 : square_a1-1;

  /* init promotioncounter and checkcounter */
  pp = 0;
  cp = 0;
  {
    piece p;
    piece firstprompiece;

    if (CondFlag[losingchess] || CondFlag[dynasty] || CondFlag[extinction])
      firstprompiece = roib;
    else if (CondFlag[singlebox] && SingleBoxType!=singlebox_type1)
      firstprompiece = pb;
    else
      firstprompiece = db;

    for (p = firstprompiece; p<=derbla; ++p)
    {
      getprompiece[p] = vide;

      if (exist[p])
      {
        if ((p!=pb || (CondFlag[singlebox] && SingleBoxType!=singlebox_type1))
            && (p!=roib
                || CondFlag[losingchess]
                || CondFlag[dynasty]
                || CondFlag[extinction])
            && p!=dummyb
            && p!=pbb
            && p!=bspawnb
            && p!=spawnb
            && p!=reversepb
            && (!CondFlag[promotiononly] || promonly[p]))
        {
          getprompiece[pp] = p;
          pp = p;
        }

        if (p>fb && p!=dummyb) {
          /* only fairy pieces until now ! */
          optim_neutralretractable = false;
          add_ortho_mating_moves_generation_obstacle();
          if (p!=hamstb)
          {
            checkpieces[cp] = p;
            cp++;
          }
        }
      }
    }

    checkpieces[cp] = vide;
  }

  tp = 0;
  op = 0;
  {
    PieNam p;
    for (p = King; p<PieceCount; ++p) {
      if (exist[p] && p!=Dummy && p!=Hamster)
      {
        if (whitenormaltranspieces)
          whitetransmpieces[tp] = p;
        if (blacknormaltranspieces)
          blacktransmpieces[tp] = p;
        tp++;
        if (p!=Orphan
            && p!=Friend
            && (exist[Orphan] || exist[Friend]))
          orphanpieces[op++] = p;
      }
    }
  }

  if (whitenormaltranspieces)
    whitetransmpieces[tp] = vide;
  if (blacknormaltranspieces)
    blacktransmpieces[tp] = vide;

  if (calc_whrefl_king || calc_blrefl_king)
  {
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
  }
  orphanpieces[op] = vide;

  if ((calc_whrefl_king
       && rb != initsquare
       && (e[rb] != roib || CondFlag[sting]))
      || (calc_blrefl_king
          && rn != initsquare
          && (e[rn] != roin || CondFlag[sting])))
  {
    VerifieMsg(TransmRoyalPieces);
    return false;
  }

  if ((exist[Orphan]
       || exist[Friend]
       || calc_whrefl_king
       || calc_blrefl_king)
      && TSTFLAG(PieSpExFlags, Neutral))
  {
    VerifieMsg(NeutralAndOrphanReflKing);
    return false;
  }

  if ((eval_white==eval_isardam) && CondFlag[vogt])
  {
    VerifieMsg(VogtlanderandIsardam);
    return false;
  }

  for (n = 2; n <= maxply; n++)
    inum[n] = inum[1];

  if ((CondFlag[chamchess] || CondFlag[linechamchess])
      && TSTFLAG(PieSpExFlags, Chameleon))
  {
    VerifieMsg(ChameleonPiecesAndChess);
    return false;
  }

  if (TSTFLAG(PieSpExFlags, ColourChange))
  {
    checkhopim = true;
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
    jouegenre = true;
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
    rbechec = &annan_rbechec;
    rnechec = &annan_rnechec;
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
    rbechec = &losingchess_rbnechec;
    rnechec = &losingchess_rbnechec;

    /* capturing moves are "longer" than non-capturing moves */
    black_length = &len_losingchess;
    white_length = &len_losingchess;
    flagwhitemummer = true;
    flagblackmummer = true;
  }

  /* check castling possibilities */
  CLEARFL(castling_flag[0]);
  /* castling_supported has to be adjusted if there are any problems */
  /* with castling and fairy conditions/pieces */
  castling_supported = !(
      /* Let's see if transmuting kings can castle without
         problems ... */
      /* Unfortunately they can't ! So I had to exclude them
         again ...  */
      /* A wK moving from anywhere to e1 and then like a queen from
         e1 to g1 would get the castling right when this last move is
         retracted  (:-( */
      /* transmuting kings and castling enabled again
       */
      CondFlag[patience]
      || CondFlag[parrain]
      || CondFlag[haanerchess]);

  complex_castling_through_flag = CondFlag[imitators];

  if (castling_supported) {
    if ((abs(e[square_e1])== King) && TSTFLAG(spec[square_e1], White)
        && (!CondFlag[dynasty] || nbpiece[roib]==1))
      SETFLAGMASK(castling_flag[0],ke1_cancastle);
    if ((abs(e[square_h1])== Rook) && TSTFLAG(spec[square_h1], White))
      SETFLAGMASK(castling_flag[0],rh1_cancastle);
    if ((abs(e[square_a1])== Rook) && TSTFLAG(spec[square_a1], White))
      SETFLAGMASK(castling_flag[0],ra1_cancastle);
    if ((abs(e[square_e8])== King) && TSTFLAG(spec[square_e8], Black)
        && (!CondFlag[dynasty] || nbpiece[roin]==1))
      SETFLAGMASK(castling_flag[0],ke8_cancastle);
    if ((abs(e[square_h8])== Rook) && TSTFLAG(spec[square_h8], Black))
      SETFLAGMASK(castling_flag[0],rh8_cancastle);
    if ((abs(e[square_a8])== Rook) && TSTFLAG(spec[square_a8], Black))
      SETFLAGMASK(castling_flag[0],ra8_cancastle);
  }

  if (stip_ends_in(si,goal_castling) && !castling_supported)
  {
    VerifieMsg(StipNotSupported);
    return false;
  }

  castling_flag[0] &= no_castling;
  castling_flag[1] = castling_flag[0];
  castling_flag[2] = castling_flag[0];
  /* At which ply do we begin ??  NG */

  testcastling=
      TSTFLAGMASK(castling_flag[0],whq_castling&no_castling)==whq_castling
      || TSTFLAGMASK(castling_flag[0],whk_castling&no_castling)==whk_castling
      || TSTFLAGMASK(castling_flag[0],blq_castling&no_castling)==blq_castling
      || TSTFLAGMASK(castling_flag[0],blk_castling&no_castling)==blk_castling;

  /* a small hack to enable ep keys */
  trait[1] = no_side;

  if (CondFlag[exclusive] && !exclusive_verifie_position(si))
    return false;

  if (CondFlag[isardam]
      || CondFlag[ohneschach])
  {
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
  }

  if (!CondFlag[patience]) {           /* needed because of twinning */
    PatienceB = false;
  }

  jouetestgenre = jouetestgenre
      || CondFlag[exclusive]
      || CondFlag[isardam]
      || CondFlag[ohneschach]
      || flagAssassin
      || stip_ends_in(si,goal_doublemate)
      || stip_ends_in(si,goal_countermate)
      || CondFlag[patience]
      || CondFlag[blackultraschachzwang]
      || CondFlag[whiteultraschachzwang]
      || CondFlag[BGL];
  jouetestgenre_save = jouetestgenre;

  jouetestgenre1 = CondFlag[blackultraschachzwang]
      || CondFlag[whiteultraschachzwang];

  supergenre = supergenre
      || CondFlag[supercirce]
      || CondFlag[circecage]
      || CondFlag[antisuper]
      || CondFlag[april];

  if (CondFlag[extinction] || flagAssassin)
  {
    optim_neutralretractable = false; /* TODO why for extinction */
    add_ortho_mating_moves_generation_obstacle();
  }

  if (CondFlag[actrevolving] || CondFlag[arc])
  {
    jouegenre = true;
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
  }

  if (anytraitor) {
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
  }

  if (InitChamCirce)
  {
    if (CondFlag[leofamily]) {
      NextChamCircePiece[Leo] = Mao;
      NextChamCircePiece[Pao] = Leo;
      NextChamCircePiece[Vao] = Pao;
      NextChamCircePiece[Mao] = Vao;
    }
    else {
      piece actknight = CondFlag[cavaliermajeur] ? NightRider : Knight;
      NextChamCircePiece[actknight] = Bishop;
      NextChamCircePiece[Bishop] = Rook;
      NextChamCircePiece[Rook] = Queen;
      NextChamCircePiece[Queen] = actknight;
    }
  }

  RB_[1] = rb;
  RN_[1] = rn;

  if (CondFlag[SAT] || CondFlag[strictSAT])
  {
    SATCheck = true;
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
    WhiteStrictSAT[1] = echecc_normal(nbply,White);
    BlackStrictSAT[1] = echecc_normal(nbply,Black);
    satXY = WhiteSATFlights > 1 || BlackSATFlights > 1;
  }

  if (CondFlag[BGL])
  {
    BGL_white_store[1] = BGL_white;
    BGL_black_store[1] = BGL_black;
  }

  if (CondFlag[schwarzschacher])
  {
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
  }

  if (flagwhitemummer /* counting opponents moves not useful */
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
      || exist[ubib] /* sorting by nr of opponents moves doesn't work - why?? */
      || exist[hunter0b] /* ditto */
      || (CondFlag[singlebox] && SingleBoxType==singlebox_type3)) /* ditto */
    disable_countnropponentmoves_defense_move_optimisation(White);

  if (flagblackmummer /* counting opponents moves not useful */
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
      || exist[ubib] /* sorting by nr of opponents moves doesn't work  - why?? */
      || exist[hunter0b] /* ditto */
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

  if (CondFlag[castlingchess])
  {
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
    castling_supported = false;
    jouegenre = true;
  }

  if (flagblackmummer
      || CondFlag[messigny]
      || (CondFlag[singlebox] && SingleBoxType==singlebox_type3)
      || CondFlag[whsupertrans_king]
      || CondFlag[blsupertrans_king]
      || CondFlag[takemake]
      || CondFlag[exclusive]
      || CondFlag[isardam]
      || CondFlag[ohneschach])
    disable_killer_move_optimisation(Black);
  if (flagwhitemummer
      || CondFlag[messigny]
      || (CondFlag[singlebox] && SingleBoxType==singlebox_type3)
      || CondFlag[whsupertrans_king]
      || CondFlag[blsupertrans_king]
      || CondFlag[takemake]
      || CondFlag[exclusive]
      || CondFlag[isardam]
      || CondFlag[ohneschach])
    disable_killer_move_optimisation(White);

  if (flagblackmummer)
    disable_orthodox_mating_move_optimisation(Black);
  if (flagwhitemummer)
    disable_orthodox_mating_move_optimisation(White);

  return true;
}

boolean moves_equal(coup const *move1, coup const *move2)
{
  return (move1->cdzz==move2->cdzz
          && move1->cazz==move2->cazz
          && move1->norm_prom==move2->norm_prom
          && move1->cir_prom==move2->cir_prom
          && move1->bool_cir_cham_prom==move2->bool_cir_cham_prom
          && move1->bool_norm_cham_prom==move2->bool_norm_cham_prom
          && move1->sb3where==move2->sb3where
          && move1->sb3what==move2->sb3what
          && move1->sb2where==move2->sb2where
          && move1->sb2what==move2->sb2what
          && move1->hurdle==move2->hurdle
          && (!CondFlag[takemake] || move1->cpzz==move2->cpzz)
          && (!supergenre
              || ((!(CondFlag[supercirce]
                     || CondFlag[april]
                     || CondFlag[circecage])
                   || move1->sqren==move2->sqren)
                  && (!CondFlag[republican]
                      || republican_moves_equal(move1,move2))
                  && (!CondFlag[antisuper]
                      || move1->renkam==move2->renkam))
              )
          );
}

void current(ply ply_id, coup *mov)
{
  numecoup const coup_id = ply_id==nbply ? nbcou : repere[ply_id+1];
  square const sq_arrival = move_generation_stack[coup_id].arrival;

  mov->tr =          trait[ply_id];
  mov->cdzz =           move_generation_stack[coup_id].departure;
  mov->cazz =            sq_arrival;
  mov->cpzz =            move_generation_stack[coup_id].capture;
  mov->pjzz =            pjoue[ply_id];
  mov->norm_prom =       norm_prom[ply_id];
  mov->ppri =            pprise[ply_id];
  mov->sqren =           sqrenais[ply_id];
  mov->cir_prom =        cir_prom[ply_id];

  mov->renkam = sq_rebirth_capturing[ply_id];
  mov->promi =  Iprom[ply_id];
  mov->numi =     inum[ply_id] - (mov->promi ? 1 : 0);
  /* Promoted imitator will be output 'normally'
     from the next move on. */
  mov->sum = isquare[0] - im0;
  mov->speci = jouespec[ply_id];

  /* hope the following works with parrain too */
  mov->ren_spec =  spec[sqrenais[ply_id]];
  mov->bool_senti = senti[ply_id];
  mov->ren_parrain = ren_parrain[ply_id];
  mov->bool_norm_cham_prom = norm_cham_prom[ply_id];
  mov->bool_cir_cham_prom = cir_cham_prom[ply_id];
  mov->pjazz =     jouearr[ply_id];
  if (CondFlag[republican])
    republican_current(ply_id,mov);
  mov->new_spec =  mov->renkam == initsquare ? spec[sq_arrival] : spec[mov->renkam];
  mov->hurdle =    chop[coup_id];
  mov->sb3where =  sb3[coup_id].where;
  mov->sb3what = sb3[coup_id].what;
  if (mov->sb3what!=vide && mov->sb3where==mov->cdzz)
    mov->pjzz = mov->pjazz = mov->sb3what;

  mov->sb2where = sb2[ply_id].where;
  mov->sb2what = sb2[ply_id].what;
  mov->mren = cmren[coup_id];
  mov->osc = oscillatedKs[ply_id];
  /* following only overwritten if change stack is saved in
   * append_to_top_table() */
  /* redundant to init push_top */
  mov->push_bottom = NULL;
  mov->roch_sq=rochade_sq[coup_id];
  mov->roch_pc=rochade_pc[coup_id];
  mov->roch_sp=rochade_sp[coup_id];

  mov->ghost_piece = e[mov->cdzz];
  mov->ghost_flags = spec[mov->cdzz];
  if (CondFlag[BGL]) {
    mov->bgl_wh = BGL_white_store[ply_id];
    mov->bgl_bl = BGL_black_store[ply_id];
  }
}

boolean WriteSpec(Flags sp, boolean printcolours) {
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
    if ( (spname != Volage || !CondFlag[volage])
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
    if (!TSTFLAG(spec[*bnp], Neutral) && e[*bnp] != vide)
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
  slice_solve(si);
  closehash();

#ifdef _SE_DECORATE_SOLUTION_
  se_end_half_duplex();
#endif
}

typedef enum
{
  dont_know_meaning_of_whitetoplay,
  whitetoplay_means_change_colors,
  whitetoplay_means_shorten
} meaning_of_whitetoplay;

static meaning_of_whitetoplay detect_meaning_of_whitetoplay(slice_index si)
{
  meaning_of_whitetoplay result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STGoalReachedTester:
    {
      slice_index const fork = slices[si].u.goal_tester.fork;
      result = detect_meaning_of_whitetoplay(fork);
      break;
    }

    case STGoalMateReachedTester:
    case STGoalStalemateReachedTester:
    case STGoalDoubleStalemateReachedTester:
    case STGoalTargetReachedTester:
    case STGoalCheckReachedTester:
    case STGoalCaptureReachedTester:
    case STGoalSteingewinnReachedTester:
    case STGoalEnpassantReachedTester:
    case STGoalDoubleMateReachedTester:
    case STGoalCounterMateReachedTester:
    case STGoalCastlingReachedTester:
    case STGoalAutoStalemateReachedTester:
    case STGoalCircuitReachedTester:
    case STGoalExchangeReachedTester:
    case STGoalCircuitByRebirthReachedTester:
    case STGoalExchangeByRebirthReachedTester:
    case STGoalAnyReachedTester:
    case STGoalProofgameReachedTester:
    case STGoalMateOrStalemateReachedTester:
      result = whitetoplay_means_shorten;
      break;

    case STGoalAToBReachedTester:
      result = whitetoplay_means_change_colors;
      break;

    case STDefenseAdapter:
    case STReadyForDefense:
    case STContinuationSolver:
    case STDefenseMove:
      result = whitetoplay_means_shorten;
      break;

    case STHelpAdapter:
    case STHelpFindShortest:
    case STReadyForHelpMove:
    case STHelpMoveGenerator:
    case STHelpMove:
    case STMoveInverter:
    case STDeadEnd:
    case STForkOnRemaining:
    case STProxy:
    {
      slice_index const next = slices[si].u.pipe.next;
      result = detect_meaning_of_whitetoplay(next);
      break;
    }

    case STEndOfBranch:
    case STEndOfBranchGoal:
    case STEndOfBranchGoalImmobile:
    case STEndOfBranchForced:
    {
      slice_index const fork = slices[si].u.fork.fork;
      result = detect_meaning_of_whitetoplay(fork);
      break;
    }

    default:
      result = dont_know_meaning_of_whitetoplay;
      break;
  }

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
  slice_index next = slices[proxy].u.pipe.next;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParamListEnd();

  TraceStipulation(proxy);
  assert(slices[proxy].type==STProxy);

  while (slices[next].type==STProxy || slices[next].type==STOutputModeSelector)
    next = slices[next].u.pipe.next;

  TraceEnumerator(SliceType,slices[next].type,"\n");
  switch (slices[next].type)
  {
    case STHelpAdapter:
    {
      meaning_of_whitetoplay const meaning = detect_meaning_of_whitetoplay(next);
      if (meaning==whitetoplay_means_shorten)
      {
        slice_index const inverter = alloc_move_inverter_slice();
        help_branch_shorten(next);
        pipe_append(slices[slices[next].prev].prev,inverter);
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
      meaning_of_whitetoplay const meaning = detect_meaning_of_whitetoplay(next);
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

  stip_impose_starter(si,advers(slices[si].starter));
  swapcolors();
  reflectboard();
  StorePosition();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors duplex_initialisers[] =
{
  { STIntelligentHelpFilter,   &intelligent_init_duplex     },
  { STIntelligentSeriesFilter, &intelligent_init_duplex     }
};

enum
{
  nr_duplex_initialisers = (sizeof duplex_initialisers
                            / sizeof duplex_initialisers[0])
};

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
  stip_structure_traversal_override(&st,
                                    duplex_initialisers,
                                    nr_duplex_initialisers);
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
    stip_impose_starter(si,advers(slices[si].starter));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors duplex_finishers[] =
{
  { STIntelligentHelpFilter,    &intelligent_fini_duplex     },
  { STIntelligentSeriesFilter,  &intelligent_fini_duplex     }
};

enum
{
  nr_duplex_finishers = (sizeof duplex_finishers / sizeof duplex_finishers[0])
};

/* Un-initialise from solving a duplex
 * @param si identifies the root slice of the stipulation
 */
static void fini_duplex(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,duplex_finishers,nr_duplex_finishers);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* Perform verifications and initialisations before solving a twin
 * (which can be the only twin of the problem)
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
        if (!OptFlag[noboard])
          ProofWriteStartPosition(si);
        initialise_piece_flags();

        result = true;
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

  /* allow line-oriented output to restore the initial position */
  StorePosition();

  if (!OptFlag[halfduplex])
    solveHalfADuplex(si);

  if (OptFlag[halfduplex] || OptFlag[duplex])
  {
    /* Set next side to calculate for duplex "twin" */
    stip_impose_starter(si,advers(slices[si].starter));
    TraceStipulation(si);

    init_duplex(si);

    if (locateRoyal() && verify_position(si))
      solveHalfADuplex(si);

    fini_duplex(si);

    stip_impose_starter(si,advers(slices[si].starter));
  }

  Message(NewLine);

  WRITE_COUNTER(empile);
  WRITE_COUNTER(jouecoup);
  WRITE_COUNTER(orig_rbechec);
  WRITE_COUNTER(orig_rnechec);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef enum
{
  solver_insertion_setplay,
  solver_insertion_root,
  solver_insertion_nested
} solver_insertion_state;

void insert_solvers_setplay_fork(slice_index si, stip_structure_traversal *st)
{
  solver_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  *state = solver_insertion_setplay;
  stip_traverse_structure(slices[si].u.fork.fork,st);
  *state = solver_insertion_root;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void insert_solvers_constraint(slice_index si, stip_structure_traversal *st)
{
  solver_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (*state==solver_insertion_root)
  {
    stip_traverse_structure_pipe(si,st);

    *state = solver_insertion_nested;
    stip_traverse_structure(slices[si].u.fork.fork,st);
    *state = solver_insertion_root;
  }
  else
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void insert_solvers_attack_adapter(slice_index si, stip_structure_traversal *st)
{
  solver_insertion_state const * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (*state==solver_insertion_root)
  {
    if (OptFlag[solvariantes])
    {
      if (!OptFlag[nothreat])
        stip_insert_threat_handlers(si);
    }
    else
    {
      slice_index const prototype = alloc_postkeyplay_suppressor_slice();
      battle_branch_insert_slices(si,&prototype,1);
    }

    if (OptFlag[soltout]) /* this includes OptFlag[solessais] */
    {
      branch_insert_try_solvers(si,get_max_nr_refutations());
      {
        slice_index const prototype = alloc_refutations_solver();
        battle_branch_insert_slices(si,&prototype,1);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void insert_solvers_attack(slice_index si, stip_structure_traversal *st)
{
  solver_insertion_state * const state = st->param;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (*state==solver_insertion_root)
  {
    *state = solver_insertion_nested;
    stip_traverse_structure_children(si,st);
    *state = solver_insertion_root;
  }
  else
  {
    stip_traverse_structure_children(si,st);

    if (length>=min_length+2)
    {
      slice_index const proto = alloc_attack_find_shortest_slice(length,
                                                                 min_length);
      battle_branch_insert_slices(si,&proto,1);
    }
  }

  if (length>slack_length_battle)
  {
    slice_index const prototype = alloc_continuation_solver_slice();
    battle_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void insert_solvers_defense_adapter(slice_index si,
                                    stip_structure_traversal *st)
{
  solver_insertion_state * const state = st->param;
  boolean const save_state = *state;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *state = solver_insertion_nested;
  stip_traverse_structure_children(si,st);
  *state = save_state;

  if (*state==solver_insertion_root)
  {
    if (!OptFlag[nothreat])
      stip_insert_threat_handlers(si);

    if (OptFlag[soltout]) /* this includes OptFlag[solessais] */
      Message(TryPlayNotApplicable);
  }
  else
  {
    if (slices[si].u.branch.length>slack_length_battle)
    {
      slice_index const prototype = alloc_continuation_solver_slice();
      battle_branch_insert_slices(si,&prototype,1);
    }
    if (*state==solver_insertion_setplay)
    {
      unsigned int const max_nr_refutations = UINT_MAX;
      branch_insert_try_solvers(si,max_nr_refutations);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void insert_solvers_help(slice_index si, stip_structure_traversal *st)
{
  solver_insertion_state * const state = st->param;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (*state==solver_insertion_nested)
  {
    stip_traverse_structure_children(si,st);

    if (length-min_length>=2)
    {
      slice_index const prototype = alloc_help_find_shortest_slice(length,
                                                                   min_length);
      help_branch_insert_slices(si,&prototype,1);
    }
  }
  else
  {
    solver_insertion_state const save_state = *state;
    *state = solver_insertion_nested;
    stip_traverse_structure_children(si,st);
    *state = save_state;

    if (length-min_length>=2)
    {
      slice_index const ready1 = branch_find_slice(STReadyForHelpMove,si);
      slice_index const ready2 = branch_find_slice(STReadyForHelpMove,ready1);
      slice_index const ready3 = branch_find_slice(STReadyForHelpMove,ready2);
      slice_index const prototypes[] =
      {
        alloc_help_find_by_increasing_length_slice(length,min_length),
        alloc_fork_on_remaining_slice(ready3,length-1-slack_length_help)
      };
      enum
      {
        nr_prototypes = sizeof prototypes / sizeof prototypes[0]
      };
      help_branch_insert_slices(si,prototypes,nr_prototypes);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void insert_solvers_series(slice_index si, stip_structure_traversal *st)
{
  solver_insertion_state * const state = st->param;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (*state==solver_insertion_nested)
  {
    stip_traverse_structure_children(si,st);

    if (length-min_length>=2)
    {
      slice_index const prototype = alloc_series_find_shortest_slice(length,min_length);
      series_branch_insert_slices(si,&prototype,1);
    }
  }
  else
  {
    solver_insertion_state const save_state = *state;
    *state = solver_insertion_nested;
    stip_traverse_structure_children(si,st);
    *state = save_state;

    if (length-min_length>=2)
    {
      slice_index const ready1 = branch_find_slice(STReadyForSeriesMove,si);
      slice_index const ready2 = branch_find_slice(STReadyForSeriesMove,ready1);
      slice_index const ready3 = branch_find_slice(STReadyForSeriesMove,ready2);
      slice_index const prototypes[] =
      {
        alloc_series_find_by_increasing_length_slice(length,min_length),
        alloc_fork_on_remaining_slice(ready3,length-1-slack_length_series)
      };
      enum
      {
        nr_prototypes = sizeof prototypes / sizeof prototypes[0]
      };
      series_branch_insert_slices(si,prototypes,nr_prototypes);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

structure_traversers_visitors const strategy_inserters[] =
{
  { STSetplayFork,    &insert_solvers_setplay_fork    },
  { STConstraint,     &insert_solvers_constraint      },
  { STAttackAdapter,  &insert_solvers_attack_adapter  },
  { STReadyForAttack, &insert_solvers_attack          },
  { STDefenseAdapter, &insert_solvers_defense_adapter },
  { STHelpAdapter,    &insert_solvers_help            },
  { STSeriesAdapter,  &insert_solvers_series          }
};

enum
{
  nr_strategy_inserters = (sizeof strategy_inserters
                           / sizeof strategy_inserters[0])
};

void stip_insert_solvers(slice_index si)
{
  stip_structure_traversal st;
  solver_insertion_state state = solver_insertion_root;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override(&st,
                                    strategy_inserters,
                                    nr_strategy_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Iterate over the twins of a problem
 * @prev_token token that ended the previous twin
 * @return token that ended the current twin
 */
static Token iterate_twins(Token prev_token)
{
  unsigned int twin_index = 0;
  slice_index template_slice_hook;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",prev_token);
  TraceFunctionParamListEnd();

  template_slice_hook = alloc_proxy_slice();

  do
  {
    InitAlways();

    prev_token = ReadTwin(prev_token,template_slice_hook);

    if (twin_index==0)
      /* Set the timer for real calculation time */
      StartTimer();

    if (prev_token==ZeroPosition)
    {
      if (!OptFlag[noboard])
        WritePosition();

      prev_token = ReadTwin(prev_token,template_slice_hook);
      if (LaTeXout)
        LaTeXBeginDiagram();

      ++twin_index;
    }

    if (slices[template_slice_hook].starter==no_side)
    {
      if (OptFlag[quodlibet] && !transform_to_quodlibet(template_slice_hook))
        Message(QuodlibetNotApplicable);

      if (OptFlag[whitetoplay] && !apply_whitetoplay(template_slice_hook))
        Message(WhiteToPlayNotApplicable);

      stip_insert_goal_prerequisite_guards(template_slice_hook);

      if (OptFlag[postkeyplay] && !battle_branch_apply_postkeyplay(template_slice_hook))
        Message(PostKeyPlayNotApplicable);

      stip_insert_root_slices(template_slice_hook);

      if (OptFlag[solapparent] && !OptFlag[restart]
          && !stip_apply_setplay(template_slice_hook))
        Message(SetPlayNotApplicable);

      stip_remove_unsatisfiable_goals(template_slice_hook);

      stip_insert_check_detectors(template_slice_hook);

      if (OptFlag[nontrivial])
        stip_insert_max_nr_nontrivial_guards(template_slice_hook);

      if (!init_intelligent_mode(template_slice_hook))
        Message(IntelligentRestricted);

      if (OptFlag[solflights])
        stip_insert_maxflight_guards(template_slice_hook);

      if (OptFlag[solmenaces]
          && !stip_insert_maxthreatlength_guards(template_slice_hook))
        Message(ThreatOptionAndExactStipulationIncompatible);

      if (OptFlag[movenbr]
          && !(OptFlag[intelligent] && intelligent_mode_overrides_movenbr()))
        stip_insert_restart_guards(template_slice_hook);

      if (dealWithMaxtime())
        stip_insert_maxtime_guards(template_slice_hook);

      if (OptFlag[maxsols])
        stip_insert_maxsolutions_filters(template_slice_hook);

      if (OptFlag[stoponshort]
          && !stip_insert_stoponshortsolutions_filters(template_slice_hook))
        Message(NoStopOnShortSolutions);

      stip_detect_starter(template_slice_hook);
      stip_impose_starter(template_slice_hook,
                          slices[template_slice_hook].starter);
    }

    TraceStipulation(template_slice_hook);

    if (slices[slices[template_slice_hook].u.pipe.next].starter==no_side)
      VerifieMsg(CantDecideWhoIsAtTheMove);
    else if (initialise_verify_twin(slices[template_slice_hook].u.pipe.next))
    {
      slice_index root_slice = stip_deep_copy(template_slice_hook);

      slice_index const template = slices[template_slice_hook].u.pipe.next;
      Side const starter = slices[template].starter;
      stip_impose_starter(root_slice,starter);

      /* only now that we can find out which side's pieces to keep */
      if (OptFlag[keepmating])
        stip_insert_keepmating_filters(root_slice);

      stip_optimise_with_orthodox_mating_move_generators(root_slice);
      stip_optimise_with_countnropponentmoves(root_slice);
      stip_optimise_with_killer_moves(root_slice);

      if (is_hashtable_allocated())
        stip_insert_hash_slices(root_slice);

      if (OptFlag[noshort])
        stip_insert_no_short_variations_filters(root_slice);

      if (CondFlag[amu])
        stip_insert_amu_mate_filters(root_slice);

      if (CondFlag[whiteultraschachzwang]
          || CondFlag[blackultraschachzwang])
        stip_insert_ultraschachzwang_goal_filters(root_slice);

      if (anycirce)
        stip_insert_circe_goal_filters(root_slice);

      if (anyanticirce)
        stip_insert_anticirce_goal_filters(root_slice);

      if (TSTFLAG(PieSpExFlags,Paralyse))
        stip_insert_paralysing_goal_filters(root_slice);

      if (TSTFLAG(PieSpExFlags,Kamikaze))
        stip_insert_kamikaze_goal_filters(root_slice);

      stip_insert_selfcheck_guards(root_slice);

      stip_optimise_dead_end_slices(root_slice);

      stip_optimise_with_end_of_branch_goal_immobile(root_slice);

      /* operations depend on existance of root slices from here on */

      stip_insert_solvers(root_slice);

      if (OptFlag[degeneratetree])
        stip_insert_degenerate_tree_guards(root_slice);

      if (OptFlag[movenbr]
          && !(OptFlag[intelligent] && intelligent_mode_overrides_movenbr()))
        stip_insert_restart_guards(root_slice);

      stip_insert_output_slices(root_slice);

      resolve_proxies(&root_slice);

      /* if root_slice had type STProxy, its value has just changed,
       * and the new slice may not have a starter yet. So let's use
       * template_slice_hook's starter.
       */
      stip_impose_starter(root_slice,
                          slices[slices[template_slice_hook].u.pipe.next].starter);
      TraceStipulation(root_slice);

      solve_twin(root_slice,twin_index,prev_token);

      dealloc_slices(root_slice);
    }

    ++twin_index;
  } while (prev_token==TwinProblem);

  dealloc_slices(template_slice_hook);

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
    FlagMaxSolsPerMatingPosReached = false;
    reset_short_solution_found_in_problem();

    prev_token = iterate_twins(prev_token);

    if (max_solutions_reached()
        || FlagMaxSolsPerMatingPosReached
        || has_short_solution_been_found_in_problem()
        || maxsol_per_matingpos!=ULONG_MAX
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
