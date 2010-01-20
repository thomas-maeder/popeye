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
#include "pyint.h"
#include "pymovein.h"
#include "pybrah.h"
#include "pyquodli.h"
#include "pykeepmt.h"
#include "pyselfcg.h"
#include "pymovenb.h"
#include "pyflight.h"
#include "pydegent.h"
#include "pythreat.h"
#include "pynontrv.h"
#include "pypipe.h"
#include "pyleaf.h"
#include "stipulation/branch.h"
#include "trace.h"
#include "pyslice.h"
#include "pyoutput.h"
#include "conditions/exclusive.h"
#include "conditions/republican.h"
#include "platform/maxmem.h"
#include "platform/maxtime.h"
#include "platform/pytime.h"
#include "platform/priority.h"
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

static void initialise_piece_flags(void)
{
  square const *bnp;
  for (bnp = boardnum; *bnp; bnp++)
  {
    piece const p = e[*bnp];
    if (p!=vide)
    {
      if (CondFlag[volage] && rb!=*bnp && rn!=*bnp)
        SETFLAG(spec[*bnp], Volage);

      if ((PieSpExFlags >> DiaCirce) || flagdiastip || flag_magic)
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

typedef boolean found_slice_types_type[nr_slice_types];

static boolean root_slice_type_found(slice_index si, slice_traversal *st)
{
  found_slice_types_type * const found = st->param;
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  (*found)[slices[si].type] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",true);
  TraceFunctionResultEnd();
  return true;
}

static slice_operation const slice_type_finders[] =
{
  0,                                  /* STProxy */
  0,                                  /* STBranchDirect */
  0,                                  /* STBranchDirectDefender */
  0,                                  /* STBranchHelp */
  0,                                  /* STHelpFork */
  0,                                  /* STBranchSeries */
  0,                                  /* STSeriesFork */
  &root_slice_type_found,             /* STLeafDirect */
  &root_slice_type_found,             /* STLeafHelp */
  &root_slice_type_found,             /* STLeafForced */
  &slice_traverse_children,           /* STReciprocal */
  &slice_traverse_children,           /* STQuodlibet */
  &slice_traverse_children,           /* STNot */
  &slice_traverse_children,           /* STMoveInverter */
  &root_slice_type_found,             /* STDirectRoot */
  &root_slice_type_found,             /* STDirectDefenderRoot */
  &slice_traverse_children,           /* STDirectHashed */
  &root_slice_type_found,             /* STHelpRoot */
  0,                                  /* STHelpHashed */
  &root_slice_type_found,             /* STSeriesRoot */
  0,                                  /* STParryFork */
  0,                                  /* STSeriesHashed */
  &slice_traverse_children,           /* STSelfCheckGuard */
  &slice_traverse_children,           /* STDirectDefense */
  0,                                  /* STReflexGuard */
  0,                                  /* STSelfAttack */
  0,                                  /* STSelfDefense */
  0,                                  /* STRestartGuard */
  0,                                  /* STGoalReachableGuard */
  0,                                  /* STKeepMatingGuard */
  0,                                  /* STMaxFlightsquares */
  0,                                  /* STDegenerateTree */
  0,                                  /* STMaxNrNonTrivial */
  0                                   /* STMaxThreatLength */
};

static SliceType findUniqueRootSliceType(void)
{
  SliceType found_slice_type = no_slice;
  SliceType s;
  slice_traversal st;
  found_slice_types_type found_slice_types = { false };

  slice_traversal_init(&st,&slice_type_finders,&found_slice_types);
  traverse_slices(root_slice,&st);

  for (s = 0; s!=nr_slice_types; ++s)
    if (found_slice_types[s])
    {
      if (found_slice_type==no_slice)
        found_slice_type = s;
      else
        return no_slice_type;
    }

  return found_slice_type;
}

static boolean determineRestrictedSide(void)
{
  boolean result;
  SliceType unique_slice_type;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  unique_slice_type = findUniqueRootSliceType();
  if (unique_slice_type==no_slice_type)
    result = false;
  else
  {
    Side const restricted_side = (unique_slice_type==STHelpRoot
                                  ? slices[root_slice].starter
                                  : advers(slices[root_slice].starter));
    if (flagmaxi)
    {
      if (restricted_side==Black)
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
      CondFlag[blackultraschachzwang] = restricted_side==Black;
      CondFlag[whiteultraschachzwang] = restricted_side==White;
    }

    result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static Goal const proof_goals[] = { goal_proof, goal_atob };

static unsigned int const nr_proof_goals = (sizeof proof_goals
                                            / sizeof proof_goals[0]);

static boolean verify_position(void)
{
  square const *bnp;
  piece     p;
  ply           n;
  int      cp, pp, tp, op;
  boolean          nonoptgenre;

  jouegenre = false;
  jouetestgenre = false;
  nonoptgenre = false;
  supergenre = false;
  reset_ortho_mating_moves_generation_obstacles();

  move_generation_mode_opti_per_side[White]
      = move_generation_optimized_by_nr_opponent_moves;
  move_generation_mode_opti_per_side[Black]
      = move_generation_optimized_by_nr_opponent_moves;

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

  if (slices[root_slice].type==STBranchDirect)
  {
    slice_index const peer = slices[root_slice].u.pipe.next;
    slice_index const next = slices[peer].u.pipe.next;
    assert(slices[peer].type==STBranchDirectDefender);

    if (2*get_max_threat_length()+slack_length_direct
        <slices[root_slice].u.pipe.u.branch.min_length)
    {
      VerifieMsg(ThreatOptionAndExactStipulationIncompatible);
      return false;
    }

    if (slices[root_slice].u.pipe.u.branch.length<=get_max_threat_length())
      reset_max_threat_length();

    if (slices[root_slice].u.pipe.u.branch.length<1
        && max_nr_refutations>0
        && slices[next].type!=STLeafHelp)
    {
      ErrorMsg(TryInLessTwo);
      max_nr_refutations = 0;
    }

    if (OptFlag[stoponshort])
    {
      ErrorMsg(NoStopOnShortSolutions);
      OptFlag[stoponshort] = false;
    }
  }

  if (get_max_nr_moves(root_slice) >= maxply-2)
  {
    VerifieMsg(BigNumMoves);
    return false;
  }

  if (CondFlag[parrain])
  {
    Goal const pieceWinGoals[] = { goal_steingewinn };
    size_t const nrPieceWinGoals = (sizeof pieceWinGoals
                                    / sizeof pieceWinGoals[0]);
    if (stip_ends_in_one_of(pieceWinGoals,nrPieceWinGoals))
    {
      VerifieMsg(PercentAndParrain);
      return false;
    }
  }

  {
    Goal const diastipGoals[] =
    {
      goal_circuit,
      goal_exchange,
      goal_circuitB,
      goal_exchangeB
    };

    size_t const nrDiastipGoals = (sizeof diastipGoals
                                   / sizeof diastipGoals[0]);
    flagdiastip = stip_ends_only_in(diastipGoals,nrDiastipGoals);
  }

  if (TSTFLAG(PieSpExFlags, HalfNeutral))
    SETFLAG(PieSpExFlags, Neutral);

  if (CondFlag[republican] && !republican_verifie_position())
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

  if (flagmaxi || flagultraschachzwang)
    if (!determineRestrictedSide())
    {
      VerifieMsg(CantDecideOnSideWhichConditionAppliesTo);
      return false;
    }

  if (flagultraschachzwang)
  {
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
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

  if (TSTFLAG(PieSpExFlags, Neutral))
    flag_nk = rb!=initsquare && TSTFLAG(spec[rb],Neutral);

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

  if (flagdiastip
      && (CondFlag[frischauf]
          || CondFlag[sentinelles]
          || CondFlag[imitators]))
  {
    VerifieMsg(DiaStipandsomeCond);
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

  nonkilgenre = CondFlag[messigny]
      || (CondFlag[singlebox] && SingleBoxType==singlebox_type3)
      || CondFlag[whsupertrans_king]
      || CondFlag[blsupertrans_king]
      || CondFlag[takemake];

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
  for (p = roib; p <= derbla; p++) {
    if (exist[p] && p != dummyb && p != hamstb)
    {
      if (whitenormaltranspieces)
        whitetransmpieces[tp] = p;
      if (blacknormaltranspieces)
        blacktransmpieces[tp] = p;
      tp++;
      if (p != Orphan
          && p != Friend
          && (exist[Orphan] || exist[Friend]))
        orphanpieces[op++] = p;
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
    Goal const incompatibleGoals[] =
    {
      goal_mate,
      goal_check,
      goal_mate_or_stale
    };
    size_t const nrIncompatibleGoals
        = sizeof incompatibleGoals / sizeof incompatibleGoals[0];
    
    if (stip_ends_in_one_of(incompatibleGoals,nrIncompatibleGoals))
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

  {
    Goal const castlingGoal = goal_castling;
    if (stip_ends_in_one_of(&castlingGoal,1)
        && !castling_supported)
    {
      VerifieMsg(StipNotSupported);
      return false;
    }
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

  if (CondFlag[exclusive] && !exclusive_verifie_position())
    return false;

  if (CondFlag[isardam]
      || CondFlag[ohneschach])
  {
    flag_testlegality = true;
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
  }

  if (!CondFlag[patience]) {           /* needed because of twinning */
    PatienceB = false;
  }

  {
    Goal const doublemate_goals[] = { goal_doublemate };

    jouetestgenre = jouetestgenre
        || flag_testlegality
        || flagAssassin
        || stip_ends_in_one_of(doublemate_goals,1)
        || CondFlag[patience]
        || CondFlag[blackultraschachzwang]
        || CondFlag[whiteultraschachzwang]
        || CondFlag[BGL];
    jouetestgenre_save = jouetestgenre;
  }

  jouetestgenre1 = CondFlag[blackultraschachzwang]
      || CondFlag[whiteultraschachzwang];      


  nonoptgenre = nonoptgenre
      || TSTFLAG(PieSpExFlags, Neutral)
      || flag_testlegality
      || anymars
      || anyantimars
      || CondFlag[brunner]
      || CondFlag[blsupertrans_king]
      || CondFlag[whsupertrans_king]
      || CondFlag[takemake]
      || CondFlag[circecage];

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
    nonoptgenre = true;
    WhiteStrictSAT[1] = echecc_normal(nbply,White);
    BlackStrictSAT[1] = echecc_normal(nbply,Black);
    satXY = WhiteSATFlights > 1 || BlackSATFlights > 1;
  }

  if (CondFlag[schwarzschacher])
  {
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
    nonoptgenre = true;
  }

  if (flagwhitemummer || nonoptgenre)
    move_generation_mode_opti_per_side[White] = 
        move_generation_optimized_by_killer_move;

  if (flagblackmummer || nonoptgenre)
    move_generation_mode_opti_per_side[Black] = 
        move_generation_optimized_by_killer_move;

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
  square sq = move_generation_stack[coup_id].arrival;

  mov->tr =          trait[ply_id];
  mov->cdzz =           move_generation_stack[coup_id].departure;
  mov->cazz =            sq;
  mov->cpzz =            move_generation_stack[coup_id].capture;
  mov->pjzz =            pjoue[ply_id];
  mov->norm_prom =       norm_prom[ply_id];
  mov->ppri =            pprise[ply_id];
  mov->sqren =           sqrenais[ply_id];
  mov->cir_prom =        cir_prom[ply_id];

  mov->echec = echecc(ply_id,advers(mov->tr));

  mov->renkam = crenkam[ply_id];
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
  mov->new_spec =  spec[sq];
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

/* assert()s below this line must remain active even in "productive"
 * executables. */
#undef NDEBUG
#include <assert.h>

/* Check assumptions made throughout the program. Abort if one of them
 * isn't met. */
static void checkGlobalAssumptions(void)
{
  /* Make sure that the characters relevant for entering problems are
   * encoded contiguously and in the natural order. This is assumed
   * in pyio.c.
   *
   * NB: There is no need for the analoguous check for digits, because
   * decimal digits are guaranteed by the language to be encoded
   * contiguously and in the natural order. */
  assert('b'=='a'+1);
  assert('c'=='b'+1);
  assert('d'=='c'+1);
  assert('e'=='d'+1);
  assert('f'=='e'+1);
  assert('g'=='f'+1);
  assert('h'=='g'+1);

  check_hash_assumptions();
}

static void solveHalfADuplex(void)
{
  inithash();

  if (isIntelligentModeActive
      && OptFlag[restart]
      && !stip_ends_in_one_of(proof_goals,nr_proof_goals))
  {
    /* In intelligent mode, the restart number means the minimal
     * number of moves.
     */
    stip_length_type const
        save_min_length = set_min_length(root_slice,get_restart_number());
    OptFlag[restart] = false;
    slice_root_solve(root_slice);
    OptFlag[restart] = true;
    set_min_length(root_slice,save_min_length);
  }
  else
    slice_root_solve(root_slice);

  closehash();

  output_end_half_duplex();
}

typedef enum
{
  dont_know_meaning_of_whitetoplay,
  whitetoplay_means_change_colors,
  whitetoplay_means_shorten_root_slice
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
    case STLeafHelp:
      if (slices[si].u.leaf.goal==goal_atob)
        result = whitetoplay_means_change_colors;
      else
        result = whitetoplay_means_shorten_root_slice;
      break;

    case STLeafDirect:
    case STLeafForced:
      result = whitetoplay_means_shorten_root_slice;
      break;

    case STHelpRoot:
    case STBranchHelp:
    case STMoveInverter:
    case STNot:
    case STProxy:
    {
      slice_index const next = slices[si].u.pipe.next;
      result = detect_meaning_of_whitetoplay(next);
      break;
    }

    case STHelpFork:
    case STSeriesFork:
    {
      slice_index const to_goal = slices[si].u.pipe.u.branch.towards_goal;
      result = detect_meaning_of_whitetoplay(to_goal);
      break;
    }

    case STReciprocal:
    case STQuodlibet:
    {
      slice_index const op1 = slices[si].u.fork.op1;
      meaning_of_whitetoplay const res1 = detect_meaning_of_whitetoplay(op1);
      slice_index const op2 = slices[si].u.fork.op2;
      meaning_of_whitetoplay const res2 = detect_meaning_of_whitetoplay(op2);
      if (res1==res2)
        result = res1;
      else
        result = dont_know_meaning_of_whitetoplay;
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
static void apply_whitetoplay(slice_index proxy)
{
  slice_index const next = slices[proxy].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParamListEnd();

  TraceStipulation(proxy);
  assert(slices[proxy].type==STProxy);

  TraceEnumerator(SliceType,slices[next].type,"\n");
  switch (slices[next].type)
  {
    case STHelpRoot:
    {
      meaning_of_whitetoplay const meaning = detect_meaning_of_whitetoplay(next);
      /* calculate new starter now - shorten_root_branch_h_slice() may
       * replace si
       */
      if (meaning==whitetoplay_means_shorten_root_slice)
      {
        slice_index const shortened = help_root_shorten_help_play(next);
        slice_index const inverter = alloc_move_inverter_slice();
        branch_link(inverter,shortened);
        branch_link(proxy,inverter);
        slices[inverter].starter = advers(slices[shortened].starter);
        TraceValue("%u\n",slices[inverter].starter);
      }
      else
        slices[next].starter = advers(slices[next].starter);
      break;
    }

    case STLeafHelp:
      slices[next].starter = advers(slices[next].starter);
      break;

    case STMoveInverter:
    {
      meaning_of_whitetoplay const meaning = detect_meaning_of_whitetoplay(next);
      slice_index const inverter = next;
      slice_index const next_next = slices[inverter].u.pipe.next;
      if (slices[next_next].prev==inverter)
        pipe_set_predecessor(next_next,slices[inverter].prev);
      dealloc_slice(inverter);
      if (meaning==whitetoplay_means_shorten_root_slice
          && slices[next_next].type==STHelpRoot)
        pipe_set_successor(proxy,help_root_shorten_help_play(next_next));
      else
        pipe_set_successor(proxy,next_next);
      break;
    }

    case STQuodlibet:
    case STReciprocal:
      apply_whitetoplay(slices[next].u.fork.op1);
      apply_whitetoplay(slices[next].u.fork.op2);
      assert(slices[slices[next].u.fork.op1].starter
             ==slices[slices[next].u.fork.op2].starter);
      slices[next].starter = slices[slices[next].u.fork.op1].starter;
      break;

    default:
      pipe_set_successor(proxy,no_slice);
      break;
  }

  if (slices[proxy].u.pipe.next!=no_slice)
    slices[proxy].starter  = slices[slices[proxy].u.pipe.next].starter;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static int parseCommandlineOptions(int argc, char *argv[])
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

/* prepare for solving duplex */
static void init_duplex(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",isIntelligentModeActive);
  if (isIntelligentModeActive)
  {
    /*
     * A hack to make the intelligent mode work with duplex.
     */
    swapcolors();

    reflectboard();

    /* allow line-oriented output to restore the initial
     * position */
    StorePosition();
  }
  else
  {
    Side const starter = slices[root_slice].starter;
    TraceValue("%u\n",starter);
    stip_impose_starter(advers(starter));

    TraceStipulation(root_slice);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* restore from preparations for solving duplex */
static void fini_duplex(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (isIntelligentModeActive)
  {
    /* cf. init_duplex */
    reflectboard();
    swapcolors();
  }
  else
  {
    Side const starter = slices[root_slice].starter;
    stip_impose_starter(advers(starter));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traverse a slice while inserting hash elements
 * @param si identifies slice
 * @param st address of structure holding status of traversal
 * @return result of traversing si's children
 */
boolean insert_hash_element_branch_direct(slice_index si, slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* First traverse childen, then insert STDirectHashed slice;
   * otherwise the STDirectHashed will be traversed as well.
   */
  slice_traverse_children(si,st);
  insert_directhashed_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Traverse a slice while inserting hash elements
 * @param si identifies slice
 * @param st address of structure holding status of traversal
 * @return result of traversing si's children
 */
boolean insert_hash_element_direct_defender_root(slice_index si,
                                                 slice_traversal *st)
{
  boolean const result = true;
  branch_level * const level = st->param;
  branch_level const save_level = *level;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* First traverse childen, then insert STDirectHashed slice;
   * otherwise the STDirectHashed will be traversed as well.
   */
  *level = nested_branch;
  slice_traverse_children(si,st);
  *level = save_level;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_goal_move_oriented(slice_index leaf)
{
  boolean result;
  Goal const goal = slices[leaf].u.leaf.goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  result = (goal==goal_target
            || goal==goal_ep
            || goal==goal_capture
            || goal==goal_steingewinn
            || goal==goal_castling);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* Traverse a slice while inserting hash elements
 * @param si identifies slice
 * @param st address of structure holding status of traversal
 * @return result of traversing si's children
 */
boolean insert_hash_element_leaf_direct(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  branch_level const * const level = st->param;;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* In move orientated stipulations (%, z, x etc.) it's less
   * expensive to compute an end in 1. TLi
   */
  if (*level==nested_branch && !is_goal_move_oriented(si))
    insert_directhashed_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Traverse a slice while inserting hash elements
 * @param si identifies slice
 * @param st address of structure holding status of traversal
 * @return result of traversing si's children
 */
boolean insert_hash_element_branch_help(slice_index si, slice_traversal *st)
{
  boolean result;
  slice_index const towards_goal = slices[si].u.pipe.u.branch.towards_goal;
  branch_level * const level = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (*level==nested_branch)
  {
    /* First traverse childen, then insert STHelpHashed slice;
     * otherwise the STHelpHashed will be traversed as well.
     */
    slice_traverse_children(si,st);
    insert_helphashed_slice(si);
    traverse_slices(towards_goal,st);
    /* no need to visit this leaf again in this traversal */
    result = true;
  }
  else
  {
    *level = nested_branch;
    slice_traverse_children(si,st);
    *level = toplevel_branch;
    /* on the next visit, *level might be ==nested_branch */
    result = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Traverse a slice while inserting hash elements
 * @param si identifies slice
 * @param st address of structure holding status of traversal
 * @return result of traversing si's children
 */
boolean insert_hash_element_leaf_help(slice_index si, slice_traversal *st)
{
  boolean result;
  branch_level const * const level = st->param;;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (*level==nested_branch)
  {
    insert_helphashed_slice(si);
    /* no need to visit this leaf again in this traversal */
    result = true;
  }
  else
    /* on the next visit, *level might be ==nested_branch */
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Traverse a slice while inserting hash elements
 * @param si identifies slice
 * @param st address of structure holding status of traversal
 * @return result of traversing si's children
 */
boolean insert_hash_element_branch_series(slice_index si, slice_traversal *st)
{
  boolean result;
  branch_level * const level = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (*level==nested_branch)
  {
    /* First traverse childen, then insert STSeriesHashed slice;
     * otherwise the STSeriesHashed will be traversed as well.
     */
    slice_traverse_children(si,st);
    insert_serieshashed_slice(si);
    /* no need to visit this leaf again in this traversal */
    result = true;
  }
  else
  {
    *level = nested_branch;
    slice_traverse_children(si,st);
    *level = toplevel_branch;
    /* on the next visit, *level might be ==nested_branch */
    result = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const hash_element_inserters[] =
{
  &slice_traverse_children,                  /* STProxy */
  &insert_hash_element_branch_direct,        /* STBranchDirect */
  &slice_traverse_children,                  /* STBranchDirectDefender */
  &insert_hash_element_branch_help,          /* STBranchHelp */
  &slice_traverse_children,                  /* STHelpFork */
  &insert_hash_element_branch_series,        /* STBranchSeries */
  &slice_traverse_children,                  /* STSeriesFork */
  &insert_hash_element_leaf_direct,          /* STLeafDirect */
  &insert_hash_element_leaf_help,            /* STLeafHelp */
  &slice_traverse_children,                  /* STLeafForced */
  &slice_traverse_children,                  /* STReciprocal */
  &slice_traverse_children,                  /* STQuodlibet */
  &slice_traverse_children,                  /* STNot */
  &slice_traverse_children,                  /* STMoveInverter */
  &slice_traverse_children,                  /* STDirectRoot */
  &insert_hash_element_direct_defender_root, /* STDirectDefenderRoot */
  &slice_traverse_children,                  /* STDirectHashed */
  &slice_traverse_children,                  /* STHelpRoot */
  &slice_traverse_children,                  /* STHelpHashed */
  &slice_traverse_children,                  /* STSeriesRoot */
  &pipe_traverse_next,                       /* STParryFork */
  &slice_traverse_children,                  /* STSeriesHashed */
  &slice_traverse_children,                  /* STSelfCheckGuard */
  &slice_traverse_children,                  /* STDirectDefense */
  &slice_traverse_children,                  /* STReflexGuard */
  &slice_traverse_children,                  /* STSelfAttack */
  &slice_traverse_children,                  /* STSelfDefense */
  &slice_traverse_children,                  /* STRestartGuard */
  &slice_traverse_children,                  /* STGoalReachableGuard */
  &slice_traverse_children,                  /* STKeepMatingGuard */
  &slice_traverse_children,                  /* STMaxFlightsquares */
  &slice_traverse_children,                  /* STDegenerateTree */
  &slice_traverse_children,                  /* STMaxNrNonTrivial */
  &slice_traverse_children                   /* STMaxThreatLength */
};

static void insert_hash_slices(void)
{
  slice_traversal st;
  branch_level level = toplevel_branch;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  slice_traversal_init(&st,&hash_element_inserters,&level);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean fork_resolve_proxies(slice_index si, slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_resolve_proxy(&slices[si].u.fork.op1);
  pipe_resolve_proxy(&slices[si].u.fork.op2);
  slice_traverse_children(si,st);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean pipe_resolve_proxies(slice_index si, slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.pipe.next!=no_slice)
    pipe_resolve_proxy(&slices[si].u.pipe.next);
  slice_traverse_children(si,st);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean branch_resolve_proxies(slice_index si, slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.pipe.u.branch.towards_goal!=no_slice)
    pipe_resolve_proxy(&slices[si].u.pipe.u.branch.towards_goal);
  pipe_resolve_proxies(si,st);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean reflex_guard_resolve_proxies(slice_index si, slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_resolve_proxy(&slices[si].u.pipe.u.reflex_guard.avoided);
  pipe_resolve_proxies(si,st);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const proxy_resolvers[] =
{
  &slice_traverse_children,      /* STProxy */
  &branch_resolve_proxies,       /* STBranchDirect */
  &branch_resolve_proxies,       /* STBranchDirectDefender */
  &branch_resolve_proxies,       /* STBranchHelp */
  &branch_resolve_proxies,       /* STHelpFork */
  &branch_resolve_proxies,       /* STBranchSeries */
  &branch_resolve_proxies,       /* STSeriesFork */
  &slice_traverse_children,      /* STLeafDirect */
  &slice_traverse_children,      /* STLeafHelp */
  &slice_traverse_children,      /* STLeafForced */
  &fork_resolve_proxies,         /* STReciprocal */
  &fork_resolve_proxies,         /* STQuodlibet */
  &slice_traverse_children,      /* STNot */
  &pipe_resolve_proxies,         /* STMoveInverter */
  &branch_resolve_proxies,       /* STDirectRoot */
  &branch_resolve_proxies,       /* STDirectDefenderRoot */
  &branch_resolve_proxies,       /* STDirectHashed */
  &branch_resolve_proxies,       /* STHelpRoot */
  &branch_resolve_proxies,       /* STHelpHashed */
  &branch_resolve_proxies,       /* STSeriesRoot */
  &slice_traverse_children,      /* STParryFork */
  &branch_resolve_proxies,       /* STSeriesHashed */
  &pipe_resolve_proxies,         /* STSelfCheckGuard */
  &branch_resolve_proxies,       /* STDirectDefense */
  &reflex_guard_resolve_proxies, /* STReflexGuard */
  &branch_resolve_proxies,       /* STSelfAttack */
  &branch_resolve_proxies,       /* STSelfDefense */
  &slice_traverse_children,      /* STRestartGuard */
  &pipe_resolve_proxies,         /* STGoalReachableGuard */
  &slice_traverse_children,      /* STKeepMatingGuard */
  &slice_traverse_children,      /* STMaxFlightsquares */
  &slice_traverse_children,      /* STDegenerateTree */
  &slice_traverse_children,      /* STMaxNrNonTrivial */
  &slice_traverse_children       /* STMaxThreatLength */
};

/* Substitute links to proxy slices by the proxy's target
 */
static void resolve_proxies(void)
{
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  assert(slices[root_slice].type==STProxy);
  pipe_resolve_proxy(&root_slice);

  slice_traversal_init(&st,&proxy_resolvers,0);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


static boolean initialise_verify_twin(void)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  initPieces();

  if (stip_ends_in_one_of(proof_goals,nr_proof_goals))
  {
    countPieces();
    if (locateRoyal())
    {
      ProofSaveTargetPosition();

      {
        Goal const proof_goal = goal_proof;
        if (stip_ends_in_one_of(&proof_goal,1))
          ProofInitialiseStartPosition();
      }

      ProofRestoreStartPosition();

      countPieces();
      if (locateRoyal() && verify_position())
      {
        ProofSaveStartPosition();
        ProofRestoreTargetPosition();

        ProofInitialise();

        if (!OptFlag[noboard])
          WritePosition();
        initialise_piece_flags();

        ProofRestoreStartPosition();
        if (!OptFlag[noboard])
          ProofWriteStartPosition();
        initialise_piece_flags();

        result = true;
      }
    }
  }
  else
  {
    countPieces();
    if (locateRoyal() && verify_position())
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
 * @param twin_index 0 for first, 1 for second ...; if the problem has
 *                   a zero position, solve_twin() is invoked with
 *                   1, 2, ... but not with * 0
 * @param end_of_twin_token token that ended this twin
 */
static void solve_twin(unsigned int twin_index, Token end_of_twin_token)
{
  if (initialise_verify_twin())
  {
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
    solveHalfADuplex();

    if (OptFlag[duplex])
    {
      /* Set next side to calculate for duplex "twin" */
      if (OptFlag[stoponshort] && FlagShortSolsReached)
        FlagMaxSolsReached = true;

      FlagShortSolsReached = false;

      init_duplex();

      if (locateRoyal() && verify_position())
        solveHalfADuplex();

      fini_duplex();
    }

    Message(NewLine);
  }
}

/* Iterate over the twins of a problem
 * @prev_token token that ended the previous twin
 * @return token that ended the current twin
 */
static Token iterate_twins(Token prev_token)
{
  unsigned int twin_index = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",prev_token);
  TraceFunctionParamListEnd();

  do
  {
    boolean shouldDetectStarter = false;
 
    InitAlways();

    prev_token = ReadTwin(prev_token,&shouldDetectStarter);

    if (twin_index==0)
      /* Set the timer for real calculation time */
      StartTimer();

    if (prev_token==ZeroPosition)
    {
      boolean dummy;

      if (!OptFlag[noboard])
        WritePosition();

      prev_token = ReadTwin(prev_token,&dummy);
      shouldDetectStarter = true;
      if (LaTeXout)
        LaTeXBeginDiagram();

      ++twin_index;
    }

    dealWithMaxtime();

    TraceValue("%u",twin_index);
    TraceValue("%u\n",shouldDetectStarter);
    if (twin_index==0 || shouldDetectStarter)
    {
      if (OptFlag[postkeyplay] && !stip_apply_postkeyplay())
        Message(PostKeyPlayNotApplicable);

      stip_detect_starter();

      switch (stip_supports_intelligent())
      {
        case intelligent_not_supported:
          if (OptFlag[intelligent])
            Message(IntelligentRestricted);
          isIntelligentModeActive = false;
          break;

        case intelligent_not_active_by_default:
          isIntelligentModeActive = OptFlag[intelligent];
          break;

        case intelligent_active_by_default:
          isIntelligentModeActive = true;
          break;
      }
      TraceValue("%u\n",isIntelligentModeActive);

      if (OptFlag[whitetoplay])
      {
        apply_whitetoplay(root_slice);
        if (slices[root_slice].u.pipe.next==no_slice)
          Message(WhiteToPlayNotApplicable);
      }

      if (OptFlag[nontrivial])
        stip_insert_max_nr_nontrivial_guards();
      
      stip_insert_selfcheck_guards();

      /* Add slices responsible for setplay only *after* those
       * responsible for detecting selfchecks, beause otherwise, we
       * would test for illegal selfchecks twice at the beginning of
       * setplay.
       */
      if (OptFlag[solapparent] && !OptFlag[restart] && !stip_apply_setplay())
        Message(SetPlayNotApplicable);

      if (is_hashtable_allocated())
        insert_hash_slices();

      if (isIntelligentModeActive)
        stip_insert_goalreachable_guards();

      if (!OptFlag[intelligent] && OptFlag[movenbr])
        stip_insert_restart_guards();

      if (OptFlag[keepmating])
        stip_insert_keepmating_guards();

      if (OptFlag[solflights])
        stip_insert_maxflight_guards();

      if (OptFlag[solmenaces])
        stip_insert_maxthreatlength_guards();

      if (OptFlag[degeneratetree])
        stip_insert_degenerate_tree_guards();

      /* intelligent AND duplex means that the board is mirrored and
       * the colors swapped by swapcolors() and reflectboard() ->
       * start with the regular side. */
      if (OptFlag[halfduplex] && !isIntelligentModeActive)
      {
        Side const non_duplex_starter = slices[root_slice].starter;
        stip_impose_starter(advers(non_duplex_starter));
      }
      else
        stip_impose_starter(slices[root_slice].starter);

      resolve_proxies();

      dealloc_proxy_pipes();

      TraceStipulation(root_slice);

      assert_no_leaked_slices();
    }

    if (slices[root_slice].starter==no_side)
      VerifieMsg(CantDecideWhoIsAtTheMove);
    else
    {
      TraceValue("%u\n",slices[root_slice].starter);
      solve_twin(twin_index,prev_token);

      if (OptFlag[stoponshort] && FlagShortSolsReached)
        FlagMaxSolsReached = true;

      FlagShortSolsReached = false;
    }

    ++twin_index;
  } while (prev_token==TwinProblem);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",prev_token);
  TraceFunctionResultEnd();
  return prev_token;
}

/* Iterate over the problems read from standard input or the input
 * file indicated in the command line options
 */
static void iterate_problems(void)
{
  Token prev_token = BeginProblem;

  do
  {
    InitBoard();
    InitCond();
    InitOpt();
    InitStip();

    FlagMaxSolsReached = false;
    FlagShortSolsReached = false;

    prev_token = iterate_twins(prev_token);

    if (FlagMaxSolsReached
        || (isIntelligentModeActive && maxsol_per_matingpos!=ULONG_MAX)
        || periods_counter>=nr_periods)
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

#include <limits.h>

/* Guess the "bitness" of the platform
 * @return 32 if we run on a 32bit platform etc.
 */
static unsigned int guessPlatformBitness(void)
{
#if defined(__unix) || __APPLE__ & __MACH__
#  if defined(ULONG_MAX) && ULONG_MAX==18446744073709551615U
  return 64;
#  else
  return 32;
#  endif
#elif defined(_WIN64)
  return 64;
#elif defined(_WIN32)
  return 32;
#endif
}

int main(int argc, char *argv[])
{
  int idx_end_of_options;

  checkGlobalAssumptions();

  set_nice_priority();

  init_slice_allocator();

  sprintf(versionString,
          "Popeye %s-%uBit v%.2f",
          OSTYPE,guessPlatformBitness(),VERSION);
  
  MaxPositions = ULONG_MAX;
  LaTeXout = false;
  flag_regression = false;

  initMaxmem();

  /* Initialize message table with default language.
   * This default setting is hopefully overriden later by ReadBeginSpec().
   */
  InitMsgTab(LanguageDefault);

  idx_end_of_options = parseCommandlineOptions(argc,argv);
  
  OpenInput(idx_end_of_options<argc ? argv[idx_end_of_options] : " ");

  initMaxtime();

  if (!dimensionHashtable())
    pyfputs("Couldn't allocate the requested amount of memory\n",stdout);

  /* start timer to be able to display a reasonable time if the user
   * aborts execution before the timer is started for the first
   * problem */
  StartTimer();

  InitCheckDir();

  /* Don't use StdString() - possible trace file is not yet opened
   */
  pyfputs(versionString,stdout);
  pyfputs(maxmemString(),stdout);

  iterate_problems();

  CloseInput();

  if (LaTeXout)
    LaTeXClose();
  
  return 0;
}

#if defined(NOMEMSET)
void memset(char *poi, char val, int len)
{
  while (len--)
    *poi++ = val;
}
#endif
