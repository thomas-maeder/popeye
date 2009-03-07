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
#include "pyquodli.h"
#include "platform/maxmem.h"
#include "platform/maxtime.h"
#include "platform/pytime.h"
#include "platform/priority.h"
#include "trace.h"
#include "pyslice.h"
#include "pyoutput.h"
#include "trace.h"

boolean supergenre;

unsigned long MaxMemory;
maxmemory_unit_type MaxMemory_unit;
char MaxMemoryString[37];

sig_atomic_t volatile maxtime_status;

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
    *kingsquare= square;
    return true;
  }
  else
    return false;
}

static void countPieces(void)
{
  square        *bnp;
  piece     p;

  TraceFunctionEntry(__func__);
  TraceText("\n");

  for (p = roib; p<=derbla; p++)
  {
    nbpiece[p] = 0;
    nbpiece[-p] = 0;
  }

  for (p= roib; p<=fb; p++)
    exist[p]= true;

  for (p= fb+1; p<=derbla; p++)
    exist[p]= false;

  if (CondFlag[sentinelles])
  {
    exist[sentineln]= true;
    exist[sentinelb]= true;
  }

  for (bnp = boardnum; *bnp; bnp++)
  {
    p = e[*bnp];
    if (p!=vide)
    {
      if (p<fn)
        exist[-p]= true;
      else if (p>fb)
        exist[p]= true;

      ++nbpiece[p];
    }
  }

  if (CondFlag[leofamily])
    for (p = db; p<=fb; p++)
      exist[p]= false;

  if (CondFlag[chinoises])
    for (p= leob; p <= vaob; p++)
      exist[p]= true;

  if (CondFlag[cavaliermajeur])
  {
    exist[cb]= false;
    exist[nb]= true;
  }

  for (p= CondFlag[losingchess] || CondFlag[dynasty] ? roib : db;
       p <= derbla;
       p++)
    if (promonly[p])
      exist[p]= true;

  if (CondFlag[protean])
    exist[reversepb]= true;

  TraceValue("%d\n",nbpiece[pb]);

  TraceFunctionExit(__func__);
  TraceText("\n");
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
    square *bnp;
    square s;

    OptFlag[sansrn]= true;
    OptFlag[sansrb]= true;

    if (nbpiece[roib]==1)
      for (bnp= boardnum; *bnp; bnp++)
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
      for (bnp= boardnum; *bnp; bnp++)
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
  else if (CondFlag[losingchess])
  {
    OptFlag[sansrn]= true;
    OptFlag[sansrb]= true;
  }
  else
  {
    square *bnp;
    for (bnp= boardnum; *bnp; bnp++)
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

static Goal const proof_goals[] = { goal_proof, goal_atob };

static unsigned int const nr_proof_goals = (sizeof proof_goals
                                            / sizeof proof_goals[0]);

static boolean verify_position(void)
{
  square        *bnp;
  piece     p;
  ply           n;
  int      cp, pp, tp, op;
  boolean          nonoptgenre;

  if (CondFlag[glasgow] && CondFlag[circemalefique])
    anycirprom= true;

  /* initialize promotion squares */
  if (!CondFlag[einstein])
  {
    square i;
    if (!CondFlag[whprom_sq])
      for (i= 0; i < 8; i++)
        SETFLAG(sq_spec[CondFlag[glasgow] ? square_h7-i : square_h8-i],
                WhPromSq);

    if (!CondFlag[blprom_sq])
      for (i= 0; i < 8; i++)
        SETFLAG(sq_spec[CondFlag[glasgow] ? square_a2+i : square_a1+i],
                BlPromSq);

  }

  im0= isquare[0];
  if (! CondFlag[imitators])
    CondFlag[noiprom]= true;

  if (slices[root_slice].type==STBranchDirect)
  {
    slice_index const peer = slices[root_slice].u.branch_d.peer;
    slice_index const next = slices[peer].u.branch_d_defender.next;
    assert(slices[peer].type==STBranchDirectDefender);

    if (2*max_len_threat+slack_length_direct
        <slices[root_slice].u.branch_d.min_length)
    {
      VerifieMsg(ThreatOptionAndExactStipulationIncompatible);
      return false;
    }

    if (slices[root_slice].u.branch_d.length<=max_len_threat)
      max_len_threat = maxply;

    if (slices[root_slice].u.branch_d.length<1
        && max_nr_refutations>0
        && !(slices[next].type==STLeafSelf
             || slices[next].type==STLeafHelp))
    {
      ErrorMsg(TryInLessTwo);
      max_nr_refutations = 0;
    }

    if (OptFlag[stoponshort])
    {
      ErrorMsg(NoStopOnShortSolutions);
      OptFlag[stoponshort]= false;
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
    if (stip_ends_in(pieceWinGoals,nrPieceWinGoals))
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

  if ((bl_royal_sq!=initsquare || wh_royal_sq!=initsquare
       || CondFlag[republican]
       || CondFlag[white_oscillatingKs] || CondFlag[black_oscillatingKs]
       || rex_circe
       || rex_immun)
      && (CondFlag[dynasty] || CondFlag[losingchess]))
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

  for (bnp= boardnum; *bnp; bnp++)
  {
    p= e[*bnp];
    if (p != vide)
    {
      if (CondFlag[volage] && rb != *bnp && rn != *bnp)
        SETFLAG(spec[*bnp], Volage);

      if ((PieSpExFlags >> DiaCirce) || flagdiastip || flag_magic)
        SetDiaRen(spec[*bnp], *bnp);

      if (TSTFLAG(spec[*bnp], ColourChange))
      {
        if (!is_simplehopper(abs(e[*bnp])))
        {
          /* relies on imitators already having been implemented */
          CLRFLAG(spec[*bnp], ColourChange);
          ErrorMsg(ColourChangeRestricted);
        }
      }
      /* known limitation: will print rK rather than just K as usual */
      if (abs(e[*bnp]) == King && (CondFlag[protean] || flag_magic)) 
        SETFLAG(spec[*bnp], Royal);
    }
  }

  flagleofamilyonly= CondFlag[leofamily] ? true : false;
  for (p= fb + 1; p <= derbla; p++)
  {
    if (exist[p] || promonly[p])
    {
      flagfee= true;
      if (is_rider(p))
        flagriders= true;
      else if (is_leaper(p))
        flagleapers= true;
      else if (is_simplehopper(p))
        flagsimplehoppers= true;
      else if (is_simpledecomposedleaper(p))
        flagsimpledecomposedleapers= true;
      else if (is_symmetricfairy(p))
        flagsymmetricfairy= true;
      else {
        if (!is_pawn(p) && p != dummyb && (p<leob || p>vaob))
          flagleofamilyonly= false;
        flagveryfairy= true;
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

  optim_orthomatingmoves = true;

  /* otherwise, the optimisation would be correct, too, but we
   * wouldn't care */
  optim_neutralretractable = TSTFLAG(PieSpExFlags,Neutral);

  if (CondFlag[sting])
  {
    optim_neutralretractable = false;
    optim_orthomatingmoves = false;
    flagfee= true;
    flagsimplehoppers= true;
  }

  if (!CondFlag[noiprom])
    for (n= 0; n <= maxply; n++)
      Iprom[n]= false;

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
    optim_orthomatingmoves = false;
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
    flagfee= true;

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
      optim_orthomatingmoves = false;
    }
  }

  {
    Side const restricted_side = (slices[root_slice].type==STBranchHelp
                                  ? slice_get_starter(root_slice)
                                  : advers(slice_get_starter(root_slice)));
    if (flagmaxi)
    {
      if (restricted_side==Black)
      {
        CondFlag[blmax] = true;
        CondFlag[whmax] = false;
        bl_ultra= CondFlag[ultra];
        bl_exact= CondFlag[exact];
        black_length= len_max;
        flagblackmummer= true;
        flagwhitemummer= false;
      }
      else
      {
        CondFlag[blmax] = false;
        CondFlag[whmax] = true;
        wh_ultra= CondFlag[ultra];
        wh_exact= CondFlag[exact];
        white_length= len_max;
        flagwhitemummer= true;
        flagblackmummer= false;
      }
    }
  
    if (flagultraschachzwang)
    {
      CondFlag[blackultraschachzwang]= restricted_side==Black;
      CondFlag[whiteultraschachzwang]= restricted_side==White;
      optim_neutralretractable = false;
      optim_orthomatingmoves = false;
    }
  }

  if (CondFlag[cavaliermajeur])
  {
    if (nbpiece[cb] + nbpiece[cn] > 0)
    {
      VerifieMsg(CavMajAndKnight);
      return false;
    }
    flagfee= true;
  }

  if (CondFlag[republican])
  {
    if (CondFlag[masand])
    {
      /* TODO what else should we prohibit here? */
      VerifieMsg(NoRepublicanWithConditionsDependingOnCheck);
      return false;
    }

    OptFlag[sansrn]= true;
    OptFlag[sansrb]= true;
    optim_neutralretractable = false;
    optim_orthomatingmoves = false;
    is_republican_suspended = false;
  }

  if (OptFlag[sansrb] && rb!=initsquare)
    OptFlag[sansrb]= false;

  if (OptFlag[sansrn] && rn!=initsquare)
    OptFlag[sansrn]= false;

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
    flag_nk= rb!=initsquare && TSTFLAG(spec[rb],Neutral);

  if (CondFlag[bicolores])
    if (TSTFLAG(PieSpExFlags, Neutral))
    {
      VerifieMsg(NeutralAndBicolor);
      return false;
    }

  if (CondFlag[bichro] || CondFlag[monochro])
    optim_orthomatingmoves = false;
  
  eval_2= eval_white= eval_ortho;
  rbechec = &orig_rbechec;
  rnechec = &orig_rnechec;

  flaglegalsquare= flaglegalsquare
      || CondFlag[bichro]
      || CondFlag[monochro];

  if (flaglegalsquare)
  {
    eval_white= legalsquare;
    eval_2= eval_ortho;
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

  if (TSTFLAG(PieSpExFlags,Paralyse)
      && !(CondFlag[patrouille]
           || CondFlag[beamten]
           || TSTFLAG(PieSpExFlags, Beamtet)))
  {
    eval_2= eval_white;
    eval_white= paraechecc;
  }

  if (TSTFLAG(PieSpExFlags, Kamikaze))
  {
    optim_neutralretractable = optim_orthomatingmoves = false;
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

  if ((CondFlag[supercirce] || CondFlag[april])
      && (CondFlag[koeko] || CondFlag[newkoeko] || CondFlag[antikoeko]))
  {
    VerifieMsg(SuperCirceAndOthers);
    return false;
  }

  {
    int numsuper=0;
    if (CondFlag[supercirce]) numsuper++;
    if (CondFlag[april]) numsuper++;
    if (CondFlag[republican]) numsuper++;
    if (CondFlag[antisuper]) numsuper++;
    if (numsuper > 1)
    {
      VerifieMsg(SuperCirceAndOthers);
      return false;
    }
  }

  if (CondFlag[patrouille]
      || CondFlag[beamten]
      || CondFlag[central]
      || TSTFLAG(PieSpExFlags, Beamtet)
      || CondFlag[ultrapatrouille])
  {
    eval_2= eval_white;
    eval_white= soutenu;
  }
  if (CondFlag[lortap])
  {
    eval_2= eval_white;
    eval_white= notsoutenu;
  }

  if (CondFlag[disparate])
  {
    eval_white= eval_black= eval_disp;
    optim_orthomatingmoves= false;
  }

  if (CondFlag[nowhcapture] && CondFlag[noblcapture])
  {
    CondFlag[nocapture]= true;
    if (CondFlag[nocapture])
    {
      CondFlag[nowhcapture]= false;
      CondFlag[noblcapture]= false;
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
      CondFlag[black_oscillatingKs]= CondFlag[white_oscillatingKs]= false;
    else
    {
      optim_neutralretractable = false;
      optim_orthomatingmoves = false;
    }
  }
  if (CondFlag[black_oscillatingKs] && OscillatingKingsTypeC[White]
      && CondFlag[white_oscillatingKs] && OscillatingKingsTypeC[White])
    CondFlag[swappingkings]= true;

  if (anymars||anyantimars) {
    optim_neutralretractable = optim_orthomatingmoves = false;
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
    eval_white= eval_BGL;
    BGL_whiteinfinity= BGL_white == BGL_infinity;
    BGL_blackinfinity= BGL_black == BGL_infinity;
    optim_neutralretractable = false;
    optim_orthomatingmoves = false;
  }

  if (flag_madrasi || CondFlag[isardam])
  {
    if ( CondFlag[imitators]
         || TSTFLAG(PieSpExFlags,Paralyse))
    {
      VerifieMsg(MadrasiParaAndOthers);
      return false;
    }
    if (!(CondFlag[patrouille]
          || CondFlag[beamten]
          || TSTFLAG(PieSpExFlags, Beamtet)))
    {
      eval_2= eval_white;
      eval_white= CondFlag[isardam]
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
    optim_orthomatingmoves = false;
    eval_2= eval_white;
    eval_white= eval_wooheff;
  }

  if (CondFlag[brunner])
    eval_white=eval_isardam;

  if (CondFlag[isardam] && IsardamB)
    eval_white=eval_ortho;

  if (CondFlag[shieldedkings])
    eval_white=eval_shielded;


  if (flagAssassin) {
    if (TSTFLAG(PieSpExFlags,Neutral) /* Neutrals not implemented */
        || CondFlag[bicolores])             /* others? */
    {
      VerifieMsg(AssassinandOthers);
      return false;
    }
  }
  eval_black= eval_white;
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
    eval_2= eval_white;
    if (rex_circe) {
      eval_white= rbcircech;
      eval_black= rncircech;
      cirrenroib= (*circerenai)(nbply, roib, spec[rb], initsquare, initsquare, initsquare, Black);
      cirrenroin= (*circerenai)(nbply, roin, spec[rn], initsquare, initsquare, initsquare, White);
    }
    else {
      eval_white= rbimmunech;
      eval_black= rnimmunech;
      immrenroib= (*immunrenai)(nbply, roib, spec[rb], initsquare, initsquare, initsquare, Black);
      immrenroin= (*immunrenai)(nbply, roin, spec[rn], initsquare, initsquare, initsquare, White);
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
    optim_orthomatingmoves = false;
    eval_2= eval_white;
    eval_white= rbanticircech;
    eval_black= rnanticircech;
  }

  if ((CondFlag[singlebox]  && SingleBoxType==singlebox_type1)) {
    if (flagfee)
    {
      VerifieMsg(SingleBoxAndFairyPieces);
      return false;
    }
    optim_neutralretractable = false;
    optim_orthomatingmoves = false;
    eval_2= eval_white;
    eval_white= rbsingleboxtype1ech;
    eval_black= rnsingleboxtype1ech;
  }

  if ((CondFlag[singlebox]  && SingleBoxType==singlebox_type3)) {
    optim_neutralretractable = optim_orthomatingmoves = false;
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
    eval_2= eval_white;
    eval_black= rnultraech;
    if (TSTFLAG(PieSpExFlags, Neutral))
    {
      VerifieMsg(OthersNeutral);
      return false;
    }
  }
  if (bl_ultra && !CondFlag[blcapt]) {
    eval_2= eval_white;
    eval_white= rbultraech;
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
#if !defined(DATABASE)  /* TLi */
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
#endif

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

  jouegenre =
      CondFlag[black_oscillatingKs]
      || CondFlag[white_oscillatingKs]
      || CondFlag[republican]
      || anycirce
      || CondFlag[sentinelles]
      || anyanticirce
      || CondFlag[singlebox]
      || CondFlag[blroyalsq]
      || CondFlag[whroyalsq]
      || CondFlag[dynasty]
      || CondFlag[strictSAT]
      || CondFlag[masand]
      || CondFlag[BGL]
      || CondFlag[duellist]
      || TSTFLAG(PieSpExFlags,HalfNeutral)
      || exist[Orphan]
      || exist[Friend]
      || calc_whrefl_king || calc_blrefl_king
      || CondFlag[phantom]
      || CondFlag[extinction]
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
      || CondFlag[patrouille]
      || CondFlag[lortap]
      || CondFlag[ultrapatrouille]
      || CondFlag[imitators]
      || CondFlag[beamten]
      || TSTFLAG(PieSpExFlags, Beamtet)
      || CondFlag[central]
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

    for (bnp= boardnum; *bnp; bnp++)
    {
      s = *bnp;
      if (abs(e[s])==roib && TSTFLAG(spec[s],Kamikaze))
      {
        empilegenre= true;
        break;
      }
    }
  }

  nonkilgenre= CondFlag[messigny]
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
      || CondFlag[dynasty]
      || flag_magic)
  {
    optim_neutralretractable = false;
    optim_orthomatingmoves = false;
  }

  superbas= CondFlag[antisuper] ? square_a1 : square_a1 - 1;

  /* init promotioncounter and checkcounter */
  pp= 0;
  cp= 0;
  for (p= CondFlag[losingchess] || CondFlag[dynasty] ? roib : db;
       p <= derbla;
       p++)
  {
    getprompiece[p]= vide;

    if (exist[p])
    {
      if ( p != pb
           && p != dummyb
           && p != pbb
           && p != bspawnb
           && p != spawnb
           && p != reversepb
           && (!CondFlag[promotiononly] || promonly[p]))
      {
        getprompiece[pp]= p;
        pp= p;
      }
      if (p > fb && p != dummyb) {
        /* only fairy pieces until now ! */
        optim_neutralretractable = false;
        optim_orthomatingmoves = false;
        if (p != hamstb)
        {
          checkpieces[cp]= p;
          cp++;
        }
      }
    }
  }
  checkpieces[cp]= vide;

  tp= 0;
  op= 0;
  for (p= roib; p <= derbla; p++) {
    if (exist[p] && p != dummyb && p != hamstb)
    {
      if (whitenormaltranspieces)
        whitetransmpieces[tp]= p;
      if (blacknormaltranspieces)
        blacktransmpieces[tp]= p;
      tp++;
      if (p != Orphan
          && p != Friend
          && (exist[Orphan] || exist[Friend]))
        orphanpieces[op++]= p;
    }
  }

  if (whitenormaltranspieces)
    whitetransmpieces[tp]= vide;
  if (blacknormaltranspieces)
    blacktransmpieces[tp]= vide;

  if (calc_whrefl_king || calc_blrefl_king)
  {
    optim_neutralretractable = false;
    optim_orthomatingmoves = false;
  }
  orphanpieces[op]= vide;

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

  for (n= 2; n <= maxply; n++)
    inum[n]= inum[1];

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
    optim_orthomatingmoves  = false;
    jouegenre = true;
  }
  checkhopim |= CondFlag[imitators];

  if (CondFlag[sentinelles])
  {
    /* a nasty drawback */
    optim_neutralretractable = false;
    optim_orthomatingmoves  = false;
  }

  if (CondFlag[annan])
  {
    optim_neutralretractable = false;
    optim_orthomatingmoves = false;
    rbechec= &annan_rbechec;
    rnechec= &annan_rnechec;
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
    
    if (stip_ends_in(incompatibleGoals,nrIncompatibleGoals))
    {
      VerifieMsg(LosingChessNotInCheckOrMateStipulations);
      return false;
    }

    /* no king is ever in check */
    rbechec= &losingchess_rbnechec;
    rnechec= &losingchess_rbnechec;

    /* capturing moves are "longer" than non-capturing moves */
    black_length= &len_losingchess;
    white_length= &len_losingchess;
    flagwhitemummer= true;
    flagblackmummer= true;
  }

  {
    Goal const moveOrientatedGoals[] =
    {
      goal_target,
      goal_ep,
      goal_capture,
      goal_steingewinn,
      goal_castling
    };
    size_t const nrMoveOrientatedGoals
        = sizeof moveOrientatedGoals / sizeof moveOrientatedGoals[0];
    
    FlagMoveOrientatedStip = stip_ends_only_in(moveOrientatedGoals,
                                               nrMoveOrientatedGoals);
  }

  /* check castling possibilities */
  CLEARFL(castling_flag[0]);
  /* castling_supported has to be adjusted if there are any problems */
  /* with castling and fairy conditions/pieces */
  castling_supported= !(
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

  complex_castling_through_flag= CondFlag[imitators];

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

  if (find_next_goal(goal_castling,root_slice)!=no_slice
      && !castling_supported)
  {
    VerifieMsg(StipNotSupported);
    return false;
  }

  castling_flag[0] &= no_castling;
  castling_flag[2]= castling_flag[1]= castling_flag[0];
  /* At which ply do we begin ??  NG */

  testcastling=
      TSTFLAGMASK(castling_flag[0],whq_castling&no_castling)==whq_castling
      || TSTFLAGMASK(castling_flag[0],whk_castling&no_castling)==whk_castling
      || TSTFLAGMASK(castling_flag[0],blq_castling&no_castling)==blq_castling
      || TSTFLAGMASK(castling_flag[0],blk_castling&no_castling)==blk_castling;

  /* a small hack to enable ep keys */
  trait[1]= 2;

  /* we have to know which goal has to be reached in a dual-free
   * way */
  if (CondFlag[exclusive] && find_unique_goal()==no_slice)
  {
    VerifieMsg(StipNotSupported);
    return false;
  }

  if (CondFlag[isardam]
      || CondFlag[ohneschach]
      || CondFlag[exclusive])
  {
    flag_testlegality= true;
    optim_neutralretractable = false;
    optim_orthomatingmoves = false;
  }

  if (!CondFlag[patience]) {           /* needed because of twinning */
    PatienceB= false;
  }

  jouetestgenre=
      flag_testlegality
      || flagAssassin
      || find_next_goal(goal_doublemate,root_slice)!=no_slice
      || CondFlag[patience]
      || CondFlag[republican]
      || CondFlag[blackultraschachzwang]
      || CondFlag[whiteultraschachzwang]
      || CondFlag[BGL];
  jouetestgenre_save= jouetestgenre;

  jouetestgenre1 = CondFlag[blackultraschachzwang]
      || CondFlag[whiteultraschachzwang];      


  nonoptgenre= TSTFLAG(PieSpExFlags, Neutral)
      || flag_testlegality
      || anymars
      || anyantimars
      || CondFlag[brunner]
      || CondFlag[blsupertrans_king]
      || CondFlag[whsupertrans_king]
      || CondFlag[republican]
      || CondFlag[takemake];

  supergenre=
      CondFlag[supercirce]
      || CondFlag[antisuper]
      || CondFlag[april]
      || CondFlag[republican];

  if (CondFlag[extinction] || flagAssassin)
  {
    optim_neutralretractable = false;
    optim_orthomatingmoves = false;
  }

  if (CondFlag[actrevolving] || CondFlag[arc])
  {
    jouegenre= true;
    optim_neutralretractable = false;
    optim_orthomatingmoves = false;
  }

  if (anytraitor) {
    optim_neutralretractable = false;
    optim_orthomatingmoves = false;
  }

  if (InitChamCirce)
  {
    if (CondFlag[leofamily]) {
      NextChamCircePiece[Leo]= Mao;
      NextChamCircePiece[Pao]= Leo;
      NextChamCircePiece[Vao]= Pao;
      NextChamCircePiece[Mao]= Vao;
    }
    else {
      piece actknight= CondFlag[cavaliermajeur] ? NightRider : Knight;
      NextChamCircePiece[actknight]= Bishop;
      NextChamCircePiece[Bishop]= Rook;
      NextChamCircePiece[Rook]= Queen;
      NextChamCircePiece[Queen]= actknight;
    }
  }

  RB_[1]= rb;
  RN_[1]= rn;

  if (CondFlag[SAT] || CondFlag[strictSAT])
  {
    SATCheck= true;
    optim_neutralretractable = false;
    optim_orthomatingmoves = false;
    nonoptgenre= true;
    WhiteStrictSAT[1]= echecc_normal(nbply,White);
    BlackStrictSAT[1]= echecc_normal(nbply,Black);
    satXY= WhiteSATFlights > 1 || BlackSATFlights > 1;
  }

  if (CondFlag[schwarzschacher])
  {
    optim_neutralretractable = false;
    optim_orthomatingmoves = false;
    nonoptgenre= true;
  }

  move_generation_mode_opti_per_side[White]
      = flagwhitemummer||nonoptgenre
      ? move_generation_optimized_by_killer_move
      : move_generation_optimized_by_nr_opponent_moves;
  move_generation_mode_opti_per_side[Black]
      = flagblackmummer||nonoptgenre
      ? move_generation_optimized_by_killer_move
      : move_generation_optimized_by_nr_opponent_moves;

  if (CondFlag[takemake])
  {
    optim_neutralretractable = false;
    optim_orthomatingmoves = false;
  }

  if (CondFlag[protean])
  {
    flagfee= true;
    optim_neutralretractable = false;
    optim_orthomatingmoves = false;
  }

  switch (stip_supports_intelligent())
  {
    case intelligent_not_supported:
      if (OptFlag[intelligent])
      {
        VerifieMsg(IntelligentRestricted);
        return false;
      }
      else
        isIntelligentModeActive = false;
      break;

    case intelligent_not_active_by_default:
      isIntelligentModeActive = OptFlag[intelligent];
      break;

    case intelligent_active_by_default:
      isIntelligentModeActive = true;
      break;
  }

  if (CondFlag[castlingchess])
  {
    optim_neutralretractable = false;
    optim_orthomatingmoves = false;
    castling_supported= false;
    jouegenre= true;
  }

#if !defined(DATABASE)
  if (stip_ends_in(proof_goals,nr_proof_goals))
    return ProofVerifie();
#endif
    
  return true;
}

boolean moves_equal(coup *move1, coup *move2)
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
              || ((!(CondFlag[supercirce] || CondFlag[april])
                   || move1->sqren==move2->sqren)
                  && (!CondFlag[republican]
                      || move1->repub_k==move2->repub_k)
                  && (!CondFlag[antisuper]
                      || move1->renkam==move2->renkam))
              )
          );
}

void current(ply ply_id, coup *mov)
{
  numecoup const coup_id = ply_id==nbply ? nbcou : repere[ply_id+1];
  square sq = move_generation_stack[coup_id].arrival;

  mov->tr=          trait[ply_id];
  mov->cdzz =           move_generation_stack[coup_id].departure;
  mov->cazz=            sq;
  mov->cpzz=            move_generation_stack[coup_id].capture;
  mov->pjzz=            pjoue[ply_id];
  mov->norm_prom=       norm_prom[ply_id];
  mov->ppri=            pprise[ply_id];
  mov->sqren=           sqrenais[ply_id];
  mov->cir_prom=        cir_prom[ply_id];

  mov->echec= echecc(ply_id,advers(mov->tr));

  mov->renkam= crenkam[ply_id];
  mov->promi=  Iprom[ply_id];
  mov->numi=     inum[ply_id] - (mov->promi ? 1 : 0);
  /* Promoted imitator will be output 'normally'
     from the next move on. */
  mov->sum= isquare[0] - im0;
  mov->speci= jouespec[ply_id];

  /* hope the following works with parrain too */
  mov->ren_spec=  spec[sqrenais[ply_id]];
  mov->bool_senti= senti[ply_id];
  mov->ren_parrain= ren_parrain[ply_id];
  mov->bool_norm_cham_prom= norm_cham_prom[ply_id];
  mov->bool_cir_cham_prom= cir_cham_prom[ply_id];
  mov->pjazz=     jouearr[ply_id];
  mov->repub_k=   repub_k[ply_id];
  mov->new_spec=  spec[sq];
  mov->hurdle=    chop[coup_id];
  mov->sb3where=  sb3[coup_id].where;
  mov->sb3what= sb3[coup_id].what;
  if (mov->sb3what!=vide && mov->sb3where==mov->cdzz)
    mov->pjzz= mov->pjazz= mov->sb3what;

  mov->sb2where= sb2[ply_id].where;
  mov->sb2what= sb2[ply_id].what;
  mov->mren= cmren[coup_id];
  mov->osc= oscillatedKs[ply_id];
  /* following only overwritten if change stack is saved in
   * append_to_top_table() */
  /* redundant to init push_top */
  mov->push_bottom= NULL;
  mov->roch_sq=rochade_sq[coup_id];
  mov->roch_pc=rochade_pc[coup_id];
  mov->roch_sp=rochade_sp[coup_id];

  mov->ghost_piece = e[mov->cdzz];
  mov->ghost_flags = spec[mov->cdzz];
}

boolean WriteSpec(Flags sp, boolean printcolours) {
  boolean ret= false;
  PieSpec spname;

  if (printcolours && !TSTFLAG(sp, Neutral))
    spname= White;
  else
    spname= Neutral;

  for (; spname < PieSpCount; spname++) {
    if ( (spname != Volage || !CondFlag[volage])
         && TSTFLAG(sp, spname))
    {
      StdChar(tolower(*PieSpString[UserLanguage][spname]));
      ret= true;
    }
  }
  return ret;
}

#if defined(DATABASE)
extern boolean two_same_pieces;
#endif

#if !defined(DATABASE)

/* Determine whether the defending side has more flights than allowed
 * by the user.
 * @param defender defending side
 * @return true iff the defending side has too many flights.
 */
boolean has_too_many_flights(Side defender)
{
  square save_rbn = defender==Black ? rn : rb;
  if (save_rbn==initsquare)
    return false;
  else
  {
    int nrflleft = max_nr_flights+1;
    genmove(defender);
    while (encore() && nrflleft>0)
    {
      if (jouecoup(nbply,first_play))
      {
        square const rbn = defender==Black ? rn : rb;
        if (save_rbn!=rbn && !echecc(nbply,defender))
          nrflleft--;
      }
      repcoup();
    }
    finply();

    return nrflleft==0;
  }
}

static void swapcolors(void) {
  square *bnp;
  for (bnp= boardnum; *bnp; bnp++)
  {
    if (!TSTFLAG(spec[*bnp], Neutral) && e[*bnp] != vide)
    {
      e[*bnp]= -e[*bnp];
      spec[*bnp]^= BIT(White)+BIT(Black);
    }
  }
}

static void reflectboard(void) {
  square *bnp;
  for (bnp= boardnum; *bnp < (square_a1+square_h8)/2; bnp++)
  {
    square sq2= *bnp%onerow+onerow*((onerow-1)-*bnp/onerow);

    piece p= e[sq2];
    Flags sp= spec[sq2];

    e[sq2]= e[*bnp];
    spec[sq2]= spec[*bnp];

    e[*bnp]= p;
    spec[*bnp]= sp;
  }

  isBoardReflected = !isBoardReflected;
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
  init_output();

  if (isIntelligentModeActive && OptFlag[restart])
  {
    /* In intelligent mode, RestartNbr means the minimal number of
     * moves.
     */
    stip_length_type const save_min_length = set_min_length(root_slice,
                                                            RestartNbr);
    OptFlag[restart] = false;
    slice_root_solve(root_slice);
    OptFlag[restart] = true;
    set_min_length(root_slice,save_min_length);
  }
  else
    slice_root_solve(root_slice);

  closehash();

  Message(NewLine);
}

static boolean initialise_position(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceText("\n");

  if (stip_ends_in(proof_goals,nr_proof_goals))
  {
    countPieces();
    result = locateRoyal();
    if (result)
    {
      ProofSaveTargetPosition();
      ProofRestoreStartPosition();
      countPieces();
      result = locateRoyal();
      ProofAtoBSaveStartRoyal();
    }
  }
  else
  {
    countPieces();
    result = locateRoyal();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

typedef enum
{
  dont_know_meaning_of_whitetoplay,
  whitetoplay_means_change_colors,
  whitetoplay_means_shorten_root_slice
} meaning_of_whitetoplay;

static meaning_of_whitetoplay detect_meaning_of_whitetoplay(slice_index si)
{
  meaning_of_whitetoplay result = dont_know_meaning_of_whitetoplay;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u\n",si);

  switch (slices[si].type)
  {
    case STLeafHelp:
      if (slices[si].u.leaf.goal==goal_atob)
        result = whitetoplay_means_change_colors;
      break;

    case STBranchHelp:
    {
      slice_index const next = slices[si].u.branch.next;
      meaning_of_whitetoplay const next_result =
          detect_meaning_of_whitetoplay(next);
      if (next_result==dont_know_meaning_of_whitetoplay)
        result = whitetoplay_means_shorten_root_slice;
      else
        result = next_result;
      break;
    }

    case STMoveInverter:
    {
      slice_index const next = slices[si].u.move_inverter.next;
      result = detect_meaning_of_whitetoplay(next);
      break;
    }

    default:
      /* nothing */
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
}

static void shorten_root_branch_h_slice(void)
{
  TraceFunctionEntry(__func__);
  TraceText("\n");

  --slices[root_slice].u.branch.length;
  --slices[root_slice].u.branch.min_length;
  if (slices[root_slice].u.branch.min_length<slack_length_help)
    slices[root_slice].u.branch.min_length += 2;
  TraceValue("->%u",slices[root_slice].u.branch.length);
  TraceValue("->%u\n",slices[root_slice].u.branch.min_length);

  if (slices[root_slice].u.branch.length==slack_length_help
      && slices[root_slice].u.branch.min_length==slack_length_help)
  {
    slice_index const save_root_slice = root_slice;
    root_slice = slices[root_slice].u.move_inverter.next;
    dealloc_slice_index(save_root_slice);
  }
  
  TraceFunctionExit(__func__);
  TraceText("\n");
}

/* Apply the option White to play
 * @return true iff the option is applicable (and was applied)
 */
static boolean root_slice_apply_whitetoplay(void)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceText("\n");

  TraceValue("%u\n",slices[root_slice].type);
  switch (slices[root_slice].type)
  {
    case STBranchHelp:
    {
      meaning_of_whitetoplay const
          meaning = detect_meaning_of_whitetoplay(root_slice);
      if (meaning==whitetoplay_means_shorten_root_slice)
        shorten_root_branch_h_slice();
      slice_impose_starter(root_slice,
                           advers(slices[root_slice].u.branch.starter));
      if (meaning==whitetoplay_means_shorten_root_slice)
        root_slice = alloc_move_inverter_slice(root_slice);
      result = true;
      break;
    }

    case STLeafHelp:
      slice_impose_starter(root_slice,
                           advers(slices[root_slice].u.branch.starter));
      result = true;
      break;

    case STMoveInverter:
    {
      meaning_of_whitetoplay const
          meaning = detect_meaning_of_whitetoplay(root_slice);
      slice_index const save_root_slice = root_slice;
      root_slice = slices[root_slice].u.move_inverter.next;
      dealloc_slice_index(save_root_slice);
      if (meaning==whitetoplay_means_shorten_root_slice
          && slices[root_slice].type==STBranchHelp)
      {
        shorten_root_branch_h_slice();
        return true;
      }
      break;
    }

    default:
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
  return result;
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
      idx++;
      MaxTime = strtol(argv[idx], &end, 10);
      if (argv[idx]==end)
      {
        /* conversion failure -> assume no max time */
        MaxTime = UINT_MAX;
      }
      idx++;
      continue;
    }
    else if (idx+1<argc && strcmp(argv[idx], "-maxmem")==0)
    {
      char *end;
      idx++;
      MaxMemory = strtoul(argv[idx], &end, 10);
      if (argv[idx]==end)
      {
        /* conversion failure
         * -> set to 0 now and to default value further down */
        MaxMemory = 0;
      }
      else if (*end=='G')
      {
        MaxMemory <<= 30;
        MaxMemory_unit = maxmemory_giga;
      }
      else if (*end=='M')
      {
        MaxMemory <<= 20;
        MaxMemory_unit = maxmemory_mega;
      }
      else
      {
        MaxMemory <<= 10;
        MaxMemory_unit = maxmemory_kilo;
      }

      idx++;
      continue;
    }
    else if (strcmp(argv[idx], "-regression")==0)
    {
      flag_regression = true;
      idx++;
      continue;
    }
    else if (strcmp(argv[idx], "-notrace")==0)
    {
      TraceDeactivate();
      idx++;
      continue;
    }
    else
      break;
  }

  return idx;
}

static void initMaxMemoryString(void)
{
  /* We do not issue our startup message via the language
     dependant Msg-Tables, since there the version is
     too easily changed, or not updated.
  */
  if ((MaxMemory>>10)<(1<<10) || MaxMemory_unit==maxmemory_kilo)
    sprintf(MaxMemoryString, " (%u KB)\n", (unsigned)(MaxMemory>>10));
  else if ((MaxMemory>>20)<(1<<10) || MaxMemory_unit==maxmemory_mega)
    sprintf(MaxMemoryString, " (%u MB)\n", (unsigned)(MaxMemory>>20));
  else
    sprintf(MaxMemoryString, " (%u GB)\n", (unsigned)(MaxMemory>>30));
}

/* prepare for solving duplex */
static void init_duplex(void)
{
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
    Side const starter = slice_get_starter(root_slice);
    slice_impose_starter(root_slice,advers(starter));
  }
}

/* restore from preparations for solving duplex */
static void fini_duplex(void)
{
  if (isIntelligentModeActive)
  {
    /* cf. init_duplex */
    reflectboard();
    swapcolors();
  }
  else
  {
    Side const starter = slice_get_starter(root_slice);
    slice_impose_starter(root_slice,advers(starter));
  }
}

static boolean root_slice_apply_setplay(void)
{
  boolean result;
  slice_index setplay;

  TraceFunctionEntry(__func__);
  TraceText("\n");

  setplay = slice_root_make_setplay_slice(root_slice);
  if (setplay==no_slice)
    result = false;
  else
  {
    slice_index const mi = alloc_move_inverter_slice(setplay);
    root_slice = alloc_quodlibet_slice(mi,root_slice);
    TraceValue("->%u\n",root_slice);
    result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u\n",result);
  return result;
}

static boolean root_slice_apply_postkeyplay(void)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceText("\n");

  if (slices[root_slice].type==STBranchDirect)
  {
    slice_index const root_peer = slices[root_slice].u.branch_d.peer;
    root_slice = alloc_move_inverter_slice(root_peer);
    result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u\n",result);
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
  slice_index const leaf_unique_goal = find_unique_goal();
  Goal const unique_goal = (leaf_unique_goal==no_slice
                            ? no_goal
                            : slices[leaf_unique_goal].u.leaf.goal);

  if (unique_goal==goal_proof || unique_goal==goal_atob)
    ProofSetGoal(unique_goal);

  if (initialise_position() && verify_position())
  {
    if (!OptFlag[noboard])
    {
      if (stip_ends_in(proof_goals,nr_proof_goals))
        ProofWritePosition();
      else
        WritePosition();
    }

    if (twin_index==0)
    {
      if (LaTeXout)
        LaTeXBeginDiagram();

      if (end_of_twin_token==TwinProblem)
        StdString("a)\n\n");
    }

    /* allow line-oriented output to restore the initial position */
    StorePosition();
    solveHalfADuplex();

    if (OptFlag[duplex])
    {
      /* Set next side to calculate for duplex "twin" */
      if ((OptFlag[maxsols] && solutions>=maxsolutions)
          || (OptFlag[stoponshort] && FlagShortSolsReached))
        FlagMaxSolsReached = true;

      /* restart calculation of maxsolution after half-duplex */
      solutions = 0;
      FlagShortSolsReached = false;

      init_duplex();

      if (locateRoyal() && verify_position())
        solveHalfADuplex();

      fini_duplex();
    }
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
  TraceFunctionParam("%u\n",prev_token);

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

    /* Set maximal solving time if the user asks for it on the
     * command line or as an option.
     * If a maximal time is indicated both on the command line and as
     * an option, use the smaller value.
     */
    if (OptFlag[maxtime] || MaxTime<UINT_MAX)
    {
      if (MaxTime<maxsolvingtime)
        maxsolvingtime = MaxTime;
    }
    else
      /* maxsolvingtime should already be ==UINT_MAX, but let's err on
       * the safe side */
      maxsolvingtime = UINT_MAX;
      
    setMaxtime(&maxsolvingtime);

    TraceValue("%u",twin_index);
    TraceValue("%u\n",shouldDetectStarter);
    if (twin_index==0 || shouldDetectStarter)
    {
      boolean const same_starter_as_root = true;

      slice_detect_starter(root_slice,same_starter_as_root);

      /* intelligent AND duplex means that the board is mirrored and
       * the colors swapped by swapcolors() and reflectboard() ->
       * start with the regular side. */
      if (OptFlag[halfduplex] && !isIntelligentModeActive)
      {
        Side const non_duplex_starter = slice_get_starter(root_slice);
        slice_impose_starter(root_slice,advers(non_duplex_starter));
      }

      hash_reset_derivations();
      
      if (OptFlag[whitetoplay] && !root_slice_apply_whitetoplay())
        Message(WhiteToPlayNotApplicable);

      if (OptFlag[postkeyplay] && !root_slice_apply_postkeyplay())
        Message(PostKeyPlayNotApplicable);

      if (OptFlag[solapparent] && !OptFlag[restart]
          && !root_slice_apply_setplay())
        Message(SetPlayNotApplicable);
    }

    if (slice_get_starter(root_slice)==no_side)
      VerifieMsg(CantDecideWhoIsAtTheMove);
    else
    {
      TraceValue("%u\n",slice_get_starter(root_slice));
      solve_twin(twin_index,prev_token);

      if ((OptFlag[maxsols] && solutions>=maxsolutions)
          || (OptFlag[stoponshort] && FlagShortSolsReached))
        FlagMaxSolsReached = true;

      /* restart calculation of maxsolution after twinning */
      solutions = 0;
      FlagShortSolsReached = false;
    }

    ++twin_index;
  } while (prev_token==TwinProblem);


  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",prev_token);
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
        || maxtime_status==MAXTIME_TIMEOUT)
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

/* Guess the "bitness" of the platform
 * @return 16 if we run on a 16bit platform, 32 if we run on a 32bit
 * platform etc.
 */
static unsigned int guessPlatformBitness(void)
{
  if (UINT_MAX < 1UL<<16)
    return 16;
  else if (UINT_MAX==ULONG_MAX)
    return 32;
  else
    return 64;
}

int main(int argc, char *argv[])
{
  int idx_end_of_options;

  checkGlobalAssumptions();

  set_nice_priority();

  sprintf(versionString,
          "Popeye %s-%uBit v%.2f",
          OSTYPE,guessPlatformBitness(),VERSION);
  
  MaxTime = UINT_MAX;
  MaxPositions = ULONG_MAX;
  MaxMemory = 0;
  MaxMemory_unit = maxmemory_kilo;
  LaTeXout = false;
  flag_regression = false;

  /* Initialize message table with default language.
   * This default setting is hopefully overriden later by ReadBeginSpec().
   */
  InitMsgTab(LanguageDefault);

  idx_end_of_options = parseCommandlineOptions(argc,argv);
  
  OpenInput(idx_end_of_options<argc ? argv[idx_end_of_options] : " ");

  initMaxtime();

  MaxMemory = adjustMaxmemory(MaxMemory);
  initMaxMemoryString();

  /* start timer to be able to display a reasonable time if the user
   * aborts execution before the timer is started for the first
   * problem */
  StartTimer();

  InitCheckDir();

  /* Don't use StdString() - possible trace file is not yet opened
   */
  pyfputs(versionString,stdout);
  pyfputs(MaxMemoryString,stdout);

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

#endif  /* ! DATABASE */
