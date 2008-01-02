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
 ** 2007/11/19 NG   Bugfix: intelligent mode with helpselfmate not supported
 **
 ** 2008/01/02 NG   New condition: Geneva Chess 
 **
 ***************************** End of List ******************************/

#ifdef macintosh	/* is always defined on macintosh's  SB */
#	define SEGM2
#	include "pymac.h"
#endif

#ifdef ASSERT
#include <assert.h> /* V3.71 TM */
#else
/* When ASSERT is not defined, eliminate assert calls.
 * This way, "#ifdef ASSERT" is not clobbering the source.
 *						ElB, 2001-12-17.
 */
#define assert(x)
#endif	/* ASSERT */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>		  /* H.D. 10.02.93 prototype for exit */
#include <ctype.h>
#ifdef UNIX
#	include <unistd.h>		  /* alarm() prototype ElB, 2001-12-09*/
#endif	/* UNIX */

/* TurboC and BorlandC	 TLi */
#ifdef	__TURBOC__
#	include <mem.h>
#	include <alloc.h>
#	include <conio.h>
#endif	/* __TURBOC__ */

#ifdef WIN32			/* V3.54  NG */
#include <process.h>
#endif	/* WIN32 */

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

boolean supergenre;	  /* V3.50 SE */

boolean is_rider(piece p)				 /* TM V3.12 */
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
  case    vizridb:			/* V3.54  NG */
  case    fersridb:			/* V3.54  NG */
    return true;
  default:	return false;
  }
}

boolean is_leaper(piece p)		 /* TM V3.12 */
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
  case    okapib:			 /* V3.32  TLi */
  case    cb:
  case    leap15b:			      /* V3.46	NG */
  case    leap16b:			      /* V3.46	NG */
  case    leap24b:			      /* V3.46	NG */
  case    leap25b:			      /* V3.46	NG */
  case    leap35b:			      /* V3.46	NG */
  case    leap37b:			      /* V3.46	NG */
  case    leap36b:			      /* V3.64	TLi */
    return true;
  default:	return false;
  }
} /* is_leaper */

boolean is_simplehopper(piece p)			 /* TM V3.12 */
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
  case g2b:			/* V3.44  NG */
  case g3b:			/* V3.44  NG */
  case khb:			/* V3.44  NG */
  case orixb:			/* V3.44  NG */

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
  default:	return false;
  }
}

boolean is_simpledecomposedleaper(piece p) /* V3.81 SE */
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

boolean is_symmetricfairy(piece p) /* V3.81 SE */
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


boolean verifieposition(void) {			    /* H.D. 10.02.93 */
  square	    *bnp;
  piece	    p;
  ply		    n;
  smallint	    cp, pp, tp, op;
  int		    i;			     /* V3.41  TLi */
  boolean          nonoptgenre;

  if (CondFlag[glasgow] && CondFlag[circemalefique])	/* V3.39  TLi */
	anycirprom= True;

  /* initialize promotion squares */
  if (!CondFlag[einstein]) {				/* V3.39  TLi */
	square i;
	if (!CondFlag[whprom_sq]) {	      /* V3.57	NG, V3.64  NG */
      for (i= 0; i < 8; i++)
		SETFLAG(sq_spec[CondFlag[glasgow] ? square_h7-i : square_h8-i],
                WhPromSq);
	}
	if (!CondFlag[blprom_sq]) {	      /* V3.57	NG, V3.64  NG */
      for (i= 0; i < 8; i++)
		SETFLAG(sq_spec[CondFlag[glasgow] ? square_a2+i : square_a1+i],
                BlPromSq);
	}
  }

  /* aus main() verschoben  V3.03  TLi */
  im0= isquare[0];					/* V2.4d  TM */
  if (! CondFlag[imitators])				/* V3.00  NG */
	CondFlag[noiprom]= true;

  if ((droh < 0) || (enonce <= droh)) {	       /* V2.1c  TLi */
    /* droh= enonce;		    twinning bug  V3.41  TLi */
	droh= maxply;			/* V3.41  TLi */
  }

  zugebene= 0;
  if (FlowFlag(Alternate) && ! SortFlag(Proof)) {	/* V3.54  NG */
	if ((enonce < 2) && (maxdefen > 0) && (!SortFlag(Self))) {
      ErrorMsg(TryInLessTwo);
      maxdefen= 0;
	}
	if ( OptFlag[stoponshort]
         && (SortFlag(Direct) || SortFlag(Self)))	 /* V3.60  NG */
	{
      ErrorMsg(NoStopOnShortSolutions);
      OptFlag[stoponshort]= false;
	}
	if (enonce > (maxply-1)/2) {			/* V3.13  NG */
      return VerifieMsg(BigNumMoves);
	}
  }
  else {
	if (enonce >= maxply-2) {			/* V3.13  NG */
      return VerifieMsg(BigNumMoves);
	}
  }

  totalortho=
    ((NonReciStipulation == stip_mate)		/* V3.32  TLi */
	 || (NonReciStipulation == stip_check)
	 || (NonReciStipulation == stip_doublemate))
    && (!FlowFlag(Reci)				/* V3.52  NG */
        || (ReciStipulation == stip_mate)
        || (ReciStipulation == stip_check)
        || (ReciStipulation == stip_doublemate));

  if (stipulation == stip_steingewinn		       /* V3.03  TLi */
      && CondFlag[parrain])
  {
	return VerifieMsg(PercentAndParrain);
  }

  flagdiastip=			  /* V3.50 SE */
	NonReciStipulation == stip_circuit
    || NonReciStipulation == stip_exchange
    || NonReciStipulation == stip_circuitB
    || NonReciStipulation == stip_exchangeB
    || ReciStipulation == stip_circuit
    || ReciStipulation == stip_exchange
    || ReciStipulation == stip_circuitB
    || ReciStipulation == stip_exchangeB;

  for (p= roib; p <= derbla; p++) {
	nbpiece[p]= 0;
	nbpiece[-p]= 0;
  }
  if (TSTFLAG(PieSpExFlags, HalfNeutral)) {		 /* V3.57 SE */
	SETFLAG(PieSpExFlags, Neutral);
  }

  /* moved from InitBoard() (py1.c)  V3.41  TLi */
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
	if (p != vide) {				/* V3.22  TLi */
      if (p < fn) {
        exist[-p]= true;
      }
      else if (p > fb) {
        exist[p]= true;
      }
      nbpiece[e[*bnp]]++;
    }
  }
  
  if (CondFlag[dynasty]) { /* V4.02 TM */
    square *bnp;
    square s;

    if (bl_royal_sq!=initsquare || wh_royal_sq!=initsquare )
      return VerifieMsg(DynastyAndRoyalSquare);

    if (CondFlag[republican])
      return VerifieMsg(DynastyAndRepublican);

    if (CondFlag[white_oscillatingKs] || CondFlag[black_oscillatingKs])
      return VerifieMsg(DynastyAndOscillatingKings);

    if (rex_circe) 
      return VerifieMsg(DynastyAndCirceRexIncl);
    
    if (rex_immun)
      return VerifieMsg(DynastyAndImmun);
      
    if (nbpiece[roib]==1)
      for (bnp= boardnum; *bnp; bnp++) {
        s = *bnp;
        if (e[s] == roib) {
          rb = s;
          break;
        }
      }

    if (nbpiece[roin]==1)
      for (bnp= boardnum; *bnp; bnp++) {
        s = *bnp;
        if (e[s] == roin) {
          rn = s;
          break;
        }
      }
  }


  if (CondFlag[takemake])
  {
    if (CondFlag[sentinelles]
        || CondFlag[nocapture]
        || anyanticirce)
      return VerifieMsg(TakeMakeAndFairy);
  }

  for (bnp= boardnum; *bnp; bnp++) {
	p= e[*bnp];
	if (p != vide) {				/* V3.22  TLi */
      if (CondFlag[volage] && rb != *bnp && rn != *bnp) {
		SETFLAG(spec[*bnp], Volage);
      }
      /* V3.22	TLi, V3.50 SE */
      if ((PieSpExFlags >> DiaCirce) || flagdiastip) {
		SetDiaRen(spec[*bnp], *bnp);
      }
      if (TSTFLAG(spec[*bnp], ColourChange)) {	  /* V3.64 SE */
		if (!is_simplehopper(abs(e[*bnp]))) {
          /* relies on imitators already having been
             implemented */
          CLRFLAG(spec[*bnp], ColourChange);
          ErrorMsg(ColourChangeRestricted);
		}
      }
	}
  }

  flagleofamilyonly= CondFlag[leofamily] ? true : false;	/* V3.77  NG */
  for (p= fb + 1; p <= derbla; p++) {
	if (exist[p] || promonly[p]) {			/* V3.43  NG */
      flagfee= true;
      if (is_rider(p))				 /* V3.12 TM */
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
		if (!is_pawn(p) && p != dummyb && (p<leob || p>vaob))/* V3.77  NG */
          flagleofamilyonly= false;
		flagveryfairy= true;
      }

      if (CondFlag[einstein])			/* V3.1  TLi */
		return	VerifieMsg(EinsteinAndFairyPieces);
	}
  }

  if (CondFlag[sting]) {				/* V3.63  NG */
	totalortho= false;
	flagfee= true;
	flagsimplehoppers= true;
  }

  if (!CondFlag[noiprom])				 /* V2.4d  TM */
	for (n= 0; n <= maxply; Iprom[n++]= false)
      ;

  flaglegalsquare=
	TSTFLAG(PieSpExFlags, Jigger)
	|| CondFlag[newkoeko]				/* V3.1  TLi */
	|| CondFlag[gridchess] || CondFlag[koeko]
	|| CondFlag[blackedge] || CondFlag[whiteedge]
	|| CondFlag[geneva];				/* V4.38  NG */
  /* V2.70  TLi, V2.90  NG */
  if (CondFlag[imitators]) {				 /* V2.4d  TM */
	if (	flagveryfairy				 /* V3.12  TM */
            || flagsymmetricfairy           /* V3.81 SE these were in flagveryfairy, ok with mars but not with I yet */
            || flaglegalsquare
            || CondFlag[chinoises]
            || anyimmun			     /* V3.1  TLi, aber warum
                                        eigentlich nicht */
            || CondFlag[haanerchess]
            || anycirce           /* V4.07 SE - rebirth square may coincide with I */
            || anyanticirce       /* V4.07 SE - rebirth square may coincide with I */
            || CondFlag[parrain])      /* V3.02  TLi verkraftet
                                          nicht 2 IUW in einem Zug !!! */
	{
      return VerifieMsg(ImitWFairy);
	}
	totalortho= false;			/* V3.0  TLi */
  }

  if (CondFlag[leofamily]) {
	for (p= db; p <= fb; p++) {
      if (nbpiece[p] + nbpiece[-p] != 0) {	/* V1.4c  NG */
		return VerifieMsg(LeoFamAndOrtho);
      }
      exist[p]= false;
	}
  }

  if (CondFlag[chinoises]) {
	for (p= leob; p <= vaob; p++) {
      exist[p]= true;
	}
	flagfee= true;					 /* V1.5c  NG */
  }
  if (anycirce) {			     /* V2.1c  NG , V3.1  TLi */
	if (exist[dummyb]) {
      return VerifieMsg(CirceAndDummy);
	}
	if (TSTFLAG(PieSpExFlags, Neutral)) {		/* V3.0  TLi */
      totalortho= false;
	}
  }

  if (flagmaxi) {					/* V3.62 SE */
	CondFlag[blmax]= (maincamp==blanc);
	CondFlag[whmax] = (maincamp==noir);
	if (maincamp==blanc) {
      bl_ultra= CondFlag[ultra];
      bl_exact= CondFlag[exact];
      black_length= len_max;
      /*	 white_length= len_normal;	*/	 /* V3.62  SE */
      flagblackmummer= true;
      flagwhitemummer= false;
	}
	else {
      wh_ultra= CondFlag[ultra];
      wh_exact= CondFlag[exact];
      white_length= len_max;
      /*	 black_length= len_normal;	*/	/* V3.62  SE */
      flagwhitemummer= true;
      flagblackmummer= false;
	}
  }
  
  if (flagultraschachzwang) {				 /* V3.62 SE */
	CondFlag[blackultraschachzwang]= (maincamp==blanc);
	CondFlag[whiteultraschachzwang]= (maincamp==noir);
  }
  if (CondFlag[blackultraschachzwang]
      || CondFlag[whiteultraschachzwang])
  {
	totalortho= false;
  }

  if (CondFlag[cavaliermajeur]) {
	if (nbpiece[cb] + nbpiece[cn] > 0) {
      return VerifieMsg(CavMajAndKnight);
	}
	exist[cb]= false;
	exist[nb]= true;				/* V2.60  NG */
	flagfee= true;					/* V2.60  NG */
  }

  if (CondFlag[republican]) {				 /* V3.50 SE */

	OptFlag[sansrb]= OptFlag[sansrn]= True;
	totalortho= False;
	flag_dontaddk=false;
  }

  if (OptFlag[sansrb] && rb) {			/* V2.60  NG */
	OptFlag[sansrb]= False;
  }
  if (OptFlag[sansrn] && rn) {	    /* V2.1c  NG,  V2.60  NG */
	OptFlag[sansrn]= False;
  }
  if (rb==initsquare && nbpiece[roib]==0
      && !OptFlag[sansrb]) {	/* V2.60  NG */
	ErrorMsg(MissingKing);
  }
  if (rn==initsquare && nbpiece[roin]==0
      && !OptFlag[sansrn]) {/* V2.1c NG, V2.60 NG */
	ErrorMsg(MissingKing);
  }

  if (rex_circe) {		   /* V3.37  TLi */
	/* why not royal pieces in PWC ??? TLi */
	/* Because we loose track of the royal attribute somewhere and
	   I didn't find where ... NG
         */
	if (CondFlag[circeequipollents]
        || CondFlag[circeclone]
        || CondFlag[couscous]
        || CondFlag[circeclonemalefique]) {
      /* V3.55 SE disallowed because of the call to (*circerenai)
         in echecc */
      /* would require knowledge of id. Other forms now allowed
       */
      if (  ((! OptFlag[sansrb]) && rb!=initsquare && (e[rb] != roib))
            || ((! OptFlag[sansrn]) && rn!=initsquare && (e[rn] != roin)))
        /* V2.1c  NG */
      {
		return VerifieMsg(RoyalPWCRexCirce);
      }
	}
  }

  if (TSTFLAG(PieSpExFlags, Neutral)) {
	initneutre(blanc);
	flag_nk= rb!=initsquare
      && TSTFLAG(spec[rb], Neutral);	/* V3.50 SE */
  }

  /*****  V3.1  TLi  begin  *****/
  if (CondFlag[bicolores]) {
    if (TSTFLAG(PieSpExFlags, Neutral))
      return VerifieMsg(NeutralAndBicolor);
	totalortho= false;				/* V3.80  SE,NG */
  }
  /*****  V3.1  TLi  end  *****/

  eval_2= eval_white= eval_ortho;			/* V2.80  TLi */

  flaglegalsquare= flaglegalsquare			/* V2.70  TLi */
    || CondFlag[bichro]
    || CondFlag[monochro];

  if (flaglegalsquare) {
	eval_white= legalsquare;			/* V2.80  TLi */
	eval_2= eval_ortho;
	if (CondFlag[monochro] && CondFlag[bichro]) {
      return VerifieMsg(MonoAndBiChrom);
	}
	if (  (CondFlag[koeko]
	       || CondFlag[newkoeko]
	       || TSTFLAG(PieSpExFlags, Jigger))	 /* V3.1  TLi */
          && anycirce					 /* V3.1  TLi */
          && TSTFLAG(PieSpExFlags, Neutral))
	{
      return VerifieMsg(KoeKoCirceNeutral);
	}
  }

  if (flaglegalsquare || TSTFLAG(PieSpExFlags,Neutral)) { /* V2.70  TLi */
	if (CondFlag[volage]) {
      return VerifieMsg(SomeCondAndVolage);
	}
  }
  if (TSTFLAG(PieSpExFlags,Paralyse)			/* V3.32  TLi */
      && !(CondFlag[patrouille]
           || CondFlag[beamten]
           || TSTFLAG(PieSpExFlags, Beamtet)))	/* V3.53  TLi */
  {
	eval_2= eval_white;
	eval_white= paraechecc;
  }
  if (TSTFLAG(PieSpExFlags, Kamikaze)) {
	totalortho= false;				/* V3.0  TLi */
	if (CondFlag[haanerchess]) {
      return VerifieMsg(KamikazeAndHaaner);
	}
	if (anycirce /* CondFlag[circe] */ ) {		/* V3.1  TLi */
      /* No Kamikaze and Circe with fairy pieces; taking and
         taken piece could be reborn on the same square! */
      if (flagfee || CondFlag[volage]) {
		return VerifieMsg(KamikazeAndSomeCond);  /* V2.90  NG */
      }
	}
  }

  if ((CondFlag[supercirce] || CondFlag[april])
      && (CondFlag[koeko] || CondFlag[newkoeko]))
  {
	return	VerifieMsg(SuperCirceAndOthers);
  }

  {
    int numsuper=0;
    if (CondFlag[supercirce]) numsuper++;
    if (CondFlag[april]) numsuper++;
    if (CondFlag[republican]) numsuper++;
    if (CondFlag[antisuper]) numsuper++;
    if (numsuper > 1)
      return	VerifieMsg(SuperCirceAndOthers);
  }

  if (CondFlag[patrouille]			       /* V2.80  TLi */
      || CondFlag[beamten]			       /* V3.32  TLi */
      || CondFlag[central]			       /* V3.50 SE */
      || TSTFLAG(PieSpExFlags, Beamtet)	       /* V3.53  TLi */
      || CondFlag[ultrapatrouille])
  {
	eval_2= eval_white;
	eval_white= soutenu;
  }

  if (CondFlag[nowhcapture] && CondFlag[noblcapture]) {/* V3.64  NG */
	CondFlag[nocapture]= true;
	if (CondFlag[nocapture]) {			 /* V3.64  NG */
      CondFlag[nowhcapture]= false;
      CondFlag[noblcapture]= false;
	}
  }

  /* if (CondFlag[isardam] && CondFlag[madras])	V3.44  SE/TLi */
  if (CondFlag[isardam] && flag_madrasi) {		/* V3.60  TLi */
	return VerifieMsg(IsardamAndMadrasi);
  }

  if (CondFlag[black_oscillatingKs] || CondFlag[white_oscillatingKs]) {
    if (rb==initsquare || rn==initsquare)
      CondFlag[black_oscillatingKs]= CondFlag[white_oscillatingKs]= false;
    else
      totalortho= false;
  }
  if (CondFlag[black_oscillatingKs] && OscillatingKingsTypeC[blanc] &&
      CondFlag[white_oscillatingKs] && OscillatingKingsTypeC[blanc]) {	/* V3.81a  NG */
    CondFlag[swappingkings]= True;
  }

  if (anymars||anyantimars) {		    /* V3.46  SE/TLi */
	totalortho= false;
	/* empilegenre= true;*/ /* set also below ...  V3.64  NG */
	if (CondFlag[whtrans_king]
        || CondFlag[whrefl_king]
        || CondFlag[bltrans_king]
        || CondFlag[blrefl_king]
        || CondFlag[bicolores]
        || CondFlag[sting]
        || flagsimplehoppers
        || (flagveryfairy && !flagleofamilyonly) )	/* V3.77  NG */
	{
      return VerifieMsg(MarsCirceAndOthers);
	}
  }

  if (CondFlag[BGL]) /* V4.06 SE */
  {
    eval_white= eval_BGL;
    BGL_whiteinfinity= BGL_white == BGL_infinity;
    BGL_blackinfinity= BGL_black == BGL_infinity;
    totalortho= false;
  }

  /* if (CondFlag[madras] || CondFlag[isardam])	V3.44  SE/TLi */

  if (flag_madrasi || CondFlag[isardam]) {		/* V3.60  TLi */
	if ( CondFlag[imitators]
         || TSTFLAG(PieSpExFlags,Paralyse))
	{
      return VerifieMsg(MadrasiParaAndOthers);
	}
	if (!(CondFlag[patrouille]
          || CondFlag[beamten]				/* V3.32  TLi */
          || TSTFLAG(PieSpExFlags, Beamtet)))		/* V3.53  TLi */
	{
      eval_2= eval_white;
      eval_white= CondFlag[isardam]
        ?	eval_isardam
        : eval_madrasi;		/* V3.60  TLi */
	}
  }

  if (CondFlag[woozles]) {				/* V3.55  TLi */
	/* if (CondFlag[madras] || CondFlag[isardam] || */
	if ( flag_madrasi
         || CondFlag[isardam]				/* V3.60  TLi */
         || CondFlag[imitators]
         || TSTFLAG(PieSpExFlags,Paralyse))
	{
      return VerifieMsg(MadrasiParaAndOthers);
	}
	totalortho= false;
	eval_2= eval_white;
	eval_white= eval_wooheff;
  }

  if (CondFlag[brunner]) {				/* V3.50 SE */
	eval_white=eval_isardam;
  }
  if (CondFlag[isardam] && IsardamB) {		/* V3.50 SE */
	eval_white=eval_ortho;
  }
  if (CondFlag[shieldedkings]) {			/* V3.62 SE */
	eval_white=eval_shielded;
  }


  if (flagAssassin) {					/* V3.50 SE */
	if (TSTFLAG(PieSpExFlags,Neutral) /* Neutrals not implemented */
        || CondFlag[bicolores]) {			  /* others? */
      return VerifieMsg(AssassinandOthers);
	}
  }
  eval_black= eval_white;				    /* V2.80  TLi */
  /* nachfolgend ueberarbeitet da neu rex_mad/cir/imm   V2.70  TLi */
  if (rex_circe || rex_immun) {
	if (rex_circe && rex_immun) {
      return VerifieMsg(RexCirceImmun);
	}
	if (anyanticirce) {				/* V3.1  TLi */
      /* an additional pointer to evaluate-functions is
         required  TLi */
      return VerifieMsg(SomeCondAndAntiCirce);
	}
	eval_2= eval_white;
	if (rex_circe) {
      eval_white= rbcircech;
      eval_black= rncircech;
      cirrenroib= (*circerenai)(roib, spec[rb], initsquare, initsquare, initsquare, noir);	    /* V3.1  TLi */
      cirrenroin= (*circerenai)(roin, spec[rn], initsquare, initsquare, initsquare, blanc);	    /* V3.1  TLi */
	}
	else {
      eval_white= rbimmunech;
      eval_black= rnimmunech;
      immrenroib= (*immunrenai)(roib, spec[rb], initsquare, initsquare, initsquare, noir);	    /* V3.1  TLi */
      immrenroin= (*immunrenai)(roin, spec[rn], initsquare, initsquare, initsquare, blanc);	    /* V3.1  TLi */
	}
  }

  if (anyanticirce) {					/* V3.1  TLi */
	if (CondFlag[couscous]
        || CondFlag[koeko]
        || CondFlag[newkoeko]
        || (CondFlag[singlebox]	&& SingleBoxType==singlebox_type1) /* V3.71 TM */
	|| CondFlag[geneva]
        || TSTFLAG(PieSpExFlags, Kamikaze))
	{
      return VerifieMsg(SomeCondAndAntiCirce);
	}
	totalortho= false;
	eval_2= eval_white;
	eval_white= rbanticircech;
	eval_black= rnanticircech;
  }

  if ((CondFlag[singlebox]	&& SingleBoxType==singlebox_type1)) {     /* V3.71 TM */
	if (flagfee) {
      return VerifieMsg(SingleBoxAndFairyPieces);
	}
	totalortho= false;
	eval_2= eval_white;
	eval_white= rbsingleboxtype1ech;
	eval_black= rnsingleboxtype1ech;
  }

  if ((CondFlag[singlebox]	&& SingleBoxType==singlebox_type3)) {     /* V3.71 TM */
	totalortho= false;
	rnechec = &singleboxtype3_rnechec;
	rbechec = &singleboxtype3_rbechec;
	gen_wh_piece = &singleboxtype3_gen_wh_piece;
	gen_bl_piece = &singleboxtype3_gen_bl_piece;
  }

  if ((CondFlag[white_oscillatingKs] || CondFlag[black_oscillatingKs]) 
      && (OptFlag[sansrb] || OptFlag[sansrn]))
    return VerifieMsg(MissingKing);

  if (wh_ultra && !CondFlag[whcapt]) {	    /* V3.1  TLi */
	eval_2= eval_white;
	eval_black= rnultraech;
	if (TSTFLAG(PieSpExFlags, Neutral)) {
      return VerifieMsg(OthersNeutral);
	}
  }
  if (bl_ultra && !CondFlag[blcapt]) {	    /* V3.1  TLi */
	eval_2= eval_white;
	eval_white= rbultraech;
	if (TSTFLAG(PieSpExFlags, Neutral)) {
      return VerifieMsg(OthersNeutral);
	}
  }

  if ( ( CondFlag[whmin]
         + CondFlag[whmax]
         + CondFlag[whcapt]
         + (CondFlag[whforsqu] || CondFlag[whconforsqu])  /* V3.20TLi */
         + CondFlag[whfollow]
         + CondFlag[duellist]
         + CondFlag[alphabetic]
         + CondFlag[whitesynchron]
         + CondFlag[whiteantisynchron]) > 1		       /* V2.90c  TLi */
       || (CondFlag[blmin]
           + CondFlag[blmax]
           + CondFlag[blcapt]
           + (CondFlag[blforsqu] || CondFlag[blconforsqu]) /* V3.20 TLi */
           + CondFlag[blfollow]
           + CondFlag[duellist]
           + CondFlag[alphabetic]
           + CondFlag[blacksynchron]
           + CondFlag[blackantisynchron] > 1))			 /* V3.0  TLi */
  {
	return VerifieMsg(TwoMummerCond);
  }
#ifndef DATABASE  /* TLi */
  if ((CondFlag[whmin]
       || CondFlag[blmin]
       || CondFlag[whmax]
       || CondFlag[blmax]
       || CondFlag[heffalumps]
        )					       /* V3.55  TLi */
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
          || exist[raob]					/* V3.63  NG */
          || exist[rookmooseb]
          || exist[rookeagleb]
          || exist[rooksparrb]
          || exist[bishopmooseb]
          || exist[bishopeagleb]
          || exist[bishopsparrb]
          || exist[doublegb]))
  {
	return VerifieMsg(SomePiecesAndMaxiHeffa);
  }
#endif

  if (flagdiastip
      && (CondFlag[frischauf]				 /* V3.50 SE */
          || CondFlag[sentinelles]
          || CondFlag[imitators]))
  {
	return VerifieMsg(DiaStipandsomeCond);
  }

  jouegenre =			   /* V3.1  TLi */
	CondFlag[black_oscillatingKs]       /* V3.51 SE */
	|| CondFlag[white_oscillatingKs]    /* V3.51 SE */
	|| CondFlag[republican]	   /* V3.50 SE */
	|| anycirce
	|| CondFlag[sentinelles]
    /*	|| CondFlag[duellist]	*/	/* V3.74  NG */
	|| anyanticirce
	|| CondFlag[singlebox] /* V3.71  TM */
	|| CondFlag[blroyalsq]
	|| CondFlag[whroyalsq]			       /* V3.50  TLi */
    || CondFlag[dynasty]
    || CondFlag[strictSAT]
    || CondFlag[masand]
    || CondFlag[BGL]
    || CondFlag[duellist]
    || TSTFLAG(PieSpExFlags,HalfNeutral)
    || exist[Orphan]
    || exist[Friend]				/* V3.65  TLi */
    || CondFlag[whrefl_king] || CondFlag[blrefl_king]
    || CondFlag[phantom]
    || CondFlag[extinction]
    || CondFlag[amu]
    || CondFlag[imitators]
    || CondFlag[blsupertrans_king] || CondFlag[whsupertrans_king];


  change_moving_piece=				/* V3.1  TLi */
	TSTFLAG(PieSpExFlags, Kamikaze)
	|| CondFlag[tibet]
	|| CondFlag[andernach]
	|| CondFlag[antiandernach]
	|| CondFlag[magic]
	|| TSTFLAG(PieSpExFlags, Chameleon)
	|| CondFlag[einstein]
	|| CondFlag[volage]
	|| TSTFLAG(PieSpExFlags, Volage)
	|| CondFlag[degradierung]
	|| CondFlag[norsk]
	|| CondFlag[traitor]
	|| CondFlag[linechamchess]			/* V3.64  TLi */
	|| CondFlag[chamchess];				/* V3.32  TLi */

  repgenre =			     /* CondFlag[tibet] ||  V3.1  TLi */
	CondFlag[sentinelles]
	|| CondFlag[imitators]
	|| anycirce					 /* V3.1  TLi */
	|| TSTFLAG(PieSpExFlags, Neutral)
	|| (CondFlag[singlebox]	&& SingleBoxType==singlebox_type1) /* V3.71  TM */
	|| anyanticirce;				 /* V3.1  TLi */

  empilegenre=
	flaglegalsquare					 /* V2.60  NG */
	|| CondFlag[patrouille]
	|| CondFlag[ultrapatrouille]
	|| CondFlag[imitators]
	|| CondFlag[beamten]				/* V3.32  TLi */
	|| TSTFLAG(PieSpExFlags, Beamtet)		/* V3.53  TLi */
	|| CondFlag[central]				/* V3.50 SE */
	|| anyimmun
	|| CondFlag[nocapture]				/* V3.1  TLi */
	|| CondFlag[nowhcapture]			/* V3.64  NG */
	|| CondFlag[noblcapture]			/* V3.64  NG */
	/*|| TSTFLAG(PieSpExFlags, Kamikaze) && CondFlag[circe] */
	|| TSTFLAG(spec[rb], Kamikaze)
	|| TSTFLAG(spec[rn], Kamikaze)			/* V3.1  TLi */
	|| flagwhitemummer
	|| flagblackmummer
	|| TSTFLAG(PieSpExFlags, Paralyse)
	|| CondFlag[vogt]
	|| anyanticirce
	|| anymars				     /* V3.46  SE/TLi */
	|| anyantimars				     /* V3.46  SE/TLi */
	|| (CondFlag[singlebox]	&& SingleBoxType==singlebox_type1)/* V3.71  TM */
	|| CondFlag[messigny]				/* V3.55  TLi */
	|| CondFlag[woozles]				/* V3.55  TLi */
	|| CondFlag[nowhiteprom]			/* V3.64 NG */
	|| CondFlag[noblackprom]			/* V3.64 NG */
	|| CondFlag[antikings]         /* V3.78 SE */
	|| CondFlag[norsk]				/* V3.1  TLi */
  || CondFlag[SAT]                /* V4.03 SE */
  || CondFlag[strictSAT]
  || CondFlag[takemake];         /* V4.03 SE */
  /* V2.90, 3.03  TLi */

  if (CondFlag[dynasty]) { /* V4.02 TM */
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

  nonkilgenre= CondFlag[messigny]		 /* V3.45, V3.55 TLi */
    || (CondFlag[singlebox]	&& SingleBoxType==singlebox_type3) /* V3.71 TM */
    || CondFlag[whsupertrans_king]    /* V3.78  SE */	
    || CondFlag[blsupertrans_king]    /* V3.78  SE */	
    || CondFlag[takemake];

  if (TSTFLAG(PieSpExFlags, Jigger)		 /* V3.1  TLi */
      || CondFlag[newkoeko]			  /* V3.1  TLi */
      || CondFlag[koeko]			   /* V3.0  TLi */
      || CondFlag[parrain]			  /* V3.02  TLi */
      || flagwhitemummer
      || flagblackmummer
      || CondFlag[vogt]				  /* V3.03  TLi */
      || (   eval_white != eval_ortho
             && eval_white != legalsquare)
      || (rb != initsquare && abs(e[rb]) != King)  /* V3.1  TLi */
      || (rn != initsquare && abs(e[rn]) != King)
      || TSTFLAG(PieSpExFlags, Chameleon)	   /* V3.1  TLi */
      || CondFlag[einstein]			   /* V3.1  TLi */
      || CondFlag[degradierung]			   /* V3.1  TLi */
      || CondFlag[norsk]			   /* V3.1  TLi */
      || CondFlag[messigny]			   /* V3.55 TLi */
      || CondFlag[linechamchess]		   /* V3.64 TLi */
      || CondFlag[chamchess]			   /* V3.32 TLi */
      || CondFlag[antikings]         /* V3.78 SE */
      || TSTFLAG(PieSpExFlags, HalfNeutral) 	   /* V3.57 SE */
      || CondFlag[geneva]            /* V4.38  NG */
      || CondFlag[dynasty])           /* V4.02 TM */
  {
	totalortho= false;
  }

  superbas= CondFlag[antisuper] ? bas : bas - 1;     /* V3.78 SE */

  pp= cp= 0;	  /* init promotioncounter and checkcounter V2.60  NG */
  for (p= CondFlag[dynasty] ? roib : db; p <= derbla; p++) {			/* V2.60  NG */  /* V4.02 TM */
	getprompiece[p]= vide;
	if (promonly[p]) {				/* V3.42  NG */
      exist[p]= True;
	}
	if (exist[p]) {
      if ( p != pb
           && p != dummyb
           && p != pbb
           && p != bspawnb
           && p != spawnb
           && (!CondFlag[promotiononly] || promonly[p]))
      {					/* V3.42, V3.44  NG */
		getprompiece[pp]= p;
		pp= p;
      }
      if (p > fb && p != dummyb) {
        /* only fairy pieces until now ! V2.60  NG */
		totalortho= false;	/* V3.0  TLi, NG */
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
      transmpieces[tp++]= p;
      if ( p != Orphan
           && p != Friend
           && (exist[Orphan] || exist[Friend]))	/* V3.65  TLi */
      {
		orphanpieces[op++]= p;
      }
	}
  }
  transmpieces[tp]= vide;
  if (CondFlag[whrefl_king] || CondFlag[blrefl_king]) {/* V3.47  NG */
	totalortho= false;
  }
  orphanpieces[op]= vide;

  if (  (CondFlag[whrefl_king]
         && rb != initsquare
         && (e[rb] != roib || CondFlag[sting]))
        /* V3.39  TLi, V3.47	NG, V3.63  NG */
        || (CondFlag[blrefl_king]
            && rn != initsquare
            && (e[rn] != roin || CondFlag[sting])))
    /* V3.39  TLi, V3.47	NG, V3.63  NG */
  {
	return VerifieMsg(TransmRoyalPieces);
  }

  if ((exist[Orphan]
       || exist[Friend]				/* V3.65  TLi */
       || CondFlag[whrefl_king]
       || CondFlag[blrefl_king])			 /* V3.47  NG */
      && TSTFLAG(PieSpExFlags, Neutral))
  {
	return VerifieMsg(NeutralAndOrphanReflKing);
  }

  if ((eval_white==eval_isardam) && CondFlag[vogt]) {   /* v3.50 SE */
	return VerifieMsg(VogtlanderandIsardam);
  }

  for (n= 2; n <= maxply; inum[n++]= inum[1])
	;						 /* V2.4d  TM */

  if (  (CondFlag[chamchess] || CondFlag[linechamchess])
        /* V3.64  TLi */
        && TSTFLAG(PieSpExFlags, Chameleon))	/* V3.32,V3.64	TLi */
  {
	return VerifieMsg(ChameleonPiecesAndChess);
  }

  if (TSTFLAG(PieSpExFlags, ColourChange)) {		 /* V3.64 SE */
	checkhopim = true;
	totalortho = false;
	jouegenre = true;
  }
  checkhopim |= CondFlag[imitators];

  totalortho &= !CondFlag[sentinelles];
  /* a nasty drawback */

  if (CondFlag[annan]) {
    totalortho= false;
    rbechec= &annan_rbechec;
    rnechec= &annan_rnechec;
  }

  FlagMoveOrientatedStip =
	ReciStipulation == stip_target
	|| ReciStipulation == stip_ep
	|| ReciStipulation == stip_capture
	|| ReciStipulation == stip_steingewinn
	|| ReciStipulation == stip_castling	       /* V3.35  TLi */
	|| NonReciStipulation == stip_target
	|| NonReciStipulation == stip_ep
	|| NonReciStipulation == stip_capture
	|| NonReciStipulation == stip_steingewinn
	|| NonReciStipulation == stip_castling;        /* V3.35  TLi */

  if (   stipulation == stip_doublemate		/* V3.33  TLi */
         && (SortFlag(Self)
             || SortFlag(Direct)
             || (FlowFlag(Reci)
                 && CounterMate
                 && ReciStipulation == stip_doublemate)))
  {
	return VerifieMsg(StipNotSupported);
  }

  /* check castling possibilities */		    /* V3.35  NG */
  CLEARFL(castling_flag[0]);	/* TLi */
  /* castling_supported has to be adjusted if there are any problems */
  /* with castling and fairy conditions/pieces */
  castling_supported= !(
	/* Let's see if transmuting kings can castle without
	   problems ... */				 /* V3.51  NG */
	/* Unfortunately they can't ! So I had to exclude them
	   again ...  */				 /* V3.55  NG */
	/* A wK moving from anywhere to e1 and then like a queen from
	   e1 to g1 would get the castling right when this last move is
	   retracted  (:-( */
	/* ElB: need partenthese for vi-%-balancing: )) */
	/* transmuting kings and castling enabled again
	 */						/* V3.55  TLi */
	CondFlag[patience]			 /* V3.53  NG */
	|| CondFlag[parrain]
	|| CondFlag[haanerchess]);		 /* V3.35, V3.51  NG */

  complex_castling_through_flag= CondFlag[imitators]; /* V3.80  SE */

  if (castling_supported) {	                 /* V3.77  TLi */
	if ((abs(e[square_e1])== King) && TSTFLAG(spec[square_e1], White)
        && (!CondFlag[dynasty] || nbpiece[roib]==1)) /* V4.02 TM */
      SETFLAGMASK(castling_flag[0],ke1_cancastle);
	if ((abs(e[square_h1])== Rook) && TSTFLAG(spec[square_h1], White))
      SETFLAGMASK(castling_flag[0],rh1_cancastle);
	if ((abs(e[square_a1])== Rook) && TSTFLAG(spec[square_a1], White))
      SETFLAGMASK(castling_flag[0],ra1_cancastle);
	if ((abs(e[square_e8])== King) && TSTFLAG(spec[square_e8], Black)
        && (!CondFlag[dynasty] || nbpiece[roin]==1))  /* V4.02 TM */
      SETFLAGMASK(castling_flag[0],ke8_cancastle);
	if ((abs(e[square_h8])== Rook) && TSTFLAG(spec[square_h8], Black))
      SETFLAGMASK(castling_flag[0],rh8_cancastle);
	if ((abs(e[square_a8])== Rook) && TSTFLAG(spec[square_a8], Black))
      SETFLAGMASK(castling_flag[0],ra8_cancastle);
  }

  if (stipulation == stip_castling && !castling_supported) {
    /* V3.53  NG */
	return VerifieMsg(StipNotSupported);
  }

  castling_flag[0] &= no_castling;			/* V3.55  NG */
  castling_flag[2]= castling_flag[1]= castling_flag[0];
  /* At which ply do we begin ??  NG */

  /* no_castling added  V3.55  NG */
  testcastling=
    TSTFLAGMASK(castling_flag[0],whq_castling&no_castling)==whq_castling
    || TSTFLAGMASK(castling_flag[0],whk_castling&no_castling)==whk_castling
    || TSTFLAGMASK(castling_flag[0],blq_castling&no_castling)==blq_castling
    || TSTFLAGMASK(castling_flag[0],blk_castling&no_castling)==blk_castling;

  /* a small hack to enable ep keys	  V3.37  TLi */
  trait[1]= 2;

  if ( CondFlag[isardam]
       || CondFlag[ohneschach]			  /* V3.44  SE/TLi */
       || CondFlag[exclusive])			  /* V3.45  TLi */
  {
	flag_testlegality= true;
	totalortho= false;
  }

  if (!CondFlag[patience]) {	       /* needed because of twinning */
	PatienceB= false;
  }

  jouetestgenre=
	flag_testlegality
	|| flagAssassin   /* V3.50 SE */
	|| stipulation==stip_doublemate
	|| ReciStipulation==stip_doublemate
	|| CondFlag[patience]
	|| CondFlag[republican]
	|| CondFlag[blackultraschachzwang]
	|| CondFlag[whiteultraschachzwang]		 /* V3.62 SE */
	|| CondFlag[BGL];                      /* V4.06 SE */
  jouetestgenre_save= jouetestgenre;

  jouetestgenre1 = CondFlag[blackultraschachzwang]
	|| CondFlag[whiteultraschachzwang];		 


  nonoptgenre= TSTFLAG(PieSpExFlags, Neutral)		/* V3.02  TLi */
    || flag_testlegality			/* V3.45  TLi */
    || anymars				/* V3.62  TLi */
    || anyantimars				/* V3.78  SE */
    || CondFlag[brunner]          /* V3.60  TLi */
    || CondFlag[blsupertrans_king]    /* V3.78  SE */
    || CondFlag[whsupertrans_king]
    || CondFlag[republican]  /* V3.80  SE */
    || CondFlag[takemake];

  supergenre=
	CondFlag[supercirce]
	|| CondFlag[antisuper]
	|| CondFlag[april]
	|| CondFlag[republican];			    /* V3.50 */

  if (CondFlag[extinction] || flagAssassin) {		/* V3.52  NG */
	totalortho= false;
  }

  if (CondFlag[actrevolving] || CondFlag[arc]) {
    /* V3.53, 3.62  TLi, SE */
	jouegenre= true;
	totalortho= false;
  }

  if (anytraitor) {
	totalortho= false;
  }
#ifdef DEBUG	    /* V3.52  NG */
  printf("int: %s, mate: %s, stalemate: %s, "
         "castling: %s, fee: %s, orth: %s, "
         "help: %s, direct: %s, series: %s\n",
         OptFlag[intelligent]?"true":"false",
         stipulation == stip_mate?"true":"false",
         stipulation == stip_stale?"true":"false",
         testcastling?"true":"false",
         flagfee?"true":"false",
         totalortho?"true":"false",
         SortFlag(Help)?"true":"false",
         SortFlag(Direct)?"true":"false",
         FlowFlag(Series)?"true":"false");
#endif	    /* DEBUG */

  if ( OptFlag[intelligent]		 /* V3.52  NG, V3.62  TLi, NG */
       && (((stipulation != stip_mate) && (stipulation != stip_stale))
           || flagfee
           || SortFlag(Self)		/* V4.38  NG */
           || !(   SortFlag(Help)
                || (SortFlag(Direct) && FlowFlag(Series))
	       )))
  {
	return VerifieMsg(IntelligentRestricted);
  }

  if (InitChamCirce) {				/* V3.45  TLi */
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
    totalortho= false;
    nonoptgenre= true;
    WhiteStrictSAT[1]= echecc_normal(blanc);
    BlackStrictSAT[1]= echecc_normal(noir);
    satXY= WhiteSATFlights > 1 || BlackSATFlights > 1;
  }

  move_generation_mode_opti_per_couleur[blanc]
    = flagwhitemummer||nonoptgenre
    ? move_generation_optimized_by_killer_move
    : move_generation_optimized_by_nr_opponent_moves;
  move_generation_mode_opti_per_couleur[noir]
    = flagblackmummer||nonoptgenre
    ? move_generation_optimized_by_killer_move
    : move_generation_optimized_by_nr_opponent_moves;

  if (CondFlag[schwarzschacher])
  {
    totalortho= false;
	nonoptgenre= true;
  }

  if (CondFlag[takemake])
    totalortho= false;

  if (OptFlag[appseul])
    flag_appseul= true;

#ifndef DATABASE  /* V3.39  TLi */
  if (SortFlag(Proof)) {		       /* V3.36  TLi */
	return ProofVerifie();
  }
#endif

  return true;
} /* verifieposition */

numecoup sic_coup;
ply sic_ply;	/* V3.51 SE */

void current(coup *mov) {
  square sq = move_generation_stack[nbcou].arrival;

  /*------------------------------------- modified. V1.2c  NG */
  mov->tr=		    trait[nbply];		 /* V3.1  TLi */
  mov->cdzz =		    move_generation_stack[nbcou].departure;
  mov->cazz=		    sq;
  mov->cpzz=		    move_generation_stack[nbcou].capture;
  mov->pjzz=		    pjoue[nbply];
  mov->norm_prom=	    norm_prom[nbply];		/* V3.02  TLi */
  mov->ppri=		    pprise[nbply];
  mov->sqren=		    sqrenais[nbply];
  mov->cir_prom=	    cir_prom[nbply];		/* V3.02  TLi */

  if ((bl_exact && mov->tr == noir)
      || (wh_exact && mov->tr == blanc))
  {
	mov->echec= false;	/* A quick and dirty hack. But echecc */
  }
  /* destroys the 'current()' entry	  */		/* V3.20  NG */
  else if (CondFlag[isardam] || CondFlag[brunner] || SATCheck) {	/* V3.51 SE */
	if (flag_writinglinesolution) {
      tempcoup= nbcou;
      tempply= nbply;
      nbcou= sic_coup;
      nbply= sic_ply;
      mov->echec= echecc(advers(mov->tr));
      nbcou= tempcoup;
      nbply= tempply;
      /* Not such a quick hack, but probably dirtier! */
      /* May work for the above exact conditions too	*/
	}
	else {
      mov->echec= echecc(advers(mov->tr));
	}
  }
  else {
	mov->echec= echecc(advers(mov->tr));
  }
  mov->renkam= crenkam[nbply];
  mov->promi=  Iprom[nbply];				/* V2.4d  TM */
  mov->numi=	 inum[nbply] - (mov->promi ? 1 : 0);
  /* Promoted imitator will be output 'normally'
     from the next move on.				   V2.4d  TM */
  mov->sum=	isquare[0] - im0;			/* V2.4d  TM */
  mov->speci= jouespec[nbply];

  /* hope the following works with parrain too   TLi */
  mov->ren_spec=  spec[sqrenais[nbply]];		/* V3.50  TLi */
  mov->bool_senti= senti[nbply];			/* V2.90  TLi */
  mov->ren_parrain= ren_parrain[nbply];		/* V3.02  TLi */
  mov->bool_norm_cham_prom= norm_cham_prom[nbply];	/* V3.1  TLi */
  mov->bool_cir_cham_prom= cir_cham_prom[nbply];	/* V3.1  TLi */
  mov->pjazz=     jouearr[nbply];			/* V3.1  TLi */
  mov->repub_k=   repub_k[nbply];			/* V3.50 SE */
  mov->new_spec=  spec[sq];				/* V3.57 SE */
  mov->hurdle=    chop[nbcou];			/* V3.64 SE */
  mov->sb3where=  sb3[nbcou].where;			/* V3.71 TM */
  mov->sb3what= sb3[nbcou].what;			/* V3.71 TM */
  if (mov->sb3what!=vide && mov->sb3where==mov->cdzz) {
	mov->pjzz= mov->pjazz= mov->sb3what;
  }
  mov->sb2where= sb2[nbply].where;
  mov->sb2what= sb2[nbply].what;
  mov->mren= cmren[nbcou];
  mov->osc= oscillatedKs[nbply];
  /*------------------------------------- modified. V1.2c  NG */
}

void alloctab(smallint *n) {
  *n= ++(tabsol.nbr);
  tabsol.cp[*n]= tabsol.cp[*n-1];
}

void freetab(void) {				     /* H.D. 10.02.93 */
  --(tabsol.nbr);
}

void pushtabsol(smallint n) {
  if (++(tabsol.cp[n]) > tabmaxcp)                    /* V3.80 TLi */
	ErrorMsg(TooManySol);
  else
	current(&(tabsol.liste[tabsol.cp[n]]));
  coupfort();
}

smallint tablen(smallint t) {
  return tabsol.cp[t]-tabsol.cp[t-1];
}

boolean WriteSpec(Flags sp, boolean printcolours) {	/* V3.62  TLi */
  boolean ret= False;
  PieSpec spname;					/* V3.50  TLi */

  if (printcolours && !TSTFLAG(sp, Neutral))		/* V3.62  TLi */
	spname= White;					/* V3.50  TLi */
  else
	spname= Neutral;

  for (; spname < PieSpCount; spname++) {	       /* V3.50  TLi */
	if ( (spname != Volage || !CondFlag[volage])
         && TSTFLAG(sp, spname))
	{
      StdChar(tolower(*PieSpString[ActLang][spname]));
      ret= True;
	}
  }
  return ret;
} /* WriteSpec */

#ifdef DATABASE
extern boolean two_same_pieces;
#endif

void editcoup(coup *mov) {
  char    BlackChar= *GetMsgString(BlackColor);
  char    WhiteChar= *GetMsgString(WhiteColor);
  short   icount, diff;				/* V2.4d  TM */
  square sq;

  if (mov->cazz==nullsquare) return;      /* V3.70 SE */

  /* Did we castle ?? */				/* V3.35  NG */
  if (mov->cpzz == kingside_castling
      || mov->cpzz == queenside_castling)			/* V3.55  TLi */
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
  } else {	/* no, we didn't castle */
	if (mov->cpzz == messigny_exchange) {			/* V3.55  TLi */
      /* Messigny Chess */
      WritePiece(mov->pjzz);
      WriteSquare(mov->cdzz);
      StdString("<->");
      WritePiece(mov->ppri);
      WriteSquare(mov->cazz);
	}
	else {
      if (mov->sb3what!=vide) {			/* V3.71  TM */
		StdString("[");
		WriteSquare(mov->sb3where);
		StdString("=");
		WritePiece(mov->sb3what);
		StdString("]");
      }
      if (WriteSpec(mov->speci, vide)
	      || (mov->pjzz != pb && mov->pjzz != pn))	/* V3.31  NG */
      {
		WritePiece(mov->pjzz);
      }
#ifdef DATABASE
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
      if (mov->cpzz != mov->cazz) {		/* V3.01  NG */
		if (is_pawn(mov->pjzz) && !CondFlag[takemake]) {
/* V3.31  NG */
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

	if (mov->bool_norm_cham_prom) {			/* V3.1  TLi */
      SETFLAG(mov->speci, Chameleon);
	}

	if ((mov->pjzz != mov->pjazz)
        || ((mov->speci != mov->new_spec) && (mov->new_spec != 0)))
	{
      /* V3.62  TLi */
      if (mov->pjazz == vide) {
		if (mov->promi) {			/* V2.4d  TM */
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

	if (mov->sqren != initsquare) {			/* V2.70  TLi */
      piece   p= CondFlag[antieinstein]
        ? inc_einstein(mov->ppri)	/* V3.50  TLi */
        : CondFlag[parrain]
        ? mov->ren_parrain		/* V3.1  TLi */
        : CondFlag[chamcirce]
        ? ChamCircePiece(mov->ppri) /* V3.45  TLi */
        : (anyclone && abs(mov->pjzz) != roib)
        ? -mov->pjzz
        : (anytraitor && abs(mov->ppri) >= roib)
        ? -mov->ppri
        : mov->ppri;
      StdString(" [+");
      WriteSpec(mov->ren_spec, p);		 /* V3.1  TLi */
      WritePiece(p);

      WriteSquare(mov->sqren);
      if (mov->bool_cir_cham_prom) {		 /* V3.1  TLi */
		SETFLAG(mov->ren_spec, Chameleon);
      }
      if (mov->cir_prom) {			/* V1.6c  NG */
		StdChar('=');
		WriteSpec(mov->ren_spec, p);
		WritePiece(mov->cir_prom);
      }
      /* V3.1  TLi */
      if (TSTFLAG(mov->ren_spec, Volage)
	      && SquareCol(mov->cpzz) != SquareCol(mov->sqren))
      {
		sprintf(GlobalStr, "=(%c)",
                (mov->tr == blanc) ? WhiteChar : BlackChar);
		StdString(GlobalStr);
      }
      StdChar(']');
	}

	if (mov->sb2where!=initsquare) {		/* V3.71 TM */
      assert(mov->sb2what!=vide);
      StdString(" [");
      WriteSquare(mov->sb2where);
      StdString("=");
      WritePiece(mov->sb2what);
      StdString("]");
	}

	/* V3.50 SE */
	if (CondFlag[republican]
        && (sq= mov->repub_k) <= haut
        && sq >= bas)
	{
      SETFLAG(mov->ren_spec, mov->tr==blanc ? Black : White);
      StdString("[+");
      WriteSpec(mov->ren_spec, roib);
      WritePiece(roib);
      WriteSquare(sq);
      StdChar(']');
	}

	if (mov->renkam) {	   /* V2.4d  TM, V2.60	NG, V3.1  TLi */
      StdChar('[');
      WriteSpec(mov->speci, mov->pjazz);
      WritePiece(mov->pjazz);
      WriteSquare(mov->cazz);
      StdString("->");
      WriteSquare(mov->renkam);
      if (mov->norm_prom != vide &&
          (!anyanticirce || (CondFlag[antisuper] && is_pawn(mov->pjzz) && !PromSq(mov->tr, mov->cazz)))) {
        /* V3.1, V3.62  TLi */
		StdChar('=');
		WriteSpec(mov->speci, mov->tr == blanc ? 1 : -1);
		WritePiece(mov->norm_prom);
      }
      StdChar(']');
	}
	if (mov->bool_senti) {				/* V2.90 TLi */
      StdString("[+");
      StdChar((!SentPionNeutral || !TSTFLAG(mov->speci, Neutral))
              ?  ((mov->tr==blanc) != SentPionAdverse
                  ? WhiteChar
                  : BlackChar)
              : 'n');
      /* V3.1  TLi */ /* V3.50 SE */
      WritePiece(sentinelb); WriteSquare(mov->cdzz);
      StdChar(']');
	}
	if (TSTFLAG(mov->speci, ColourChange)
        && (abs(e[mov->hurdle])>roib))		/* V3.64 SE */
	{
      couleur hc= e[mov->hurdle] < vide ? noir : blanc;
      StdString("[");
      WriteSquare(mov->hurdle);
      StdString("=");
      StdChar(hc == blanc ? WhiteChar : BlackChar);
      StdString("]");
	}
  } /* No castling */					/* V3.35  NG */
  /* Anyway ... */					/* V3.80  NG */
  if (mov->numi && CondFlag[imitators]) {		/* V2.4d  TM */
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
  if (mov->osc) {					/* V3.80  SE */
	StdString("[");
	StdChar(WhiteChar);
	WritePiece(roib);
	StdString("<>");
	StdChar(BlackChar);
	WritePiece(roib);
	StdString("]");
  }
  if (CondFlag[BGL]) /* V4.06 SE */
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
  if (flende) {					/* V2.90  TLi */
	if (stipulation == stip_mate_or_stale) {	/* V3.60 SE */
      if (mate_or_stale_patt)
		strcpy(AlphaEnd, " =");
      else
		strcpy(AlphaEnd, " #");
	}
	StdString(AlphaEnd);
  }
  else {
	if (mov->echec)
      StdString(" +");
  }
  flende= false;
  StdChar(bl);
} /* editcoup */

boolean nowdanstab(smallint n)
{
  smallint i;
  coup mov;

  current(&mov);
  for (i = tabsol.cp[n-1]+1; i <= tabsol.cp[n]; i++) {
	if ( mov.cdzz == tabsol.liste[i].cdzz
         && mov.cazz == tabsol.liste[i].cazz
         && mov.norm_prom == tabsol.liste[i].norm_prom
         && mov.cir_prom == tabsol.liste[i].cir_prom
         && mov.bool_cir_cham_prom
	     == tabsol.liste[i].bool_cir_cham_prom	/* V3.1  TLi */
         && mov.bool_norm_cham_prom
	     == tabsol.liste[i].bool_norm_cham_prom
         && mov.sb3where==tabsol.liste[i].sb3where	  /* V3.71 TM */
         && mov.sb3what==tabsol.liste[i].sb3what	  /* V3.71 TM */
         && mov.sb2where==tabsol.liste[i].sb2where	  /* V3.71 TM */
         && mov.sb2what==tabsol.liste[i].sb2what	  /* V3.71 TM */
         && mov.hurdle==tabsol.liste[i].hurdle
         && (!CondFlag[takemake] || mov.cpzz==tabsol.liste[i].cpzz)
         && (!supergenre				  /* V3.50 SE */
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

void videtabsol(smallint t) {
  smallint n;

  if (tabsol.cp[t] != tabsol.cp[t-1]) {
	Tabulate();
	Message(But);
	for (n = tabsol.cp[t]; n > tabsol.cp[t-1]; n--) {
      Tabulate();
      StdString("  1...");
      editcoup(&(tabsol.liste[n]));
      StdString(" !\n");
	}
  }
  StdChar('\n');
}

void linesolution(void) {
  smallint	    num= 0;
  couleur	    camp;

  sic_coup= nbcou;
  sic_ply= nbply;

#ifndef DATABASE
  if (OptFlag[intelligent]) {   /* V3.45  TLi */
	if (SolAlreadyFound()) {
      return;
	} else {
      if (OptFlag[maxsols])		/* V3.77  NG */
		solutions++;
      if (OptFlag[beep])			/* V3.77  NG */
		BeepOnSolution(maxbeep);
	}
	StoreSol();
  } else {
    if (OptFlag[maxsols])		/* V3.77  NG */
      solutions++;
	if (OptFlag[beep])			/* V3.77  NG */
      BeepOnSolution(maxbeep);
  }
#endif

  flag_writinglinesolution= true;	 /* V3.44  TLi */
  repere[nbply + 1]= nbcou;
  camp= trait[nbply= 2];
  ResetPosition();
  if (   ((!flag_atob && flag_appseul) || SatzFlag)
         && !FlowFlag(Intro))		  /* V3.44  TLi, V3.70	SE,NG */
  {
	StdString("  1...");
	num= 1;
	if (flag_appseul && SatzFlag && !flag_atob)
      StdString("  ...");
	else
      camp= advers(camp);
  }

  /* seriesmovers with introductory move  V3.31 TLi */
  if (FlowFlag(Intro) && !SatzFlag) {
	StdString("  1.");
	num= 1;
	nbcou= repere[nbply+1];
  	initneutre(advers(trait[nbply]));	/* V4.03  SE */
	jouecoup_no_test();
	ecritcoup();
	nbply++;
	camp= advers(camp);
  }

  while (nbply <= sic_ply) {
	if (FlowFlag(Intro)
        && trait[nbply] != camp			/* V3.44  TLi */
        && nbply < sic_ply)				/* V3.56  TLi */
	{
      camp= advers(camp);
      num= 0;
	}
	if (trait[nbply] == camp) {			/* V3.44  TLi */
      sprintf(GlobalStr,"%3d.",++num);
      StdString(GlobalStr);
	}
	flende= sic_ply == nbply;
	nbcou= repere[nbply + 1];
	initneutre(advers(trait[nbply]));	/* V4.03  SE */
	jouecoup_no_test();
	ecritcoup();
	nbply++;
  }
  Message(NewLine);
  nbcou= sic_coup;
  nbply= sic_ply;

  flag_writinglinesolution= false;			/* V3.44  TLi */
} /* end of linesolution */

EXTERN smallint WhMovesLeft, BlMovesLeft;      /* V3.45  TLi */

#ifndef DATABASE
boolean last_h_move(couleur camp) {
  couleur ad= advers(camp);
  boolean flag= false;

  /* double mate   V3.32  TLi */
  if (DoubleMate) {
	if (CounterMate) {
      if (!stip_mate(ad)) {
		return false;
      }
	}
	else {
      if (patt(camp)) {
		return false;
      }
	}
  }

  if (!(SortFlag(Self) && SortFlag(Help))) {		/* V3.0  TLi */
	GenMatingMove(camp);
  }
  else {
	if (SortFlag(Reflex) && !FlowFlag(Semi) && matant(camp, 1)) {	/* V4.07  NG */
 		return false;
	} else {
		genmove(camp);
	}
  }

  if (camp == blanc)
	WhMovesLeft--;
  else
	BlMovesLeft--;

  while (encore()) {
	if (jouecoup()				   /* V3.44  SE/TLi */
        && (!OptFlag[intelligent] || MatePossible()))
	{
      /* V3.45  TLi */
      if (SortFlag(Self) && SortFlag(Help)) {
		if (! echecc(camp) && dsr_e(ad,1)) {
          GenMatingMove(ad);			 /* V3.37  NG */
          while (encore()) {
			if (jouecoup()
                && (*stipulation)(ad))     /* V3.44  SE/TLi */
			{
              flag = true;		/* V3.13  TLi */
              linesolution();
              /* solutions++; V3.76  TLi */
			}
			repcoup();
          }
          finply();
		}
      }
      else {
		if ((*stipulation)(camp)) {
          flag= true;
          linesolution();
          /* solutions++; V3.76  TLi */
		}
      }
	}
	repcoup();
  }

  if (camp == blanc)
	WhMovesLeft++;
  else
	BlMovesLeft++;

  finply();
  return flag;
}

smallint dsr_def(couleur camp, smallint n, smallint t) {
  couleur ad= advers(camp);
  boolean pat= true;
  integer ntcount=0;

  if ((!FlowFlag(Exact) || enonce == 1)
      && SortFlag(Direct)
      && (*stipulation)(ad))		  /* to find short solutions */
  {
	return -1;
  }

  if (SortFlag(Reflex) && matant(camp,1)) {		/* V3.73  TLi */
	return 0;
  }

  if ( n > droh
       && !echecc(camp)
       && !((droh > 0) && dsr_ant(ad,droh)))
  {
    return(maxdefen + 1);
  }

  if (n > 2 && OptFlag[solflights]) {			/* V3.12  TLi */
	integer zae = maxflights + 1;
	square	x = camp == noir ? rn : rb;
	genmove(camp);
	while (encore() && (zae > 0)) {
      if (jouecoup()
	      && (x != (camp == noir ? rn : rb)))    /* V3.44  SE/TLi */
      {
		if (!echecc(camp)) {
          zae--;
		}
      }
      repcoup();
	}
	finply();
	if (zae == 0) {
      return maxdefen + 1;
	}
  }

  /* Check whether black has more non trivial moves than he is
     allowed to have. The number of such moves allowed
     (NonTrivialNumber) is entered using the nontrivial option.
  */
  if (n > NonTrivialLength) {				/* V3.12  TLi */
	ntcount= -1;
	/* Initialise the counter. It is counted down. */

	/* generate a ply */
	genmove(camp);

	/* test all possible moves */
	while (encore() && NonTrivialNumber >= ntcount) {
      /* Test whether the move is legal and not trivial. */
      if (jouecoup()
	      && !echecc(camp)
	      && !((NonTrivialLength > 0)
               && dsr_ant(ad, NonTrivialLength)))/* V3.44 SE/TLi */
      {
		/* The move is legal and not trivial. Hence
		   increment the counter.
        */
		ntcount++;
      }
      repcoup();
	}
	finply();

	if (NonTrivialNumber < ntcount) {
      return (maxdefen+1);
	}
	NonTrivialNumber -= ntcount;
  }

  if (n>2)
    move_generation_mode= move_generation_mode_opti_per_couleur[camp];

  genmove(camp);
  move_generation_mode= move_generation_optimized_by_killer_move;

  while (encore() && tablen(t) <= maxdefen) {
	if (jouecoup() && !echecc(camp)) {   /* V3.44  SE/TLi */
      pat= false;
      if(!dsr_ant(ad,n)) {
		pushtabsol(t);
      }
	}
	repcoup();
  }
  finply();

  if (n > NonTrivialLength) {
	NonTrivialNumber += ntcount;
  }

  return pat ? (maxdefen + 1) : tablen(t);
} /* dsr_def */

boolean dsr_parmena(couleur camp, smallint n, smallint t) {
  smallint zaehler= 0;
  boolean flag= false;
  couleur ad= advers(camp);

  if (!tablen(t)) {
	return true;
  }

  genmove(camp);
  while (encore() && !flag) {
	if (jouecoup() && nowdanstab(t) && !echecc(camp)) {
      /* V3.44  TLi */
      flag= !(  n == 1
                && SortFlag(Direct)
                ? (*stipulation)(camp)
                : dsr_e(ad, n));
      if (flag) {
		coupfort();
      }
      else {
		zaehler++;
      }
	}
	repcoup();
  }
  finply();
  return zaehler < tablen(t);
}

void dsr_vari(couleur camp, smallint n, smallint par, boolean appa) {
  couleur	ad= advers(camp);
  smallint	mats, mena, y, nrmena= 1, i, ntcount;
  boolean	indikator;

  VARIABLE_INIT(ntcount);

  if (!SortFlag(Direct) && (n == 1 || (appa && dsr_e(ad,1)))) {
	genmove(ad);
	while(encore()) {
      if (jouecoup() && (*stipulation)(ad)) {  /* V3.44  SE/TLi */
		StdString("\n");
		Tabulate();
		sprintf(GlobalStr,"%3d...",zugebene);
		if (zugebene == 1) {
          if  (OptFlag[maxsols]) 
            solutions++;
          if (OptFlag[beep])		/* V3.77  NG */
            BeepOnSolution(maxbeep);
		}
		StdString(GlobalStr);
		flende= true;
		ecritcoup();
      }
      repcoup();
      if (zugebene == 1) {			 /* V3.53  NG */
		if ((OptFlag[maxsols] && (solutions >= maxsolutions))
            || FlagTimeOut)		       /* V3.54  NG */
		{
          break;
		}
      }
	}
	finply();
	StdString("\n");
	return;
  }
  if ( !OptFlag[solvariantes]
       || (SortFlag(Direct) && n == 1))
  {
	Message(NewLine);
	return;
  }

  n--;
  alloctab(&mena);
  if (appa || OptFlag[nothreat] || echecc(ad)) {
	StdString("\n");
  }
  else {
	y = n > droh ? droh : n;
	DrohFlag= true;
	marge+= 4;
	for (i= 1;i <= y;i++) {
      dsr_sol(camp,i,mena,False);  /* V3.44  TLi */
      if (tablen(mena)) {
		nrmena= i;
		break;
      }
	}
	marge-= 4;
	if (DrohFlag) {
      Message(Zugzwang);
      DrohFlag= false;
	}
  }

  /* Update NonTrivial status */
  if (n > NonTrivialLength) {   /* V3.62  TLi */
	ntcount= -1;		  /* V3.63  TLi */
	/* generate a ply */
	genmove(ad);

	/* test all possible moves */
	while (encore()) {
      /* Test whether the move is legal and not trivial. */
      if (  jouecoup()
            && !echecc(ad)
            && !((NonTrivialLength > 0)
                 && dsr_ant(camp, NonTrivialLength)))  /* V3.44 SE/TLi */
      {
		/* The move is legal and not trivial.
		** Increment the counter. */
		ntcount++;
      }
      repcoup();
	}
	finply();

	NonTrivialNumber -= ntcount;
  } /* nontrivial */

  genmove(ad);
  while(encore()) {
	if (jouecoup() && !echecc(ad) && (!nowdanstab(par))) {
      indikator=
		appa
		? dsr_ant(camp,n)	  /* V3.39  TLi */
		: OptFlag[noshort]
        ? !dsr_ant(camp,n-1)
        : nrmena < 2 || !dsr_ant(camp,nrmena-1);

      if (!SortFlag(Direct) && indikator) {
		indikator= !(*stipulation)(ad);
      }
      if (indikator && dsr_parmena(camp,nrmena,mena)) {
		Tabulate();
		sprintf(GlobalStr,"%3d...",zugebene);
		StdString(GlobalStr);
		ecritcoup();
		StdString("\n");
		marge+= 4;
		for (i= FlowFlag(Exact) ? n : nrmena; i <= n; i++) {
          alloctab(&mats);
          dsr_sol (camp,i,mats, False);   /* V3.44  TLi */
          freetab();
          if (tablen(mats)) {
			break;
          }
		}
		if (!tablen(mats)) {		   /* V3.41  TLi */
          marge+= 2;
          Tabulate();
          Message(Refutation);
          marge-= 2;
		}
		marge-= 4;
      }
	}
	repcoup();
  }
  finply();
  freetab();

  if (n > NonTrivialLength) {
	NonTrivialNumber += ntcount;
  }
} /* dsr_vari */

void dsr_sol(
  couleur	camp,
  smallint	n,
  smallint	t,
  boolean	restartenabled)				/* V3.44  TLi */
{
  couleur ad= advers(camp);
  smallint nbd, def;

  if ((n == enonce) && !FlowFlag(Semi) && SortFlag(Reflex)) {
    /* V3.44  TLi */
	if (matant(camp,1)) {
      if (stipulation == stip_mate_or_stale) {	  /* V3.60 SE */
		if (mate_or_stale_patt)
          strcpy(AlphaEnd, " =");
		else
          strcpy(AlphaEnd, " #");
      }
      sprintf(GlobalStr, "%s1:\n", AlphaEnd);    /* V3.37  TLi */
      StdString(GlobalStr);		       /* V3.37  TLi */
      StipFlags|= SortBit(Direct);
      StipFlags|= FlowBit(Semi);
      alloctab(&def);
      dsr_sol(camp,1,def,False);			/* V3.44  TLi */
      freetab();
      return;
	}
  }

  zugebene++;
  genmove(camp);
  while (encore()) {
	if (jouecoup()
        && !(restartenabled && MoveNbr < RestartNbr)	/* V3.44  TLi */
        && (!echecc(camp)) && (!nowdanstab(t)))    /* V3.44  SE/TLi */
	{
      alloctab(&def);
      if (n == 1 && SortFlag(Direct)) {
		nbd= (*stipulation)(camp) ? 0 : maxdefen + 1;
      }
      else if (n == 1
               && OptFlag[quodlibet]
               && (*stipulation)(camp))		/* V3.60 SE */
      {
		nbd = 0;
      }
      else {
		nbd= zugebene == 1
          ? dsr_def(ad,n-1,def)		/* V3.64  TLi */
          : dsr_e(ad,n)
          ? 0
          : maxdefen+1;
      }

      if (nbd <= maxdefen) {
		flende= (n == 1 && SortFlag(Direct))
          || nbd == -1
          || (n == 1
              && OptFlag[quodlibet]
              && (*stipulation)(camp));	  /* V3.60 SE */
		if (DrohFlag) {
          Message(Threat);
          DrohFlag= false;
		}
		Tabulate();
		sprintf(GlobalStr,"%3d.",zugebene);
		StdString(GlobalStr);
		ecritcoup();
		if (zugebene == 1) {
          /* V3.1  TLi */
          if (nbd < 1) {
			StdString("! ");
			if (OptFlag[maxsols])		/* V3.77  NG */
              solutions++;
			if (OptFlag[beep])		/* V3.77  NG */
              BeepOnSolution(maxbeep);
          }
          else {
			StdString("? ");
          }
		}
		marge+= 4;
		dsr_vari(camp,n,def,false);
		if (zugebene == 1) {
          videtabsol(def);
		}
		marge-= 4;
      }
      freetab();
      if (nbd <= maxdefen) {
		pushtabsol(t);
      }
    }
	if (restartenabled) {			       /* V3.44  TLi */
      IncrementMoveNbr();
	}
	repcoup();
	if (zugebene == 1) {				/* V3.53  NG */
      if (   (OptFlag[maxsols] && (solutions >= maxsolutions))
             || FlagTimeOut				/* V3.54  NG */
	    )
	    break;
	}
  }
  zugebene--;
  finply();
} /* dsr_sol */

boolean dsr_e(couleur camp, smallint n)
{
  if (SortFlag(Direct))
    return (mate(camp,n-1));
  return (!definvref(camp,n));
}

boolean dsr_ant(couleur camp, smallint n)
{
  if (SortFlag(Direct))
    return (matant(camp,n));
  return (invref(camp,n));
}

void SolveSeriesProblems(couleur camp) {		/* V3.32  TLi */
  boolean	is_exact= FlowFlag(Exact);
  int		i;

  move_generation_mode= move_generation_not_optimized;

  flag_appseul= False;   /* V3.44  TLi
                                -- no meaning in series movers would
                                only distort output */

  if (SortFlag(Help)) {	  /* V3.44  TLi */
	camp= advers(camp);	    /* V3.44  TLi */
  }

  if (FlowFlag(Intro)) {
	/* seriesmovers with introductory move	V3.31  TLi */

	camp= advers(camp);				/* V3.44  TLi */
	/* check whether a king can be captured */
	if (  (OptFlag[solapparent] && echecc(camp))
          || echecc(advers(camp)))
	{
      ErrorMsg(KingCapture);
      return;
	}
	introseries(camp, introenonce, OptFlag[movenbr]);
  }
  else {
	if (   OptFlag[solapparent]
           && !SortFlag(Direct) && !OptFlag[restart])
	{
      /* V3.44  TLi */
      SatzFlag= True;
      if (echecc(camp)) {
		ErrorMsg(KingCapture);
      }
      else {
		if (SortFlag(Help)) {
          last_h_move(advers(camp));
		}
		else {
          zugebene++;
          dsr_vari(camp, 1, 0, True);
          zugebene--;
		}
      }
      SatzFlag= False;
      Message(NewLine);
	}

    if (   OptFlag[maxsols]    /* V3.78  SE reset after set play */
           /*&& (solutions >= maxsolutions)*/)
      solutions= 0;

	if (echecc(advers(camp))) {
      ErrorMsg(KingCapture);
	}
	else {
      int starti= FlowFlag(Exact)
        || OptFlag[restart] ? enonce : 1;
      /* V3.44  TLi */
      if (OptFlag[intelligent]) {
		for (i = starti; i <= enonce; i++) {	/* V3.44  TLi */
          if (SortFlag(Help)
              ? Intelligent(1, i, shsol, camp, i)
              : Intelligent(i, 0, ser_dsrsol, camp, i))
          {
			StipFlags |= FlowBit(Exact);
			if (OptFlag[stoponshort] && (i < enonce)) {
              /* V3.60  NG */
              FlagShortSolsReached= true;
              break;
			}
          }
		}
      }
      else {
		for (i = starti; i <= enonce; i++) {	/* V3.44  TLi */
          boolean restartenabled;
          restartenabled= OptFlag[movenbr] && i == enonce;

          if (SortFlag(Help)
              ? shsol(camp, i, restartenabled)
              : ser_dsrsol(camp, i, restartenabled))
          {
			StipFlags |= FlowBit(Exact);
			if (OptFlag[stoponshort]&& (i < enonce)) {
              /* V3.60  NG */
              FlagShortSolsReached= true;
              break;
			}
          } /* SortFlag(Help) */
		} /* for i */
      } /* OptFlag[intelligent] */
	} /* echecs(advers(camp)) */
  } /* FlowFlag(Intro) */

  if (!is_exact) {
	StipFlags &= ~FlowBit(Exact);
  }
} /* SolveSeriesProblems */

void SolveHelpProblems(couleur camp) {			/* V3.32  TLi */
  smallint	    n= 2*enonce, i;
  boolean	    is_exact= FlowFlag(Exact);

  move_generation_mode= move_generation_not_optimized;

  if (SortFlag(Self)) {
	n--;
	camp= advers(camp);
  }

  if (flag_appseul) {
	n--;
	camp= advers(camp);
  }

  if (OptFlag[solapparent]) {
	SatzFlag= True;
	if (echecc(advers(camp))) {
      ErrorMsg(KingCapture);
	}
	else {
      /* we are looking for shortest set plays only */
      int starti;

      starti= FlowFlag(Exact)
        || OptFlag[restart] ? n-1 : ((n-1)&1 ? 1 : 2);

      if (OptFlag[intelligent]) {
		WhMovesLeft= BlMovesLeft= starti/2;
		if (starti & 1) {
          WhMovesLeft++;
		}
		for (i= starti	; i <= n-1; i+=2) {	/* V3.44  TLi */
          boolean flag;
          flag= Intelligent(WhMovesLeft,
                            BlMovesLeft, mataide, camp, i);

          WhMovesLeft++; BlMovesLeft++;	/* V3.45  TLi */
          if (flag) {
			break;
          }
		}
      }
      else {
		for (i= starti; i <= n-1; i+=2) {	/* V3.44  TLi */
          if (mataide(camp, i, OptFlag[movenbr] && i == n-1))
			break;
		}
      }
	}
	StdChar('\n');
	SatzFlag= False;
  }

  if (   OptFlag[maxsols]    /* V3.78  SE reset after set play */
         /*&& (solutions >= maxsolutions)*/)
    solutions= 0;

  if (echecc(camp)) {
	ErrorMsg(KingCapture);
  }
  else {
	/* mataide(advers(camp), n);  V3.44  TLi */
	int starti= FlowFlag(Exact)
      || OptFlag[restart] ? n : (n&1 ? 1 : 2);

	if (OptFlag[intelligent]) {
      WhMovesLeft= BlMovesLeft= starti/2;
      if (starti & 1) {
		WhMovesLeft++;
      }

      for (i= starti; i <= n; i+=2) {		/* V3.44  TLi */
		if (Intelligent(WhMovesLeft,
                        BlMovesLeft, mataide, advers(camp), i))
		{
          StipFlags |= FlowBit(Exact);
          if (OptFlag[stoponshort] && (i < n)) {
            /* V3.60  NG */
			FlagShortSolsReached= true;
			break;
          }
		}
		WhMovesLeft++; BlMovesLeft++;	/* V3.45  TLi */
      }
	}
	else {
      for (i= starti  ; i <= n; i+=2) {		/* V3.44  TLi */
		if (mataide(advers(camp), i, OptFlag[movenbr]
                    && i == n))
		{
          /* Exact has to be set to find ALL longer solutions */
          StipFlags |= FlowBit(Exact);
          if (OptFlag[stoponshort] && (i < n)) {
            /* V3.60  NG */
			FlagShortSolsReached= true;
			break;
          }
		}
      }
	} /* OptFlag[intelligent] */
  }

  if (!is_exact) {
	StipFlags &= ~FlowBit(Exact);
  }
} /* SolveHelpProblems */

void SolveDirectProblems(couleur camp) {		/* V3.32  TLi */
  smallint lsgn;

  if (  (OptFlag[solapparent] && (enonce > 1))
        || OptFlag[postkeyplay])			/* V3.41  TLi */
  {
	if (echecc(camp)) {
      ErrorMsg(SetAndCheck);
	}
	else {
      alloctab(&lsgn);
      zugebene++;
      /* dsr_vari(camp, enonce, lsgn, true);    V3.41  TLi */
      dsr_vari(camp, enonce, lsgn, !OptFlag[postkeyplay]);
      /* V3.41  TLi */
      zugebene--;
      freetab();
      Message(NewLine);
	}
  }
  if (!OptFlag[postkeyplay]) {			/* V3.41  TLi */
	if (echecc(advers(camp))) {
      ErrorMsg(KingCapture);
	}
	else {
      alloctab(&lsgn);
      dsr_sol(camp, enonce, lsgn, OptFlag[movenbr]);
      /* V3.44  TLi */
      freetab();
	}
  }
}

void initduplex(void) {					/* V3.50  TLi */
  /*
    A hack to make the intelligent mode work with duplex.
    But anyway I have to think about the intelligent mode again
  */
  square *bnp, rsq;

#ifdef NODEF    /* V4.03  ThM, TLi */
  rsq= rb%onerow+onerow*((onerow-1)-rb/onerow);
  rb= rn%onerow+onerow*((onerow-1)-rn/onerow);
#endif /* NODEF */
  rsq= rb==initsquare ? initsquare : rb%onerow+onerow*((onerow-1)-rb/onerow);
  rb= rn==initsquare ? initsquare : rn%onerow+onerow*((onerow-1)-rn/onerow);
  rn= rsq;
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

int main(int argc, char *argv[]) {
  Token   tk= BeginProblem;
  int     i,l;					/* V3.15  ElB */
  boolean flag_starttimer;
  char    *ptr, ch= 'K';		     /* V3.62  TLi, V3.63  NG */

  i=1;
  MaxMemory= 0;
  flag_regression= false;	/* V3.74  NG */
  while (i<argc) {
	if (strcmp(argv[i], "-maxpos")==0) {
      MaxPositions= atol(argv[++i]);		/* V3.52  TLi */
      i++;
      continue;
	}
	else if (strcmp(argv[i], "-maxmem")==0) {
      MaxMemory= strtol(argv[++i], &ptr, 10);
      if (argv[i] == ptr) {
		MaxMemory= 0;
      }
      else {
		MaxMemory= MaxMemory<<10;
		if (*ptr == 'M') {
          MaxMemory= MaxMemory<<10;
          ch= 'M';
		} else if (*ptr == 'G') {
          MaxMemory= MaxMemory<<20;
          ch= 'G';
		}
      }
      i++;
      continue;
	}
	else if (strcmp(argv[i], "-regression")==0) {	/* V3.74  NG */
      flag_regression= true;
      i++;
      continue;
	}
	else {
      break;
	}
  }

  if (!MaxMemory) {
#ifdef DOS						/* V3.37  NG */
#if defined(__TURBOC__)
	/* MaxMemory= farcoreleft(); */
	MaxMemory= (unsigned long)coreleft();	   /* TLi, V3.62  NG */
#else /*! __TURBOC__*/
	/* DOS-default	256 KB */
	MaxMemory= (unsigned long)256*1024;	 /* V3.52, V3.62  NG */
#endif /*__TURBOC__*/
#else /* ! DOS */
#if defined(WIN16)		 /* TBa */
	MaxMemory= (unsigned long)1024*1024;		/* TBa */
#else /* ! WIN16 */ /* TBa begin */
#if defined(WIN32)
    /* get physical memory amount TBa 2003/01/05 */
    MEMORYSTATUS Mem;
    Mem.dwLength= sizeof(MEMORYSTATUS);
    GlobalMemoryStatus(&Mem);
    MaxMemory= Mem.dwAvailPhys;
    /* TBa end */
#ifdef WIN98
	/* WIN98 cannot handle more than 768MB */	/* V3.77  NG */
	if (MaxMemory > (unsigned long)700*1024*1024)
      MaxMemory= (unsigned long)700*1024*1024;
#endif	/* WIN98 */
#else  /* ! WIN32 */					/* TBa */
	/* UNIX-default   2 MB */
	MaxMemory= (unsigned long)2048*1024;   /* V3.52, V3.62	NG */
#endif /* ! WIN16 */		 /* TBa */
#endif /* ! WIN32 */		 /* NG */
#endif /* ! DOS */
  }

  if (i < argc) {
	OpenInput(argv[i]);
  }
  else {
	OpenInput(" ");
  }

  /* if we are running in an environment which supports
     signals, we initialize the signal handling here
  */
#if defined(SIGNALS)
  /* Set the timer (interrupt handling needs this !)	V3.60  NG */
  StartTimer();
  pyInitSignal();
#endif /*SIGNALS*/

  /* We do not issue our startup message via the language
     dependant Msg-Tables, since there the version is
     too easily changed, or not updated.
     StartUp is defined in pydata.h.
     12Apr1992 ElB.
  */
  if ((MaxMemory>>10) < 1024 || ch == 'K') {
	sprintf(MMString, " (%ld KB)\n", MaxMemory/1024);
  }
  else {
	if ((MaxMemory>>20) < 1024 || ch == 'M')
      sprintf(MMString, " (%ld MB)\n", MaxMemory>>20);
	else
      sprintf(MMString, " (%ld GB)\n", MaxMemory>>30);
  }

  if (!flag_regression) {
    pyfputs(StartUp, stdout);
    pyfputs(MMString, stdout);				/* V3.37  NG */
  }

  /* For the very first time we try to open any *.str
     When we reach the last remainig language, we force
     the existence of a *.str file. If none is found,
     InitMsgTab will exit with an appropriate message.
     Later, when the first Token is read, we reinitialize.
     See ReadBeginSpec in pyio.c
     12Apr1992 ElB.
  */
  l= 0;
  while (l<LangCount-1 && InitMsgTab(l, False) == False) {
	l++;
  }
  if (l == LangCount-1) {
	InitMsgTab(l, True);
  }

  InitCheckDir();					/* V3.40  TLi */

  do {						/* V2.4c  NG */
	boolean printa= true;				/* V3.50  TLi */
	InitBoard();					/* V3.40  TLi */
	InitCond();					/* V3.40  TLi */
	InitOpt();					/* V3.40  TLi */
	InitStip();					/* V3.40  TLi */

	/* reset MaxTime timer mechanisms */
#if defined(UNIX) && defined(SIGNALS)			/* V3.54  NG */
	alarm(0);
#endif	/* defined(UNIX) && defined(SIGNALS) */
	FlagTimeOut= false;				/* V3.54  NG */
	FlagTimerInUse= false;				/* V3.54  NG */
	FlagMaxSolsReached= false;			/* V3.60  NG */
	FlagShortSolsReached= false;			/* V3.60  NG */

	/* New problem, so reset the timer and the solutions */	/* V3.60 NG */

	flag_starttimer= true;				/* V3.60  NG */

	do {
      InitAlways();  /* V3.40  TLi */

      tk= ReadProblem(tk);

      if (tk == ZeroPosition) {			/* V3.41  TLi */
		if (!OptFlag[noboard]) {
          WritePosition();
		}
		tk= ReadProblem(tk);
		if (LaTeXout) {				/* V3.50  TLi */
          LaTeXBeginDiagram();
		}
		printa= false;				/* V3.50  TLi */
      }

      if (flag_starttimer) {			/* V3.60  NG */
		/* Set the timer for real calculation time */
		StartTimer();
		flag_starttimer= false;
      }

      /* Now set our timers for option  MaxTime
         moved to this place.    V3.60  NG
      */
      if (OptFlag[maxtime] && !FlagTimerInUse && !FlagTimeOut) {
		/* V3.54  NG */
		FlagTimerInUse= true;
#if defined(UNIX) && defined(SIGNALS)			/* V3.54  NG */
		alarm(maxsolvingtime);
#endif
#ifdef WIN32						/* V3.54  NG */
		GlobalThreadCounter++;
		_beginthread((void(*)(void*))WIN32SolvingTimeOver,
                     0, &maxsolvingtime);
#endif
#if defined(DOS)					/* V3.56  NG */
		VerifieMsg(NoMaxTime);
		FlagTimeOut= true;
#endif
      }
      maincamp= OptFlag[halfduplex] ? noir : blanc;
      /* V3.39, V3.42  NG, moved V3.62  SE */

      if (verifieposition()) {
		if (!OptFlag[noboard]) {
          WritePosition();
		}
		if (printa) {				/* V3.50  TLi */
          if (LaTeXout) {
			LaTeXBeginDiagram();
          }
          if (tk == TwinProblem) {
			StdString("a)\n\n");
          }
		}
		StorePosition();			/* V2.70  TLi */
		if (SortFlag(Proof)) {
      ProofInitialise();
      inithash();
      /* no DUPLEX for SPG's !  V3.42  NG */
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
          if (flag_atob && !FlowFlag(Exact)) {
            /* V3.76  TLi */
            StipFlags|= FlowBit(Exact);
            for (i= 1; i<enonce-1; i++)
              ProofSol(advers(maincamp), i, false);
          }
          ProofSol(advers(maincamp), enonce-1, OptFlag[movenbr]);
          SatzFlag=false;
			  }
        if (flag_atob && !FlowFlag(Exact)) {   /* V3.76  TLi */
          StipFlags|= FlowBit(Exact);
            for (i= 1; i<enonce; i++)
              ProofSol(maincamp, i, false);
        }
			  ProofSol(maincamp, enonce, OptFlag[movenbr]);
      }
      else {				/* V3.42  NG */
        if (flag_atob && !FlowFlag(Exact)) {   /* V3.76  TLi */
          StipFlags|= FlowBit(Exact);
          for (i= 1; i<enonce; i++)
            SeriesProofSol(i, false);
        }
			  SeriesProofSol(enonce, OptFlag[movenbr]);
      }
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
                  /* V3.50  TLi */
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
                     && OptFlag[intelligent])	/* V3.50  NG */
			{
              SolveSeriesProblems(blanc);
			}
			else {				/* V3.32  TLi */
              SolveSeriesProblems(maincamp);
			}
			Message(NewLine);
			if (OptFlag[duplex]) {		/* V3.60  NG */
              /* Set next side to calculate for duplex
                 "twin" */
              maincamp= advers(maincamp);
              if (   (OptFlag[maxsols]
                      && (solutions >= maxsolutions))
                     || (OptFlag[stoponshort]
                         && FlagShortSolsReached))
              {
				FlagMaxSolsReached= true;
				/* restart calculation of
				   maxsolution after "twinning"
                */
				solutions= 0;
              }
#if defined(HASHRATE)
              HashStats(1, "\n\n");
#endif	/* HASHRATE */
              if (OptFlag[intelligent]) { /* V3.50  TLi */
				initduplex();
				if (!verifieposition()) {
                  break;
				}
              }
			} /* OptFlag[duplex] */

			closehash();

          } while (OptFlag[duplex]
                   && maincamp == noir
                   && verifieposition()); /* V3.40, V3.62  TLi, SE */
		}
      } /* verifieposition */
      printa= false;				/* V3.50  TLi */
      if (   (OptFlag[maxsols]
              && (solutions >= maxsolutions))
             || (OptFlag[stoponshort] && FlagShortSolsReached))
      {						 /* V3.60  NG */
		FlagMaxSolsReached= true;
		/* restart calculation of maxsolution after "twinning"*/
		solutions= 0;
      }
	} while (tk == TwinProblem);			/* V3.40  TLi */

	if ((FlagMaxSolsReached)
        || (OptFlag[intelligent]
            && maxsol_per_matingpos)		/* V3.60  NG */
        || (FlagTimeOut))				/* V3.54  NG */
      StdString(GetMsgString(InterMessage));
	else
      StdString(GetMsgString(FinishProblem));

    StdString(" ");
    PrintTime();			/* V3.46  NG */
	StdString("\n\n\n");

	if (LaTeXout) {					/* V3.46  TLi */
      LaTeXEndDiagram();				/* V3.50  TLi */
	}

  } while (tk == NextProblem);			/* V2.4c  NG */

  CloseInput();				      /* V2.4c	StHoe */

  if (LaTeXout) {					/* V3.46  TLi */
	LaTeXClose();
  }
  exit(0);
} /*main */

#ifdef NOMEMSET
void memset(char *poi, char val, int len)
{
  while (len--)
    *poi++ = val;
}
#endif

#endif	/* ! DATABASE */
