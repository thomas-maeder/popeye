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
#include "platform/maxmem.h"
#include "platform/maxtime.h"
#include "platform/pytime.h"
#include "platform/priority.h"

boolean supergenre;

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

boolean is_simpledecomposedleaper(piece p)
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

boolean is_symmetricfairy(piece p)
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

boolean verifieposition(void)
{
  square        *bnp;
  piece     p;
  ply           n;
  int      cp, pp, tp, op;
  int           i;
  boolean          nonoptgenre;
  
  if (CondFlag[glasgow] && CondFlag[circemalefique])
    anycirprom= True;

  /* initialize promotion squares */
  if (!CondFlag[einstein]) {
    square i;
    if (!CondFlag[whprom_sq]) {
      for (i= 0; i < 8; i++)
        SETFLAG(sq_spec[CondFlag[glasgow] ? square_h7-i : square_h8-i],
                WhPromSq);
    }
    if (!CondFlag[blprom_sq]) {
      for (i= 0; i < 8; i++)
        SETFLAG(sq_spec[CondFlag[glasgow] ? square_a2+i : square_a1+i],
                BlPromSq);
    }
  }

  im0= isquare[0];
  if (! CondFlag[imitators])
    CondFlag[noiprom]= true;

  if ((droh < 0) || (enonce <= droh)) {
    droh= maxply;
  }

  zugebene= 0;
  if (FlowFlag(Alternate) && ! SortFlag(Proof)) {
    if (enonce<2 && max_nr_refutations>0 && !SortFlag(Self))
    {
      ErrorMsg(TryInLessTwo);
      max_nr_refutations = 0;
    }
    if ( OptFlag[stoponshort]
         && (SortFlag(Direct) || SortFlag(Self)))
    {
      ErrorMsg(NoStopOnShortSolutions);
      OptFlag[stoponshort]= false;
    }
    if (enonce > (maxply-1)/2)
    {
      VerifieMsg(BigNumMoves);
      return false;
    }
  }
  else {
    if (enonce >= maxply-2)
    {
      VerifieMsg(BigNumMoves);
      return false;
    }
  }

  optim_neutralretractable = optim_orthomatingmoves =
    ((stipSettings[nonreciprocal].stipulation == stip_mate)
     || (stipSettings[nonreciprocal].stipulation == stip_check)
     || (stipSettings[nonreciprocal].stipulation == stip_doublemate))
    && (!FlowFlag(Reci)
        || (stipSettings[reciprocal].stipulation == stip_mate)
        || (stipSettings[reciprocal].stipulation == stip_check)
        || (stipSettings[reciprocal].stipulation == stip_doublemate));

  if (stipSettings[nonreciprocal].stipulation == stip_steingewinn
      && CondFlag[parrain])
  {
    VerifieMsg(PercentAndParrain);
    return false;
  }

  flagdiastip=
    stipSettings[nonreciprocal].stipulation == stip_circuit
    || stipSettings[nonreciprocal].stipulation == stip_exchange
    || stipSettings[nonreciprocal].stipulation == stip_circuitB
    || stipSettings[nonreciprocal].stipulation == stip_exchangeB
    || stipSettings[reciprocal].stipulation == stip_circuit
    || stipSettings[reciprocal].stipulation == stip_exchange
    || stipSettings[reciprocal].stipulation == stip_circuitB
    || stipSettings[reciprocal].stipulation == stip_exchangeB;

  for (p= roib; p <= derbla; p++) {
    nbpiece[p]= 0;
    nbpiece[-p]= 0;
  }
  if (TSTFLAG(PieSpExFlags, HalfNeutral)) {
    SETFLAG(PieSpExFlags, Neutral);
  }

  for (i= fb + 1; i <= derbla; i++) {
    exist[i]= false;
  }

  for (i= roib; i <= fb; i++) {
    exist[i]= true;
  }

  if (CondFlag[sentinelles]) {
    exist[sentinelb]= exist[sentineln]= true;
  }

  for (bnp= boardnum; *bnp; bnp++) {
    p= e[*bnp];
    if (p != vide) {
      if (p < fn) {
        exist[-p]= true;
      }
      else if (p > fb) {
        exist[p]= true;
      }
      nbpiece[e[*bnp]]++;
    }
  }

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

  rb = initsquare;
  rn = initsquare;

  if (TSTFLAG(PieSpExFlags,Neutral))
    /* neutral king has to be white for initialisation of r[bn] */
    initneutre(blanc);

  if (CondFlag[dynasty]) {
    square *bnp;
    square s;

    OptFlag[sansrn]= True;
    OptFlag[sansrb]= True;

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
    OptFlag[sansrn]= True;
    OptFlag[sansrb]= True;
  }
  else
  {
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

  for (bnp= boardnum; *bnp; bnp++) {
    p= e[*bnp];
    if (p != vide) {
      if (CondFlag[volage] && rb != *bnp && rn != *bnp) {
        SETFLAG(spec[*bnp], Volage);
      }

      if ((PieSpExFlags >> DiaCirce) || flagdiastip || flag_magic) {
        SetDiaRen(spec[*bnp], *bnp);
      }
      if (TSTFLAG(spec[*bnp], ColourChange)) {
        if (!is_simplehopper(abs(e[*bnp]))) {
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
  for (p= fb + 1; p <= derbla; p++) {
    if (exist[p] || promonly[p]) {
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

  if (CondFlag[sting]) {
    optim_neutralretractable = optim_orthomatingmoves = false;
    flagfee= true;
    flagsimplehoppers= true;
  }

  if (!CondFlag[noiprom])
    for (n= 0; n <= maxply; Iprom[n++]= false)
      ;

  flaglegalsquare=
    TSTFLAG(PieSpExFlags, Jigger)
    || CondFlag[newkoeko]
    || CondFlag[gridchess] || CondFlag[koeko] || CondFlag[antikoeko]
    || CondFlag[blackedge] || CondFlag[whiteedge]
    || CondFlag[geneva];

  if (CondFlag[imitators]) {
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
    optim_neutralretractable = optim_orthomatingmoves = false;
  }

  if (CondFlag[leofamily]) {
    for (p= db; p <= fb; p++) {
      if (nbpiece[p] + nbpiece[-p] != 0)
      {
        VerifieMsg(LeoFamAndOrtho);
        return false;
      }
      exist[p]= false;
    }
  }

  if (CondFlag[chinoises]) {
    for (p= leob; p <= vaob; p++) {
      exist[p]= true;
    }
    flagfee= true;
  }
  if (anycirce) {
    if (exist[dummyb])
    {
      VerifieMsg(CirceAndDummy);
      return false;
    }
    if (TSTFLAG(PieSpExFlags, Neutral)) {
      optim_neutralretractable = optim_orthomatingmoves = false;
    }
  }

  if (flagmaxi) {
    CondFlag[blmax]= (maincamp==blanc);
    CondFlag[whmax] = (maincamp==noir);
    if (maincamp==blanc) {
      bl_ultra= CondFlag[ultra];
      bl_exact= CondFlag[exact];
      black_length= len_max;
      flagblackmummer= true;
      flagwhitemummer= false;
    }
    else {
      wh_ultra= CondFlag[ultra];
      wh_exact= CondFlag[exact];
      white_length= len_max;
      flagwhitemummer= true;
      flagblackmummer= false;
    }
  }
  
  if (flagultraschachzwang) {
    CondFlag[blackultraschachzwang]= (maincamp==blanc);
    CondFlag[whiteultraschachzwang]= (maincamp==noir);
  }
  if (CondFlag[blackultraschachzwang]
      || CondFlag[whiteultraschachzwang])
  {
    optim_neutralretractable = optim_orthomatingmoves = false;
  }

  if (CondFlag[cavaliermajeur]) {
    if (nbpiece[cb] + nbpiece[cn] > 0)
    {
      VerifieMsg(CavMajAndKnight);
      return false;
    }
    exist[cb]= false;
    exist[nb]= true;
    flagfee= true;
  }

  if (CondFlag[republican]) {
    if (CondFlag[masand])
    {
      /* TODO what else should we prohibit here? */
      VerifieMsg(NoRepublicanWithConditionsDependingOnCheck);
      return false;
    }

    OptFlag[sansrn]= True;
    OptFlag[sansrb]= True;
    optim_neutralretractable = False;
    optim_orthomatingmoves = False;
    flag_dontaddk=false;
  }

  if (OptFlag[sansrb] && rb!=initsquare) {
    OptFlag[sansrb]= False;
  }
  if (OptFlag[sansrn] && rn!=initsquare) {
    OptFlag[sansrn]= False;
  }
  if (rb==initsquare && nbpiece[roib]==0
      && !OptFlag[sansrb]) {
    ErrorMsg(MissingKing);
  }
  if (rn==initsquare && nbpiece[roin]==0
      && !OptFlag[sansrn]) {
    ErrorMsg(MissingKing);
  }

  if (rex_circe) {
    /* why not royal pieces in PWC ??? TLi */
    /* Because we loose track of the royal attribute somewhere and
       I didn't find where ... NG
    */
    if (CondFlag[circeequipollents]
        || CondFlag[circeclone]
        || CondFlag[couscous]
        || CondFlag[circeclonemalefique]) {
      /* disallowed because of the call to (*circerenai)
         in echecc */
      /* would require knowledge of id. Other forms now allowed
       */
      if (  ((! OptFlag[sansrb]) && rb!=initsquare && (e[rb] != roib))
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

  if (CondFlag[bicolores] || CondFlag[monochro])
    optim_orthomatingmoves = false;
  
  eval_2= eval_white= eval_ortho;
  rbechec = &orig_rbechec;
  rnechec = &orig_rnechec;

  flaglegalsquare= flaglegalsquare
    || CondFlag[bichro]
    || CondFlag[monochro];

  if (flaglegalsquare) {
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

  if (flaglegalsquare || TSTFLAG(PieSpExFlags,Neutral)) {
    if (CondFlag[volage])
    {
      VerifieMsg(SomeCondAndVolage);
      return false;
    }
  }
  if (TSTFLAG(PieSpExFlags,Paralyse)
      && !(CondFlag[patrouille]
           || CondFlag[beamten]
           || TSTFLAG(PieSpExFlags, Beamtet)))
  {
    eval_2= eval_white;
    eval_white= paraechecc;
  }
  if (TSTFLAG(PieSpExFlags, Kamikaze)) {
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

  if (CondFlag[nowhcapture] && CondFlag[noblcapture]) {
    CondFlag[nocapture]= true;
    if (CondFlag[nocapture]) {
      CondFlag[nowhcapture]= false;
      CondFlag[noblcapture]= false;
    }
  }

  if (CondFlag[isardam] && flag_madrasi)
  {
    VerifieMsg(IsardamAndMadrasi);
    return false;
  }

  if (CondFlag[black_oscillatingKs] || CondFlag[white_oscillatingKs]) {
    if (rb==initsquare || rn==initsquare)
      CondFlag[black_oscillatingKs]= CondFlag[white_oscillatingKs]= false;
    else
      optim_neutralretractable = optim_orthomatingmoves = false;
  }
  if (CondFlag[black_oscillatingKs] && OscillatingKingsTypeC[blanc] &&
      CondFlag[white_oscillatingKs] && OscillatingKingsTypeC[blanc]) {
    CondFlag[swappingkings]= True;
  }

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
    optim_neutralretractable = optim_orthomatingmoves = false;
  }

  if (flag_madrasi || CondFlag[isardam]) {
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
    optim_neutralretractable = optim_orthomatingmoves = false;
    eval_2= eval_white;
    eval_white= eval_wooheff;
  }

  if (CondFlag[brunner]) {
    eval_white=eval_isardam;
  }
  if (CondFlag[isardam] && IsardamB) {
    eval_white=eval_ortho;
  }
  if (CondFlag[shieldedkings]) {
    eval_white=eval_shielded;
  }


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
      cirrenroib= (*circerenai)(roib, spec[rb], initsquare, initsquare, initsquare, noir);
      cirrenroin= (*circerenai)(roin, spec[rn], initsquare, initsquare, initsquare, blanc);
    }
    else {
      eval_white= rbimmunech;
      eval_black= rnimmunech;
      immrenroib= (*immunrenai)(roib, spec[rb], initsquare, initsquare, initsquare, noir);
      immrenroin= (*immunrenai)(roin, spec[rn], initsquare, initsquare, initsquare, blanc);
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
    optim_neutralretractable = optim_orthomatingmoves = false;
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
    optim_neutralretractable = optim_orthomatingmoves = false;
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
    || TSTFLAG(PieSpExFlags, Magic);


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
    || anyanticirce;

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
    || CondFlag[losingchess];

  if (CondFlag[dynasty]) {
    /* checking for TSTFLAG(spec[rb],Kamikaze) may not be sufficient
     * in dynasty */
    square *bnp;
    square s;

    for (bnp= boardnum; *bnp; bnp++) {
      s = *bnp;
      if (abs(e[s])==roib && TSTFLAG(spec[s],Kamikaze)) {
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
      || (   eval_white != eval_ortho
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
    optim_neutralretractable = optim_orthomatingmoves = false;
  }

  superbas= CondFlag[antisuper] ? bas : bas - 1;

  pp= cp= 0;      /* init promotioncounter and checkcounter */
  for (p= CondFlag[losingchess] || CondFlag[dynasty] ? roib : db;
       p <= derbla;
       p++) {
    getprompiece[p]= vide;
    if (promonly[p]) {
      exist[p]= True;
    }
    if (CondFlag[protean])
      exist[reversepb]= true;
    if (exist[p]) {
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
        optim_neutralretractable = optim_orthomatingmoves = false;
        if (p != hamstb) {
          checkpieces[cp++]= p;
        }
      }
    }
  }
  checkpieces[cp]= vide;

  tp= op= 0;
  for (p= roib; p <= derbla; p++) {
    if (exist[p] && p != dummyb && p != hamstb) {
      if (whitenormaltranspieces) {
        whitetransmpieces[tp]= p;
      }
      if (blacknormaltranspieces) {
        blacktransmpieces[tp]= p;
      }
      tp++;
      if ( p != Orphan
           && p != Friend
           && (exist[Orphan] || exist[Friend]))
      {
        orphanpieces[op++]= p;
      }
    }
  }
  if (whitenormaltranspieces) {
    whitetransmpieces[tp]= vide;
  }
  if (blacknormaltranspieces) {
    blacktransmpieces[tp]= vide;
  }
  if (calc_whrefl_king || calc_blrefl_king) {
    optim_neutralretractable = optim_orthomatingmoves = false;
  }
  orphanpieces[op]= vide;

  if (  (calc_whrefl_king
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

  for (n= 2; n <= maxply; inum[n++]= inum[1])
    ;

  if (  (CondFlag[chamchess] || CondFlag[linechamchess])
        && TSTFLAG(PieSpExFlags, Chameleon))
  {
    VerifieMsg(ChameleonPiecesAndChess);
    return false;
  }

  if (TSTFLAG(PieSpExFlags, ColourChange)) {
    checkhopim = true;
    optim_neutralretractable = optim_orthomatingmoves  = false;
    jouegenre = true;
  }
  checkhopim |= CondFlag[imitators];

  if (CondFlag[sentinelles])
    /* a nasty drawback */
    optim_neutralretractable = optim_orthomatingmoves  = false;

  if (CondFlag[annan]) {
    optim_neutralretractable = optim_orthomatingmoves = false;
    rbechec= &annan_rbechec;
    rnechec= &annan_rnechec;
  }

  if (CondFlag[losingchess])
  {
    if (stipSettings[reciprocal].stipulation == stip_mate
        || stipSettings[reciprocal].stipulation == stip_check
        || stipSettings[reciprocal].stipulation == stip_mate_or_stale
        || stipSettings[nonreciprocal].stipulation == stip_mate
        || stipSettings[nonreciprocal].stipulation == stip_check
        || stipSettings[nonreciprocal].stipulation == stip_mate_or_stale)
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

  FlagMoveOrientatedStip =
    stipSettings[reciprocal].stipulation == stip_target
    || stipSettings[reciprocal].stipulation == stip_ep
    || stipSettings[reciprocal].stipulation == stip_capture
    || stipSettings[reciprocal].stipulation == stip_steingewinn
    || stipSettings[reciprocal].stipulation == stip_castling
    || stipSettings[nonreciprocal].stipulation == stip_target
    || stipSettings[nonreciprocal].stipulation == stip_ep
    || stipSettings[nonreciprocal].stipulation == stip_capture
    || stipSettings[nonreciprocal].stipulation == stip_steingewinn
    || stipSettings[nonreciprocal].stipulation == stip_castling;

  if (stipSettings[nonreciprocal].stipulation == stip_doublemate
      && (SortFlag(Self) || SortFlag(Direct)))
  {
    VerifieMsg(StipNotSupported);
    return false;
  }

  if (FlowFlag(Reci)
      && stipSettings[reciprocal].stipulation==stip_countermate)
  {
    VerifieMsg(StipNotSupported);
    return false;
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

  if (stipSettings[nonreciprocal].stipulation==stip_castling
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

  if ( CondFlag[isardam]
       || CondFlag[ohneschach]
       || CondFlag[exclusive])
  {
    flag_testlegality= true;
    optim_neutralretractable = optim_orthomatingmoves = false;
  }

  if (!CondFlag[patience]) {           /* needed because of twinning */
    PatienceB= false;
  }

  jouetestgenre=
    flag_testlegality
    || flagAssassin
    || stipSettings[nonreciprocal].stipulation==stip_doublemate
    || stipSettings[reciprocal].stipulation==stip_doublemate
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

  if (CondFlag[extinction] || flagAssassin) {
    optim_neutralretractable = optim_orthomatingmoves = false;
  }

  if (CondFlag[actrevolving] || CondFlag[arc]) {
    jouegenre= true;
    optim_neutralretractable = optim_orthomatingmoves = false;
  }

  if (anytraitor) {
    optim_neutralretractable = optim_orthomatingmoves = false;
  }
#if defined(DEBUG)
  printf("int: %s, mate: %s, stalemate: %s, "
         "castling: %s, fee: %s, orth: %s, "
         "help: %s, direct: %s, series: %s\n",
         OptFlag[intelligent]?"true":"false",
         stipSettings[nonreciprocal].stipulation == stip_mate?"true":"false",
         stipSettings[nonreciprocal].stipulation == stip_stale?"true":"false",
         testcastling?"true":"false",
         flagfee?"true":"false",
         SortFlag(Help)?"true":"false",
         SortFlag(Direct)?"true":"false",
         FlowFlag(Series)?"true":"false");
#endif      /* DEBUG */

  if (InitChamCirce) {
    if (CondFlag[leofamily]) {
      NextChamCircePiece[Leo]= Mao;
      NextChamCircePiece[Pao]= Leo;
      NextChamCircePiece[Vao]= Pao;
      NextChamCircePiece[Mao]= Vao;
    }
    else {
      piece actknight=
        CondFlag[cavaliermajeur] ? NightRider : Knight;
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
    optim_neutralretractable = optim_orthomatingmoves = false;
    nonoptgenre= true;
    WhiteStrictSAT[1]= echecc_normal(blanc);
    BlackStrictSAT[1]= echecc_normal(noir);
    satXY= WhiteSATFlights > 1 || BlackSATFlights > 1;
  }

  if (CondFlag[schwarzschacher])
  {
    optim_neutralretractable = optim_orthomatingmoves = false;
    nonoptgenre= true;
  }

  move_generation_mode_opti_per_couleur[blanc]
    = flagwhitemummer||nonoptgenre
    ? move_generation_optimized_by_killer_move
    : move_generation_optimized_by_nr_opponent_moves;
  move_generation_mode_opti_per_couleur[noir]
    = flagblackmummer||nonoptgenre
    ? move_generation_optimized_by_killer_move
    : move_generation_optimized_by_nr_opponent_moves;

  if (CondFlag[takemake])
    optim_neutralretractable = optim_orthomatingmoves = false;

  if (CondFlag[protean]) {
    flagfee= true;
    optim_neutralretractable = optim_orthomatingmoves = false;
    exist[reversepb]= true;
  }

  if (OptFlag[intelligent]
      && (!(stipSettings[nonreciprocal].stipulation==stip_mate
            || stipSettings[nonreciprocal].stipulation==stip_stale)
          || flagfee
          || SortFlag(Self)
          || !(SortFlag(Help) || (SortFlag(Direct) && FlowFlag(Series)))
          || anycirce
          || anyanticirce))
  {
    VerifieMsg(IntelligentRestricted);
    return false;
  }

  if (OptFlag[appseul])
    flag_appseul= true;

  if (CondFlag[castlingchess])
  {
    optim_neutralretractable = optim_orthomatingmoves = false;
    castling_supported= false;
    jouegenre= true;
  }

#if !defined(DATABASE)
  if (SortFlag(Proof)) {
    return ProofVerifie();
  }
#endif
    
  return true;
} /* verifieposition */

numecoup sic_coup;
ply sic_ply;

void current(coup *mov) {
  square sq = move_generation_stack[nbcou].arrival;

  mov->tr=          trait[nbply];
  mov->cdzz =           move_generation_stack[nbcou].departure;
  mov->cazz=            sq;
  mov->cpzz=            move_generation_stack[nbcou].capture;
  mov->pjzz=            pjoue[nbply];
  mov->norm_prom=       norm_prom[nbply];
  mov->ppri=            pprise[nbply];
  mov->sqren=           sqrenais[nbply];
  mov->cir_prom=        cir_prom[nbply];

  if ((bl_exact && mov->tr == noir)
      || (wh_exact && mov->tr == blanc))
  {
    mov->echec= false;
    /* A quick and dirty hack. But echecc destroys the 'current()'
     * entry  */
  }
  else if (CondFlag[isardam] || CondFlag[brunner] || SATCheck) {
    if (flag_writinglinesolution) {
      tempcoup= nbcou;
      tempply= nbply;
      nbcou= sic_coup;
      nbply= sic_ply;
      mov->echec= echecc(advers(mov->tr));
      nbcou= tempcoup;
      nbply= tempply;
      /* Not such a quick hack, but probably dirtier! */
      /* May work for the above exact conditions too    */
    }
    else {
      mov->echec= echecc(advers(mov->tr));
    }
  }
  else {
    mov->echec= echecc(advers(mov->tr));
  }
  mov->renkam= crenkam[nbply];
  mov->promi=  Iprom[nbply];
  mov->numi=     inum[nbply] - (mov->promi ? 1 : 0);
  /* Promoted imitator will be output 'normally'
     from the next move on. */
  mov->sum= isquare[0] - im0;
  mov->speci= jouespec[nbply];

  /* hope the following works with parrain too */
  mov->ren_spec=  spec[sqrenais[nbply]];
  mov->bool_senti= senti[nbply];
  mov->ren_parrain= ren_parrain[nbply];
  mov->bool_norm_cham_prom= norm_cham_prom[nbply];
  mov->bool_cir_cham_prom= cir_cham_prom[nbply];
  mov->pjazz=     jouearr[nbply];
  mov->repub_k=   repub_k[nbply];
  mov->new_spec=  spec[sq];
  mov->hurdle=    chop[nbcou];
  mov->sb3where=  sb3[nbcou].where;
  mov->sb3what= sb3[nbcou].what;
  if (mov->sb3what!=vide && mov->sb3where==mov->cdzz) {
    mov->pjzz= mov->pjazz= mov->sb3what;
  }
  mov->sb2where= sb2[nbply].where;
  mov->sb2what= sb2[nbply].what;
  mov->mren= cmren[nbcou];
  mov->osc= oscillatedKs[nbply];
  /* following only overwritten if change stack is saved in pushtabsol */
  /* redundant to init push_top */
  mov->push_bottom= NULL;
  mov->roch_sq=rochade_sq[nbcou];
  mov->roch_pc=rochade_pc[nbcou];
  mov->roch_sp=rochade_sp[nbcou];
}

int alloctab(void)
{
  int result = ++tabsol.nbr;
  tabsol.cp[result] = tabsol.cp[result-1];
  return result;
}

void freetab(void)
{
  --tabsol.nbr;
}

void pushtabsol(int n)
{
  if (++tabsol.cp[n] > tabmaxcp)
    ErrorMsg(TooManySol);
  else
    current(&tabsol.liste[tabsol.cp[n]]);

  if (flag_outputmultiplecolourchanges)
  {
    change_rec *rec;
    change_rec **sp= &tabsol.liste[tabsol.cp[n]].push_top;
    *sp= tabsol.liste[tabsol.cp[n] - 1].push_top;
    tabsol.liste[tabsol.cp[n]].push_bottom = *sp;
    for (rec= colour_change_sp[nbply-1];
         rec-colour_change_sp[nbply]<0;
         rec++)
      PushChangedColour(*sp,
                        push_colour_change_stack_limit,
                        rec->square,
                        rec->pc)
  }

  coupfort();
}

int tablen(int t)
{
  return tabsol.cp[t]-tabsol.cp[t-1];
}

boolean WriteSpec(Flags sp, boolean printcolours) {
  boolean ret= False;
  PieSpec spname;

  if (printcolours && !TSTFLAG(sp, Neutral))
    spname= White;
  else
    spname= Neutral;

  for (; spname < PieSpCount; spname++) {
    if ( (spname != Volage || !CondFlag[volage])
         && TSTFLAG(sp, spname))
    {
      StdChar(tolower(*PieSpString[ActLang][spname]));
      ret= True;
    }
  }
  return ret;
}

#if defined(DATABASE)
extern boolean two_same_pieces;
#endif

void editcoup(coup *mov) {
  char    BlackChar= *GetMsgString(BlackColor);
  char    WhiteChar= *GetMsgString(WhiteColor);
  int   icount, diff;
  square sq;

  if (mov->cazz==nullsquare) return;

  /* Did we castle ?? */
  if (mov->cpzz == kingside_castling
      || mov->cpzz == queenside_castling)
  {
    /* castling */
    StdString("0-0");
    if (mov->cpzz == queenside_castling) {
      StdString("-0");
    }
    if (CondFlag[einstein]) {
      StdChar('=');
      if (CondFlag[reveinstein])
        WritePiece(db);
      else
        WritePiece(fb);
    }
  } else {  /* no, we didn't castle */
    if (mov->cpzz == messigny_exchange) {
      WritePiece(mov->pjzz);
      WriteSquare(mov->cdzz);
      StdString("<->");
      WritePiece(mov->ppri);
      WriteSquare(mov->cazz);
    }
    else {
      if (mov->sb3what!=vide) {
        StdString("[");
        WriteSquare(mov->sb3where);
        StdString("=");
        WritePiece(mov->sb3what);
        StdString("]");
      }
      if (WriteSpec(mov->speci, false)
          || (mov->pjzz != pb && mov->pjzz != pn))
      {
        WritePiece(mov->pjzz);
      }
#if defined(DATABASE)
      if (two_same_pieces) {
        WriteSquare(mov->cdzz);
        if (mov->ppri == vide)
          StdChar('-');
        else
          StdString("\\x ");
      }
      else {
        if (mov->ppri != vide)
          StdString("\\x ");
      }
#else
      WriteSquare(mov->cdzz);
      if (anyantimars && (mov->ppri == vide || mov->cdzz == mov->cpzz))
      {
        StdString("->");
        WriteSquare(mov->mren);
      }
      if (mov->ppri == vide || (anyantimars && mov->cdzz == mov->cpzz))
        StdChar('-');
      else
        StdChar('*');
#endif /* DATABASE */
      if (mov->cpzz != mov->cazz && mov->roch_sq == initsquare) {
        if (is_pawn(mov->pjzz) && !CondFlag[takemake]) {
          WriteSquare(mov->cazz);
          StdString(" ep.");
        }
        else {
          WriteSquare(mov->cpzz);
          StdChar('-');
          WriteSquare(mov->cazz);
        }
      }
      else {
        WriteSquare(mov->cazz);
      }
    }

    if (mov->bool_norm_cham_prom) {
      SETFLAG(mov->speci, Chameleon);
    }

    if ((mov->pjzz != mov->pjazz)
        || ((mov->speci != mov->new_spec) && (mov->new_spec != 0)))
    {
      if (mov->pjazz == vide) {
        if (mov->promi) {
          StdString ("=I");
        }
      }
      else if (!((CondFlag[white_oscillatingKs] && mov->tr == blanc && mov->pjzz == roib) ||
                 (CondFlag[black_oscillatingKs] && mov->tr == noir && mov->pjzz == roin))) {
        StdChar('=');
        WriteSpec(mov->new_spec, mov->speci != mov->new_spec);
        WritePiece(mov->pjazz);
      }
    }

    if (mov->roch_sq != initsquare) {
      StdChar('/');
      WriteSpec(mov->roch_sp, true);
      WritePiece(mov->roch_pc);
      WriteSquare(mov->roch_sq);
      StdChar('-');
      WriteSquare((mov->cdzz + mov->cazz) / 2);
    }

    if (mov->sqren != initsquare) {
      piece   p= CondFlag[antieinstein]
        ? inc_einstein(mov->ppri)
        : CondFlag[parrain]
        ? mov->ren_parrain
        : CondFlag[chamcirce]
        ? ChamCircePiece(mov->ppri)
        : (anyclone && abs(mov->pjzz) != roib)
        ? -mov->pjzz
        : (anytraitor && abs(mov->ppri) >= roib)
        ? -mov->ppri
        : mov->ppri;
      StdString(" [+");
      WriteSpec(mov->ren_spec, p!=vide);
      WritePiece(p);

      WriteSquare(mov->sqren);
      if (mov->bool_cir_cham_prom) {
        SETFLAG(mov->ren_spec, Chameleon);
      }
      if (mov->cir_prom) {
        StdChar('=');
        WriteSpec(mov->ren_spec, p!=vide);
        WritePiece(mov->cir_prom);
      }

      if (TSTFLAG(mov->ren_spec, Volage)
          && SquareCol(mov->cpzz) != SquareCol(mov->sqren))
      {
        sprintf(GlobalStr, "=(%c)",
                (mov->tr == blanc) ? WhiteChar : BlackChar);
        StdString(GlobalStr);
      }
      StdChar(']');
    }

    if (mov->sb2where!=initsquare) {
      assert(mov->sb2what!=vide);
      StdString(" [");
      WriteSquare(mov->sb2where);
      StdString("=");
      WritePiece(mov->sb2what);
      StdString("]");
    }

    if (CondFlag[republican]
        && (sq= mov->repub_k) <= haut
        && sq >= bas)
    {
      SETFLAG(mov->ren_spec, mov->tr==blanc ? Black : White);
      StdString("[+");
      WriteSpec(mov->ren_spec, true);
      WritePiece(roib);
      WriteSquare(sq);
      StdChar(']');
    }

    if (mov->renkam) {
      StdChar('[');
      WriteSpec(mov->speci, mov->pjazz);
      WritePiece(mov->pjazz);
      WriteSquare(mov->cazz);
      StdString("->");
      WriteSquare(mov->renkam);
      if (mov->norm_prom != vide &&
          (!anyanticirce || (CondFlag[antisuper] && 
                             ((is_forwardpawn(mov->pjzz)
                               && !PromSq(mov->tr, mov->cazz)) || 
                              (is_reversepawn(mov->pjzz)
                               && !ReversePromSq(mov->tr, mov->cazz)))))) {
        StdChar('=');
        WriteSpec(mov->speci, true);
        WritePiece(mov->norm_prom);
      }
      StdChar(']');
    }
    if (mov->bool_senti) {
      StdString("[+");
      StdChar((!SentPionNeutral || !TSTFLAG(mov->speci, Neutral))
              ?  ((mov->tr==blanc) != SentPionAdverse
                  ? WhiteChar
                  : BlackChar)
              : 'n');
      WritePiece(sentinelb); WriteSquare(mov->cdzz);
      StdChar(']');
    }
    if (TSTFLAG(mov->speci, ColourChange)
        && (abs(e[mov->hurdle])>roib))
    {
      couleur hc= e[mov->hurdle] < vide ? noir : blanc;
      StdString("[");
      WriteSquare(mov->hurdle);
      StdString("=");
      StdChar(hc == blanc ? WhiteChar : BlackChar);
      StdString("]");
    }
    if (flag_outputmultiplecolourchanges)
    {
      if (mov->push_bottom != NULL) {

        if (mov->push_top - mov->push_bottom > 0) 
        {
          change_rec * rec;
          StdString(" [");
          for (rec= mov->push_bottom; rec - mov->push_top < 0; rec++)
          {
              StdChar(rec->pc > vide ? WhiteChar : BlackChar);
              WritePiece(rec->pc);
              WriteSquare(rec->square);
              if (mov->push_top - rec > 1)
                  StdString(", ");
          } 
          StdChar(']');
        }

      } else {

        if (colour_change_sp[nbply] - colour_change_sp[nbply - 1] > 0) 
        {
          change_rec * rec;
          StdString(" [");
          for (rec= colour_change_sp[nbply - 1]; rec - colour_change_sp[nbply] < 0; rec++)
          {
              StdChar(rec->pc > vide ? WhiteChar : BlackChar);
              WritePiece(rec->pc);
              WriteSquare(rec->square);
              if (colour_change_sp[nbply] - rec > 1)
                  StdString(", ");
          } 
          StdChar(']');
        }

      }
    }

  } /* No castling */

  if (mov->numi && CondFlag[imitators]) {
    diff = im0 - isquare[0];
    StdChar('[');
    for (icount = 1; icount <= mov->numi;) {
      StdChar('I');
      WriteSquare(isquare[icount-1] + mov->sum + diff);
      if (icount++ < mov->numi) {
        StdChar(',');
      }
    }
    StdChar(']');
  }
  if (mov->osc) {
    StdString("[");
    StdChar(WhiteChar);
    WritePiece(roib);
    StdString("<>");
    StdChar(BlackChar);
    WritePiece(roib);
    StdString("]");
  }
  if (CondFlag[BGL])
  {
    char s[30], buf1[12], buf2[12];
    if (BGL_global)
    {
      sprintf(s, " (%s)", WriteBGLNumber(buf1, BGL_white));
    }
    else
    {
      sprintf(s, " (%s/%s)", WriteBGLNumber(buf1, BGL_white), WriteBGLNumber(buf2, BGL_black));
    }
    StdString(s);
  }
  if (flende) {
    if (currentStipSettings.stipulation == stip_mate_or_stale)
      StdString(mate_or_stale_patt ? " =" : " #");
    else
      StdString(currentStipSettings.alphaEnd);
  }
  else {
    if (mov->echec)
      StdString(" +");
  }
  flende= false;
  StdChar(bl);
} /* editcoup */

boolean nowdanstab(int n)
{
  int i;
  coup mov;

  current(&mov);
  for (i = tabsol.cp[n-1]+1; i <= tabsol.cp[n]; i++) {
    if ( mov.cdzz == tabsol.liste[i].cdzz
         && mov.cazz == tabsol.liste[i].cazz
         && mov.norm_prom == tabsol.liste[i].norm_prom
         && mov.cir_prom == tabsol.liste[i].cir_prom
         && mov.bool_cir_cham_prom
         == tabsol.liste[i].bool_cir_cham_prom
         && mov.bool_norm_cham_prom
         == tabsol.liste[i].bool_norm_cham_prom
         && mov.sb3where==tabsol.liste[i].sb3where
         && mov.sb3what==tabsol.liste[i].sb3what
         && mov.sb2where==tabsol.liste[i].sb2where
         && mov.sb2what==tabsol.liste[i].sb2what
         && mov.hurdle==tabsol.liste[i].hurdle
         && (!CondFlag[takemake] || mov.cpzz==tabsol.liste[i].cpzz)
         && (!supergenre
             || (    (!(CondFlag[supercirce] || CondFlag[april])
                      || mov.sqren == tabsol.liste[i].sqren)
                     && (!CondFlag[republican]
                         || mov.repub_k == tabsol.liste[i].repub_k)
                     && (!CondFlag[antisuper]
                         || mov.renkam == tabsol.liste[i].renkam)
               )
           )
      ) {
      return true;
    }
  }
  return false;
}

void ecritcoup(void) {
  coup mov;

  current(&mov);
  editcoup(&mov);
}

void WriteForsyth(void)
{
  int row,file,cnt=0;
  piece p;
  square sq=square_a8;
  for (row=nr_rows_on_board; row!=0; --row, sq -= nr_files_on_board+dir_up)
  {
    for (file=nr_files_on_board; file; --file, ++sq)
    {
      if ((p= e[sq]) == vide)
        cnt++;
      else
      {
        if (cnt) {
          char buf[3];
          sprintf(buf, "%d", cnt);
          StdString(buf);
          cnt= 0;
        }
        if (TSTFLAG(spec[sq], Royal))
          StdChar('+');
        if (p < vide) 
        {
          if (PieceTab[-p][1] != ' ') {
            StdChar('.');
            StdChar(tolower(PieceTab[-p][0]));
            StdChar(tolower(PieceTab[-p][1]));
          }
          else
            StdChar(tolower(PieceTab[-p][0]));
        }
        else if (p > obs) 
        {
          if (PieceTab[p][1] != ' ') {
            StdChar('.');
            StdChar(toupper(PieceTab[p][0]));
            StdChar(toupper(PieceTab[p][1]));
          }
          else
            StdChar(toupper(PieceTab[p][0]));
        }
      }
    }
    if (cnt) {
      char buf[3];
      sprintf(buf, "%d", cnt);
      StdString(buf);
      cnt= 0;
    }
    if (row>1) StdChar('/');
  }
  StdChar(' ');
}

void linesolution(void) {
  int      num= 0;
  couleur       camp;

  sic_coup= nbcou;
  sic_ply= nbply;

#if !defined(DATABASE)
  if (OptFlag[intelligent]) {
    if (SolAlreadyFound()) {
      return;
    } else {
      if (OptFlag[maxsols])
        solutions++;
      if (OptFlag[beep])
        BeepOnSolution(maxbeep);
    }
    StoreSol();
  } else {
    if (OptFlag[maxsols])
      solutions++;
    if (OptFlag[beep])
      BeepOnSolution(maxbeep);
  }
#endif

  flag_writinglinesolution= true;
  repere[nbply + 1]= nbcou;
  camp= trait[nbply= 2];
  ResetPosition();
  if (   ((!flag_atob && flag_appseul) || SatzFlag)
         && !FlowFlag(Intro))
  {
    StdString("  1...");
    num= 1;
    if (flag_appseul && SatzFlag && !flag_atob)
      StdString("  ...");
    else
      camp= advers(camp);
  }

  /* seriesmovers with introductory move */
  if (FlowFlag(Intro) && !SatzFlag) {
    StdString("  1.");
    num= 1;
    nbcou= repere[nbply+1];
    initneutre(advers(trait[nbply]));
    jouecoup_no_test();
    ecritcoup();
    nbply++;
    camp= advers(camp);
  }

  while (nbply <= sic_ply) {
    if (FlowFlag(Intro)
        && trait[nbply] != camp
        && nbply < sic_ply)
    {
      camp= advers(camp);
      num= 0;
    }
    if (trait[nbply] == camp) {
      sprintf(GlobalStr,"%3d.",++num);
      StdString(GlobalStr);
    }
    flende= sic_ply == nbply;
    nbcou= repere[nbply + 1];
    initneutre(advers(trait[nbply]));
    jouecoup_no_test();
    ecritcoup();
    nbply++;
  }
  Message(NewLine);
  nbcou= sic_coup;
  nbply= sic_ply;

  flag_writinglinesolution= false;
} /* end of linesolution */

EXTERN int WhMovesLeft, BlMovesLeft;

#if !defined(DATABASE)
boolean last_h_move(couleur camp)
{
  couleur ad = advers(camp);
  boolean solution_found = false;

  if (currentStipSettings.stipulation==stip_countermate
      && !stip_checkers[stip_mate](ad))
    return false;
  
  if (currentStipSettings.stipulation==stip_doublemate
      && immobile(camp))
    return false;

  if (!(SortFlag(Self) && SortFlag(Help)))
    GenMatingMove(camp);
  else if (SortFlag(Reflex) && !FlowFlag(Semi) && matant(camp,1))
    return false;
  else
    genmove(camp);

  if (camp==blanc)
    WhMovesLeft--;
  else
    BlMovesLeft--;

  while (encore())
  {
    if (jouecoup()
        && (!OptFlag[intelligent] || MatePossible()))
    {
      if (SortFlag(Self) && SortFlag(Help))
      {
        if (!echecc(camp) && !definvref(ad,1))
        {
          GenMatingMove(ad);
          while (encore())
          {
            if (jouecoup()
                && currentStipSettings.checker(ad))
            {
              solution_found = true;
              linesolution();
            }
            repcoup();
          }
          finply();
        }
      }
      else
      {
        if (currentStipSettings.checker(camp))
        {
          solution_found = true;
          linesolution();
        }
      }
    }
    repcoup();
  }

  if (camp==blanc)
    WhMovesLeft++;
  else
    BlMovesLeft++;

  finply();

  return solution_found;
}

/* Count number of refutations after a move of the attacking side in
 * direct/self/reflex play.
 * @param defender defending side
 * @param n number of moves until end state has to be reached,
 *          including the move just played
 * @param t table where to store refutations
 * @return -1 iff in direct play, the move just played reached the end
 *            state to be reached (mate, stalemate, ...)
 *         0  if the defending side has at >=1 final move in reflex play
 *         max_nr_refutations+1 if
 *            if the defending side is immobile (it shouldn't be here!)
 *            if the defending side has more non-trivial moves than allowed
 *            if the defending king has more flights than allowed
 *            ?? TODO (something related to threats)
 *         number (0..max_nr_refutations) of refutations otherwise
 */
int dsr_find_refutations(couleur defender, int n, int t)
{
  couleur attacker = advers(defender);
  boolean is_defender_immobile = true;
  int ntcount = 0;

  if ((!FlowFlag(Exact) || enonce==1)
      && SortFlag(Direct)
      && currentStipSettings.checker(attacker)) /* find short solutions */
    return -1;

  if (SortFlag(Reflex) && matant(defender,1))
    return 0;

  if (n>droh
      && !echecc(defender)
      && !(droh>0 && dsr_does_attacker_win(attacker,droh)))
    return max_nr_refutations+1;

  if (n>2 && OptFlag[solflights])
  {
    int nrflleft = maxflights+1;
    square save_rbn = defender==noir ? rn : rb;
    genmove(defender);
    while (encore() && nrflleft>0)
    {
      if (jouecoup()
          && (save_rbn != (defender==noir ? rn : rb)))
      {
        if (!echecc(defender))
          nrflleft--;
      }
      repcoup();
    }
    finply();
    if (nrflleft==0)
      return max_nr_refutations+1;
  }

  /* Check whether black has more non trivial moves than he is
     allowed to have. The number of such moves allowed
     (NonTrivialNumber) is entered using the nontrivial option.
  */
  if (n>NonTrivialLength)
  {
    ntcount= -1;
    /* Initialise the counter. It is counted down. */

    /* generate a ply */
    genmove(defender);

    /* test all possible moves */
    while (encore() && NonTrivialNumber>=ntcount)
    {
      /* Test whether the move is legal and not trivial. */
      if (jouecoup()
          && !echecc(defender)
          && !((NonTrivialLength>0)
               && dsr_does_attacker_win(attacker,NonTrivialLength)))
        /* The move is legal and not trivial. Hence increment the
           counter.
        */
        ntcount++;
      repcoup();
    }
    finply();

    if (NonTrivialNumber<ntcount)
      return max_nr_refutations+1;

    NonTrivialNumber -= ntcount;
  }

  if (n>2)
    move_generation_mode= move_generation_mode_opti_per_couleur[defender];

  genmove(defender);
  move_generation_mode= move_generation_optimized_by_killer_move;

  while (encore() && tablen(t)<=max_nr_refutations)
  {
    if (jouecoup() && !echecc(defender))
    {
      is_defender_immobile = false;
      if (!dsr_does_attacker_win(attacker,n))
        pushtabsol(t);
    }
    repcoup();
  }
  finply();

  if (n>NonTrivialLength)
    NonTrivialNumber += ntcount;

  return is_defender_immobile ? max_nr_refutations+1 : tablen(t);
} /* dsr_def */

/* Determine whether the move of the defending side in
 * direct/self/reflex play defends against the threats.
 * @param attacker attacking side
 * @param n number of moves until end state has to be reached from now
 * @param t table containing the threats
 * @return true iff the move just played defends against at least one
 *         of the threats
 */
boolean dsr_defends_threats(couleur attacker, int n, int t)
{
  int zaehler = 0;
  boolean defense_found = false;
  couleur defender = advers(attacker);

  if (tablen(t)==0)
    return true;

  genmove(attacker);
  while (encore() && !defense_found)
  {
    if (jouecoup() && nowdanstab(t) && !echecc(attacker))
    {
      if (n==1 && SortFlag(Direct))
        defense_found = !currentStipSettings.checker(attacker);
      else
        defense_found = !dsr_does_defender_lose(defender,n);

      if (defense_found)
      {
        coupfort();
      }
      else
        zaehler++;
    }
    repcoup();
  }
  finply();

  return zaehler<tablen(t);
}

/* Determine and write all final moves of a self/reflex variation.
 * @param defender defending side (i.e. side executing the final move(s))
 */
void sr_find_write_final_moves(couleur defender)
{
  genmove(defender);
  while(encore())
  {
    if (jouecoup() && currentStipSettings.checker(defender))
    {
      StdString("\n");
      Tabulate();
      sprintf(GlobalStr,"%3d...",zugebene);
      if (zugebene==1)
      {
        if (OptFlag[maxsols]) 
          solutions++;
        if (OptFlag[beep])
          BeepOnSolution(maxbeep);
      }
      StdString(GlobalStr);
      flende = true;
      ecritcoup();
    }
    repcoup();
    if (zugebene==1
        && ((OptFlag[maxsols] && solutions>=maxsolutions)
            || maxtime_status==MAXTIME_TIMEOUT))
      break;
  }
  finply();
  StdString("\n");
}

/* Count all non-trivial moves of the defending side. Whether there
 * are trivial moves and whether a particular move is non-trivial is
 * determined by user input.
 * @param defender defending side (i.e.side for which to count
 *                 non-trivial moves)
 */
int count_non_trivial(couleur defender)
{
  couleur attacker = advers(defender);
  int result = 0;

  /* generate a ply */
  genmove(defender);

  /* test all possible moves */
  while (encore())
  {
    /* Test whether the move is legal and not trivial. */
    if (jouecoup()
        && !echecc(defender)
        && (NonTrivialLength==0
            || !dsr_does_attacker_win(attacker,NonTrivialLength)))
      /* The move is legal and not trivial. Increment the counter. */
      ++result;
    repcoup();
  }

  finply();

  return result;
}

/* Write a variation in the try/solution/set play of a
 * direct/self/reflex stipulation. The move of the defending side that
 * starts the variation has already been played in the current ply.
 * Only continuations of minimal length are looked for and written.
 * @param attacker attacking side
 * @param n number of moves until end state has to be reached from now
 */
void dsr_write_variation(couleur attacker, int n)
{
  boolean isRefutation = true; /* until we prove otherwise */
  int i;

  Tabulate();
  sprintf(GlobalStr,"%3d...",zugebene);
  StdString(GlobalStr);
  ecritcoup();
  StdString("\n");
  marge+= 4;

  for (i = FlowFlag(Exact) ? n : 1; i<=n && isRefutation; i++)
  {
    int mats = alloctab();
    dsr_find_write_tries_solutions(attacker,i,mats,False);
    freetab();
    isRefutation = tablen(mats)==0;
  }

  if (isRefutation)
  {
    marge+= 2;
    Tabulate();
    Message(Refutation);
    marge-= 2;
  }

  marge-= 4;
}

/* Determine and write set play of a direct/self/reflex stipulation
 * @param attacker attacking side
 * @param n number of moves until end state has to be reached,
 *          including the virtual key move
 * @param par TODO
 */
void dsr_find_write_setplay(couleur attacker, int n, int par)
{
  couleur defender = advers(attacker);
  int ntcount = 0;

  if (n==1)
  {
    if (SortFlag(Direct))
      Message(NewLine);
    else
      sr_find_write_final_moves(defender);

    return;
  }

  if (!SortFlag(Direct) && dsr_does_defender_lose(defender,1))
  {
    sr_find_write_final_moves(defender);
    return;
  }

  StdString("\n");

  n--;

  if (n>NonTrivialLength)
  {
    ntcount = count_non_trivial(defender)-1;
    NonTrivialNumber -= ntcount;
  }

  /* TODO use same genmove() here and in count_non_trivial() and just
   * rewind ply here? */
  genmove(defender);
  while(encore())
  {
    /* TODO is this nowdanstab() check useful in any way? */
    if (jouecoup() && !echecc(defender) && !nowdanstab(par))
    {
      if (!SortFlag(Direct) && currentStipSettings.checker(defender))
        ; /* oops, wrong side! */
      else if (dsr_does_attacker_win(attacker,n))
        /* yipee - this solves! */
        dsr_write_variation(attacker,n);
    }
    repcoup();
  }
  finply();

  if (n>NonTrivialLength)
    NonTrivialNumber += ntcount;
} /* dsr_find_write_setplay */

/* Determine and write the threat and variations in direct/self/reflex
 * play after the move that has just been played in the current ply.
 * We have already determined that this move doesn't have more
 * refutations than allowed.
 * @param attacker attacking side (i.e. side that has just played)
 * @param n number of moves until end state has to be reached,
 *          including the move just played
 * @param par TODO
 */
void dsr_find_write_threats_variations(couleur attacker, int n, int par)
{
  couleur defender = advers(attacker);
  int mena;
  int lenthreat = 1;
  int ntcount = 0;

  if (!OptFlag[solvariantes])
  {
    Message(NewLine);
    return;
  }

  if (n==1)
  {
    if (SortFlag(Direct))
      Message(NewLine);
    else
      sr_find_write_final_moves(defender);

    return;
  }

  if (SortFlag(Reflex) && dsr_does_defender_lose(defender,1))
  {
    sr_find_write_final_moves(defender);
    return;
  }

  n--;
  mena = alloctab();
  if (OptFlag[nothreat] || echecc(defender))
    StdString("\n");
  else
  {
    int max_threat_length = n>droh ? droh : n;
    int i;
    DrohFlag = true;
    marge+= 4;
    for (i = 1; i<=max_threat_length; i++)
    {
      dsr_find_write_tries_solutions(attacker,i,mena,False);
      if (tablen(mena)>0)
      {
        lenthreat = i;
        break;
      }
    }
    marge-= 4;
    if (DrohFlag)
    {
      Message(Zugzwang);
      DrohFlag = false;
    }
  }

  if (n>NonTrivialLength)
  {
    ntcount = count_non_trivial(defender)-1;
    NonTrivialNumber -= ntcount;
  }

  /* TODO use same genmove() here and in count_non_trivial() and just
   * rewind ply here? */
  genmove(defender);
  while(encore())
  {
    if (jouecoup() && !echecc(defender) && !nowdanstab(par))
    {
      if (OptFlag[noshort] && dsr_does_attacker_win(attacker,n-1))
        ; /* variation shorter than stip; thanks, but no thanks! */
      else if (lenthreat>1 && dsr_does_attacker_win(attacker,lenthreat-1))
        ; /* variation shorter than threat */
      /* TODO avoid double calculation if lenthreat==n*/
      else if (!SortFlag(Direct) && currentStipSettings.checker(defender))
        ; /* oops! wrong side */
      else if (!dsr_defends_threats(attacker,lenthreat,mena))
        ; /* move doesn't defend against threat */
      else
        dsr_write_variation(attacker,n);
    }
    repcoup();
  }
  finply();
  freetab();

  if (n>NonTrivialLength)
    NonTrivialNumber += ntcount;
} /* dsr_find_write_threats_variations */

/* Write the refutations stored in a table
 * @param t table containing refutations
 */
void dsr_write_refutations(int t)
{
  if (tabsol.cp[t]!=tabsol.cp[t-1])
  {
    int n;
    Tabulate();
    Message(But);
    for (n = tabsol.cp[t]; n>tabsol.cp[t-1]; n--)
    {
      Tabulate();
      StdString("  1...");
      editcoup(&tabsol.liste[n]);
      StdString(" !\n");
    }
  }
  StdChar('\n');
}

/* Determine and write the solutions and tries in the current position
 * in direct/self/reflex play.
 * @param attacker attacking side
 * @param n number of moves until end state has to be reached
 * @param t table where to store first moves of tries and solutions
 * @param restartenabled true iff the written solution should only
 *        start at the Nth legal move of attacker (determined by user
 *        input)
 */
void dsr_find_write_tries_solutions(couleur attacker,
                                    int n,
                                    int t,
                                    boolean restartenabled)
{
  couleur defender = advers(attacker);

  if (n==enonce && !FlowFlag(Semi) && SortFlag(Reflex))
  {
    if (matant(attacker,1))
    {
      int def = alloctab();
      if (currentStipSettings.stipulation==stip_mate_or_stale)
        sprintf(GlobalStr, "%s1:\n", mate_or_stale_patt ? " =" : " #");
      else
        sprintf(GlobalStr, "%s1:\n", currentStipSettings.alphaEnd);
      StdString(GlobalStr);
      StipFlags|= SortBit(Direct);
      StipFlags|= FlowBit(Semi);
      dsr_find_write_tries_solutions(attacker,1,def,False);
      freetab();
      return;
    }
  }

  zugebene++;
  genmove(attacker);
  while (encore())
  {
    if (jouecoup()
        && !(restartenabled && MoveNbr<RestartNbr)
        && !echecc(attacker)
        && !nowdanstab(t))
    {
      int def = alloctab();
      int nr_refutations;
      if (n==1 && SortFlag(Direct))
        nr_refutations = (currentStipSettings.checker(attacker)
                          ? 0
                          : max_nr_refutations+1);
      else if (n==1
               && OptFlag[quodlibet]
               && currentStipSettings.checker(attacker))
        nr_refutations = 0;
      else
        nr_refutations = (zugebene==1
                          ? dsr_find_refutations(defender,n-1,def)
                          : (dsr_does_defender_lose(defender,n)
                             ? 0
                             : max_nr_refutations+1));

      if (nr_refutations<=max_nr_refutations)
      {
        flende= ((n==1 && SortFlag(Direct))
                 || nr_refutations==-1
                 || (n==1
                     && OptFlag[quodlibet]
                     && currentStipSettings.checker(attacker)));
        if (DrohFlag)
        {
          Message(Threat);
          DrohFlag= false;
        }
        Tabulate();
        sprintf(GlobalStr,"%3d.",zugebene);
        StdString(GlobalStr);
        ecritcoup();
        if (zugebene==1)
        {
          if (nr_refutations<1)
          {
            StdString("! ");
            if (OptFlag[maxsols])
              solutions++;
            if (OptFlag[beep])
              BeepOnSolution(maxbeep);
          }
          else
            StdString("? ");
        }
        marge+= 4;
        dsr_find_write_threats_variations(attacker,n,def);
        if (zugebene==1)
          dsr_write_refutations(def);
        marge-= 4;
      }
      freetab();
      if (nr_refutations<=max_nr_refutations)
        pushtabsol(t);
    }

    if (restartenabled)
      IncrementMoveNbr();

    repcoup();

    if (zugebene==1
        && ((OptFlag[maxsols] && solutions>=maxsolutions)
            || maxtime_status==MAXTIME_TIMEOUT))
      break;
  }
  zugebene--;
  finply();
} /* dsr_find_write_tries_solutions */

boolean dsr_does_defender_lose(couleur defender, int n)
{
  return SortFlag(Direct) ? mate(defender,n-1) : !definvref(defender,n);
}

boolean dsr_does_attacker_win(couleur attacker, int n)
{
  return SortFlag(Direct) ? matant(attacker,n) : invref(attacker,n);
}

void SolveSeriesProblems(couleur camp)
{
  boolean is_exact = FlowFlag(Exact);
  int i;

  move_generation_mode = move_generation_not_optimized;

  flag_appseul= False;   /* -- no meaning in series movers would only
                            distort output */

  if (SortFlag(Help))
    camp = advers(camp);

  if (FlowFlag(Intro))
  {
    /* seriesmovers with introductory move */
    camp = advers(camp);
    
    /* check whether a king can be captured */
    if ((OptFlag[solapparent] && echecc(camp))
         || echecc(advers(camp)))
    {
      ErrorMsg(KingCapture);
      return;
    }
    introseries(camp,introenonce,OptFlag[movenbr]);
  }
  else
  {
    if (OptFlag[solapparent]
        && !SortFlag(Direct) && !OptFlag[restart])
    {
      SatzFlag= True;
      if (echecc(camp))
        ErrorMsg(KingCapture);
      else
      {
        if (SortFlag(Help))
          last_h_move(advers(camp));
        else
        {
          zugebene++;
          dsr_find_write_setplay(camp,1,0);
          zugebene--;
        }
      }
      SatzFlag= False;
      Message(NewLine);
    }

    if (OptFlag[maxsols])    /* reset after set play */
      solutions= 0;

    if (echecc(advers(camp)))
      ErrorMsg(KingCapture);
    else {
      int starti = (FlowFlag(Exact) || OptFlag[restart]
                    ? enonce
                    : 1);
      if (OptFlag[intelligent])
      {
        for (i = starti; i <= enonce; i++)
        {
          if (SortFlag(Help)
              ? Intelligent(1,i,ser_h_find_write_solutions,camp,i)
              : Intelligent(i,0,ser_dsr_find_write_solutions,camp,i))
          {
            StipFlags |= FlowBit(Exact);
            if (OptFlag[stoponshort] && i<enonce)
            {
              FlagShortSolsReached= true;
              break;
            }
          }
        }
      }
      else
      {
        for (i = starti; i<=enonce; i++)
        {
          boolean restartenabled = OptFlag[movenbr] && i==enonce;

          if (SortFlag(Help)
              ? ser_h_find_write_solutions(camp,i,restartenabled)
              : ser_dsr_find_write_solutions(camp,i,restartenabled))
          {
            StipFlags |= FlowBit(Exact);
            if (OptFlag[stoponshort]&& i<enonce)
            {
              FlagShortSolsReached= true;
              break;
            }
          } /* SortFlag(Help) */
        } /* for i */
      } /* OptFlag[intelligent] */
    } /* echecs(advers(camp)) */
  } /* FlowFlag(Intro) */

  if (!is_exact)
    StipFlags &= ~FlowBit(Exact);
} /* SolveSeriesProblems */

void SolveHelpProblems(couleur camp)
{
  int n = 2*enonce;
  int i;
  boolean is_exact = FlowFlag(Exact);

  move_generation_mode= move_generation_not_optimized;

  if (SortFlag(Self))
  {
    n--;
    camp= advers(camp);
  }

  if (flag_appseul)
  {
    n--;
    camp= advers(camp);
  }

  if (OptFlag[solapparent])
  {
    SatzFlag= True;
    if (echecc(advers(camp)))
      ErrorMsg(KingCapture);
    else
    {
      /* we are looking for shortest set plays only */
      int starti = (FlowFlag(Exact) || OptFlag[restart]
                    ? n-1
                    : ((n-1)&1 ? 1 : 2));

      if (OptFlag[intelligent])
      {
        WhMovesLeft = BlMovesLeft = starti/2;
        if (starti & 1)
          WhMovesLeft++;

        for (i = starti; i<=n-1; i += 2)
        {
          boolean flag = Intelligent(WhMovesLeft,BlMovesLeft,mataide,camp,i);

          WhMovesLeft++;
          BlMovesLeft++;

          if (flag)
            break;
        }
      }
      else
        for (i = starti; i<=n-1; i += 2)
        {
          if (mataide(camp, i, OptFlag[movenbr] && i==n-1))
            break;
        }
    }
    StdChar('\n');
    SatzFlag= False;
  }

  if (OptFlag[maxsols])    /* reset after set play */
    solutions= 0;

  if (echecc(camp))
    ErrorMsg(KingCapture);
  else
  {
    int starti= (FlowFlag(Exact) || OptFlag[restart]
                 ? n
                 : (n&1 ? 1 : 2));

    if (OptFlag[intelligent])
    {
      WhMovesLeft = BlMovesLeft = starti/2;
      if (starti & 1)
        WhMovesLeft++;

      for (i = starti; i<=n; i += 2)
      {
        if (Intelligent(WhMovesLeft,BlMovesLeft,mataide,advers(camp),i))
        {
          StipFlags |= FlowBit(Exact);
          if (OptFlag[stoponshort] && i<n)
          {
            FlagShortSolsReached= true;
            break;
          }
        }
        
        WhMovesLeft++;
        BlMovesLeft++;
      }
    }
    else
    {
      for (i = starti; i<=n; i += 2)
      {
        if (mataide(advers(camp), i, OptFlag[movenbr] && i==n))
        {
          /* Exact has to be set to find ALL longer solutions */
          StipFlags |= FlowBit(Exact);
          if (OptFlag[stoponshort] && i<n)
          {
            FlagShortSolsReached = true;
            break;
          }
        }
      }
    } /* OptFlag[intelligent] */
  }

  if (!is_exact)
    StipFlags &= ~FlowBit(Exact);
} /* SolveHelpProblems */

void SolveDirectProblems(couleur camp)
{
  if (OptFlag[postkeyplay])
  {
    if (echecc(camp))
      ErrorMsg(SetAndCheck);
    else
    {
      int lsgn = alloctab();
      zugebene++;
      dsr_find_write_threats_variations(camp,enonce,lsgn);
      zugebene--;
      freetab();
      Message(NewLine);
    }
  }
  else
  {
    if (OptFlag[solapparent] && enonce>1)
    {
      if (echecc(camp))
        ErrorMsg(SetAndCheck);
      else
      {
        int lsgn = alloctab();
        zugebene++;
        dsr_find_write_setplay(camp,enonce,lsgn);
        zugebene--;
        freetab();
        Message(NewLine);
      }
    }

    if (echecc(advers(camp)))
      ErrorMsg(KingCapture);
    else
    {
      int lsgn = alloctab();
      dsr_find_write_tries_solutions(camp,enonce,lsgn,OptFlag[movenbr]);
      freetab();
    }
  }
}

void initduplex(void) {
  /*
    A hack to make the intelligent mode work with duplex.
    But anyway I have to think about the intelligent mode again
  */
  square *bnp;
  for (bnp= boardnum; *bnp; bnp++) {
    if (!TSTFLAG(spec[*bnp], Neutral) && e[*bnp] != vide) {
      e[*bnp]= -e[*bnp];
      spec[*bnp]^= BIT(White)+BIT(Black);
    }
  }
  for (bnp= boardnum; *bnp < (bas+haut)/2; bnp++) {
    square sq2= *bnp%onerow+onerow*((onerow-1)-*bnp/onerow);

    piece p= e[sq2];
    Flags sp= spec[sq2];

    e[sq2]= e[*bnp];
    spec[sq2]= spec[*bnp];

    e[*bnp]= p;
    spec[*bnp]= sp;
  }
}

/* assert()s below this line must remain active even in "productive"
 * executables. */
#undef NDEBUG
#include <assert.h>

/* Check assumptions made throughout the program. Abort if one of them
 * isn't met. */
void checkGlobalAssumptions(void)
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

int main(int argc, char *argv[]) {
  Token   tk = BeginProblem;
  int     i, l;
  boolean flag_starttimer;
  char    *ptr, ch = 'K';
  size_t maxmemUsersetting = 0;

  set_nice_priority();
  
  checkGlobalAssumptions();

  i=1;
  MaxTime = UINT_MAX;
  flag_regression= false;
  while (i<argc) {
    if (i+1<argc && strcmp(argv[i], "-maxpos")==0)
    {
      i++;
      MaxPositions = strtoul(argv[i], &ptr, 10);
      if (argv[i]==ptr)
      {
        /* conversion failure
         * -> set to 0 now and to default value later */
        MaxPositions = 0;
      }
      i++;
      continue;
    }
    else if (i+1<argc && strcmp(argv[i], "-maxtime")==0)
    {
      i++;
      MaxTime = strtol(argv[i], &ptr, 10);
      if (argv[i]==ptr)
      {
        /* conversion failure -> assume no max time */
        MaxTime = UINT_MAX;
      }
      i++;
      continue;
    }
    else if (i+1<argc && strcmp(argv[i], "-maxmem")==0)
    {
      i++;
      maxmemUsersetting = strtoul(argv[i], &ptr, 10);
      if (argv[i]==ptr)
      {
        /* conversion failure
         * -> set to 0 now and to default value further down */
        maxmemUsersetting = 0;
      }
      else if (*ptr=='G')
      {
        maxmemUsersetting <<= 30;
        ch = 'G';
      }
      else if (*ptr=='M')
      {
        maxmemUsersetting <<= 20;
        ch = 'M';
      }
      else
      {
        maxmemUsersetting <<= 10;
        ch = 'K';
      }

      i++;
      continue;
    }
    else if (strcmp(argv[i], "-regression")==0)
    {
      flag_regression = true;
      i++;
      continue;
    }
    else {
      break;
    }
  }

  MaxMemory = adjustMaxmemory(maxmemUsersetting);
  
  if (i<argc)
    OpenInput(argv[i]);
  else
    OpenInput(" ");

  initMaxtime();

  /* We do not issue our startup message via the language
     dependant Msg-Tables, since there the version is
     too easily changed, or not updated.
     StartUp is defined in pydata.h.
  */
  if ((MaxMemory>>10)<1024 || ch=='K')
    sprintf(MMString, " (%lu KB)\n", (unsigned long)(MaxMemory>>10));
  else if ((MaxMemory>>20)<1024 || ch=='M')
    sprintf(MMString, " (%lu MB)\n", (unsigned long)(MaxMemory>>20));
  else
    sprintf(MMString, " (%lu GB)\n", (unsigned long)(MaxMemory>>30));

  pyfputs(StartUp, stdout);
  pyfputs(MMString, stdout);

  /* start timer to be able to display a reasonable time if the user
   * aborts execution before the timer is started for the first
   * problem */
  StartTimer();

  /* For the very first time we try to open any *.str
     When we reach the last remainig language, we force
     the existence of a *.str file. If none is found,
     InitMsgTab will exit with an appropriate message.
     Later, when the first Token is read, we reinitialize.
     See ReadBeginSpec in pyio.c
  */
  l= 0;
  while (l<LangCount-1 && InitMsgTab(l, False) == False) {
    l++;
  }
  if (l == LangCount-1) {
    InitMsgTab(l, True);
  }

  InitCheckDir();

  do
  {
    boolean printa= true;
    InitBoard();
    InitCond();
    InitOpt();
    InitStip();

    FlagMaxSolsReached= false;
    FlagShortSolsReached= false;

    /* New problem, so reset the timer and the solutions */

    flag_starttimer= true;

    do
    {
      InitAlways();

      tk= ReadProblem(tk);

      if (tk == ZeroPosition)
      {
        if (!OptFlag[noboard])
          WritePosition();

        tk= ReadProblem(tk);
        if (LaTeXout)
          LaTeXBeginDiagram();

        printa= false;
      }

      if (flag_starttimer)
      {
        /* Set the timer for real calculation time */
        StartTimer();
        flag_starttimer= false;
      }

      /* Set maximal solving time if the user asks for it on the
       * command line or as an option.
       * If a maximal time is indicated both on the command line and
       * as an option, use the smaller value.
       */
      if (OptFlag[maxtime] || MaxTime<UINT_MAX)
      {
        if (MaxTime<maxsolvingtime)
          maxsolvingtime = MaxTime;
      }
      else
        /* maxsolvingtime should already be ==UINT_MAX, but let's err
         * on the safe side */
        maxsolvingtime = UINT_MAX;
      
      setMaxtime(&maxsolvingtime);

      maincamp= OptFlag[halfduplex] ? noir : blanc;

      if (verifieposition()) {
        initStipCheckers();
        
        if (!OptFlag[noboard]) {
          WritePosition();
        }
        if (printa) {
          if (LaTeXout) {
            LaTeXBeginDiagram();
          }
          if (tk == TwinProblem) {
            StdString("a)\n\n");
          }
        }
        StorePosition();
        if (SortFlag(Proof)) {
          ProofInitialise();
          inithash();
          /* no DUPLEX for SPG's ! */
          if (FlowFlag(Alternate)) {
            maincamp = flag_atob
              ? (flag_appseul
                 ? blanc
                 : noir)
              : blanc;
            if (flag_atob
                && OptFlag[solapparent]
                && enonce>1) {
              SatzFlag= true;
              ProofSol(advers(maincamp), enonce-1, OptFlag[movenbr]);
              SatzFlag=false;
            }
            ProofSol(maincamp, enonce, OptFlag[movenbr]);
          }
          else
            SeriesProofSol(enonce, OptFlag[movenbr]);
          closehash();
          Message(NewLine);
        }
        else {
          do {
            inithash();
            if (FlowFlag(Alternate)) {
              if (SortFlag(Help)) {
                if (OptFlag[duplex]
                    && OptFlag[intelligent])
                {
                  SolveHelpProblems(blanc);
                }
                else {
                  SolveHelpProblems(maincamp);
                }
              }
              else {
                SolveDirectProblems(maincamp);
              }
            }
            else if (OptFlag[duplex]
                     && OptFlag[intelligent])
            {
              SolveSeriesProblems(blanc);
            }
            else {
              SolveSeriesProblems(maincamp);
            }
            Message(NewLine);
            if (OptFlag[duplex]) {
              /* Set next side to calculate for duplex "twin" */
              maincamp= advers(maincamp);
              if ((OptFlag[maxsols] && solutions>=maxsolutions)
                  || (OptFlag[stoponshort] && FlagShortSolsReached))
              {
                FlagMaxSolsReached= true;
                /* restart calculation of maxsolution after "twinning"
                 */
                solutions= 0;
              }
#if defined(HASHRATE)
              HashStats(1, "\n\n");
#endif
              if (OptFlag[intelligent]) {
                initduplex();
                if (!verifieposition()) {
                  break;
                }
              }
            } /* OptFlag[duplex] */

            closehash();

          } while (OptFlag[duplex]
                   && maincamp == noir
                   && verifieposition());
        }
      } /* verifieposition */
      printa= false;
      if ((OptFlag[maxsols] && solutions>=maxsolutions)
          || (OptFlag[stoponshort] && FlagShortSolsReached))
      {
        FlagMaxSolsReached= true;
        /* restart calculation of maxsolution after "twinning"*/
        solutions= 0;
      }
    } while (tk == TwinProblem);

    if (FlagMaxSolsReached
        || (OptFlag[intelligent] && maxsol_per_matingpos!=ULONG_MAX)
        || maxtime_status==MAXTIME_TIMEOUT)
      StdString(GetMsgString(InterMessage));
    else
      StdString(GetMsgString(FinishProblem));

    StdString(" ");
    PrintTime();
    StdString("\n\n\n");

    if (LaTeXout) {
      LaTeXEndDiagram();
    }

  } while (tk == NextProblem);

  CloseInput();

  if (LaTeXout) {
    LaTeXClose();
  }
  exit(0);
} /*main */

#if defined(NOMEMSET)
void memset(char *poi, char val, int len)
{
  while (len--)
    *poi++ = val;
}
#endif

#endif  /* ! DATABASE */
