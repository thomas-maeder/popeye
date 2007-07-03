/********************* MODIFICATIONS to py6.c **************************
**
** Date       Who  What
**
** 2001/01/14 SE   new specification : ColourChanging
**
** 2001/02/05 TLi  new piece: Friend
**
** 2001/05/08 SE   new conditions: CirceTurnCoats, CirceDoubleAgents, AMU,
**                 SentBerolina.
**
** 2001/09/29 ThM  new condition: SingleBox Type1, Type2 or Type3
**
** 2001/10/23 TLi  bug-fix: reflexmate
**
** 2001/10/26 NG   assertions only with -DASSERT due to performance penalty
**
** 2001/11/10 NG   singleboxtype? changed to singlebox type?
**		   some singlebox variable names changed ...
** 
***************************** End of List ******************************/

#ifdef macintosh        /* is always defined on macintosh's  SB */
#       define SEGM2
#       include "pymac.h"
#endif

#ifdef ASSERT
#include <assert.h> /* V3.71 TM */
#endif	/* ASSERT */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>               /* H.D. 10.02.93 prototype for exit */

/* TurboC and BorlandC   TLi */
#ifdef  __TURBOC__
#	include <ctype.h>
#	include <mem.h>
#	include <alloc.h>
#	include <conio.h>
#endif	/* __TURBOC__ */

#ifdef WIN32			/* V3.54  NG */
#include <process.h>
#endif	/* WIN32 */

#include "py.h"
#include "pyproc.h"
#include "pydata.h"
#include "pymsg.h"
#include "pyhash.h"
#include "pyint.h"
#include "DHT/dhtbcmem.h"
#include "pyproof.h"
#include "pyint.h"

boolean supergenre;       /* V3.50 SE */

boolean is_rider(piece p)                                /* TM V3.12 */
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
      case    vizridb: 			/* V3.54  NG */
      case    fersridb: 		/* V3.54  NG */
		return true;
      default:	return false;
   }
}

boolean is_leaper(piece p)               /* TM V3.12 */
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
      case    okapib:                    /* V3.32  TLi */
      case    cb: 
      case    leap15b:                        /* V3.46  NG */
      case    leap16b:                        /* V3.46  NG */
      case    leap24b:                        /* V3.46  NG */
      case    leap25b:                        /* V3.46  NG */
      case    leap35b:                        /* V3.46  NG */
      case    leap37b:                        /* V3.46  NG */
      case    leap36b:                        /* V3.64  TLi */
		return true;
      default:	return false;
   }
} /* is_leaper */

boolean is_simplehopper(piece p)                         /* TM V3.12 */
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
	case g2b:                       /* V3.44  NG */
	case g3b:                       /* V3.44  NG */
	case khb:                       /* V3.44  NG */
	case orixb:                     /* V3.44  NG */
		return true;
      default:	return false;
   }
}

boolean verifieposition(void)                     /* H.D. 10.02.93 */
{
   square          *bnp;
   piece           p;
   ply             n;
   smallint        cp, pp, tp, op;
   int             i;                       /* V3.41  TLi */
                                          
   if (CondFlag[glasgow] && CondFlag[circemalefique])    /* V3.39  TLi */
       anycirprom= True;

   /* initialize promotion squares */
   if (!CondFlag[einstein]) {              /* V3.39  TLi */
       square i;
    if (!CondFlag[whprom_sq])		/* V3.57  NG, V3.64  NG */
       for (i= 0; i < 8; i++) 
		SETFLAG(sq_spec[CondFlag[glasgow] ? haut-24-i : haut-i], WhPromSq);
    if (!CondFlag[blprom_sq])		/* V3.57  NG, V3.64  NG */
       for (i= 0; i < 8; i++) 
		SETFLAG(sq_spec[CondFlag[glasgow] ? bas+24+i : bas+i], BlPromSq);
   }

   /* aus main() verschoben  V3.03  TLi */
   im0= isquare[0];                /* V2.4d  TM */
   if (! CondFlag[imitators])      /* V3.00  NG */
      CondFlag[noiprom]= true;

   if ((droh < 0) || (enonce <= droh))     /* V2.1c  TLi */
      /* droh= enonce;                twinning bug  V3.41  TLi */
      droh= maxply;                   /* V3.41  TLi */

   zugebene= 0;
   if (FlowFlag(Alternate) && ! SortFlag(Proof)) {	/* V3.54  NG */
      if ((enonce < 2) && (maxdefen > 0) && (!SortFlag(Self))) {
         ErrorMsg(TryInLessTwo);
         maxdefen= 0;
      }
      if (OptFlag[stoponshort] && (SortFlag(Direct) || SortFlag(Self)) ) {	/* V3.60  NG */
		ErrorMsg(NoStopOnShortSolutions);
		OptFlag[stoponshort]= false;
      }
      if (enonce > (maxply-1)/2)              /* V3.13  NG */
         return VerifieMsg(BigNumMoves);
   } else
      if (enonce >= maxply-2)                 /* V3.13  NG */
         return VerifieMsg(BigNumMoves);

   totalortho= ((NonReciStipulation == stip_mate) ||               /* V3.32  TLi */
           (NonReciStipulation == stip_check) ||
           (NonReciStipulation == stip_doublemate)) &&
           (!FlowFlag(Reci) ||				/* V3.52  NG */
	   (ReciStipulation == stip_mate) ||
           (ReciStipulation == stip_check) ||
           (ReciStipulation == stip_doublemate));

   if (stipulation == stip_steingewinn &&          /* V3.03  TLi */
       CondFlag[parrain])
      return VerifieMsg(PercentAndParrain);

	flagdiastip=                          /* V3.50 SE */
		NonReciStipulation == stip_circuit ||
		NonReciStipulation == stip_exchange ||
		NonReciStipulation == stip_circuitB ||
		NonReciStipulation == stip_exchangeB ||
		ReciStipulation == stip_circuit ||
		ReciStipulation == stip_exchange ||
		ReciStipulation == stip_circuitB ||
		ReciStipulation == stip_exchangeB;

	for (p= roib; p <= derbla; p++) {
      nbpiece[p]= 0;
      nbpiece[-p]= 0;
   }
   if (TSTFLAG(PieSpExFlags, HalfNeutral))      /* V3.57 SE */
   	SETFLAG(PieSpExFlags, Neutral);

   /* moved from InitBoard() (py1.c)  V3.41  TLi */
   for (i= fb + 1; i <= derbla; i++)
      exist[i]= false;
   for (i= roib; i <= fb; i++)
      exist[i]= true;
   if (CondFlag[sentinelles])
   	exist[sentinelb]= exist[sentineln]= true;

   for (bnp= boardnum; *bnp; bnp++) {
      p= e[*bnp];
      if (p != vide) {        /* V3.22  TLi */
        if (CondFlag[volage] && rb != *bnp && rn != *bnp)
            SETFLAG(spec[*bnp], Volage);
	if ((PieSpExFlags >> DiaCirce) || flagdiastip) /* V3.22  TLi, V3.50 SE */
		SetDiaRen(spec[*bnp], *bnp);
	if (TSTFLAG(spec[*bnp], ColourChange))         /* V3.64 SE */
		if (!is_simplehopper(abs(e[*bnp]))) {
		/* relies on imitators already having been implemented */
			CLRFLAG(spec[*bnp], ColourChange);
			ErrorMsg(ColourChangeRestricted); 
		}
	}
	if (p < fn)
		exist[-p]= true;
	else if (p > fb)
		exist[p]= true;

#ifdef NODEF	/* We allow pawns on first or eight rank ... */	/* V3.63  NG */
      if (   p == pb                  /* V1.6c  NG */
          || p == pbb
         )
         if (!CondFlag[einstein])        /* V3.1  TLi */
         if (((*bnp < bas + 24) && ! anycirprom) ||      /* V3.1  TLi */
             ((*bnp > haut - 24) && ! (anycirprom && TSTFLAG(spec[*bnp], Neutral))))     /* V3.21  NG */
            return VerifieMsg(PawnFirstRank);
      if (   p == pn                  /* V1.6c  NG */
          || p == pbn
         )
         if (!CondFlag[einstein])        /* V3.1  TLi */
         if (((*bnp > haut - 24) && ! anycirprom) ||     /* V3.1  TLi */
             ((*bnp < bas + 24) && ! (anycirprom && TSTFLAG(spec[*bnp], Neutral))))
            return VerifieMsg(PawnFirstRank);                      /* V3.21  NG */
#endif	/* NODEF */

      nbpiece[p]++;
   }
   for (p= fb + 1; p <= derbla; p++)
      if (exist[p] || promonly[p]) {                  /* V3.43  NG */
         flagfee= true;
         if (is_rider(p))                        /* V3.12 TM */
            flagriders= true;
         else if (is_leaper(p))
            flagleapers= true;
         else if (is_simplehopper(p))
            flagsimplehoppers= true;
         else
            flagveryfairy= true;
         if (CondFlag[einstein])         /* V3.1  TLi */
            return  VerifieMsg(EinsteinAndFairyPieces);
      }

   if (CondFlag[sting]) {                /* V3.63  NG */
	totalortho= false;
	flagfee= true;
	flagsimplehoppers= true;
   }

   if (!CondFlag[noiprom])                                    /* V2.4d  TM */
      for (n= 0; n <= maxply; Iprom[n++]= false);

   flaglegalsquare= TSTFLAG(PieSpExFlags, Jigger) ||
          CondFlag[newkoeko] ||                  /* V3.1  TLi */
          CondFlag[gridchess] || CondFlag[koeko] ||
          CondFlag[blackedge] || CondFlag[whiteedge];      /* V2.70  TLi, V2.90  NG */
   if (CondFlag[imitators]) {                              /* V2.4d  TM */
      if (    flagveryfairy                   /* V3.12  TM */
           || flaglegalsquare
           || CondFlag[chinoises]
           || anyimmun                        /* V3.1  TLi, aber warum eigentlich nicht */
           || CondFlag[haanerchess]
           || CondFlag[parrain])              /* V3.02  TLi verkraftet nicht 2 IUW in einem Zug !!! */
          return VerifieMsg(ImitWFairy);
      totalortho= false;                      /* V3.0  TLi */
   }

   if (CondFlag[leofamily])
      for (p= db; p <= fb; p++) {
         if (nbpiece[p] + nbpiece[-p] != 0)        /* V1.4c  NG */
            return VerifieMsg(LeoFamAndOrtho);
         exist[p]= false;
      }
   if (CondFlag[chinoises]) {
      for (p= leob; p <= vaob; p++)
         exist[p]= true;
      flagfee= true;                          /* V1.5c  NG */
   }
   if (anycirce) {                          /* V2.1c  NG , V3.1  TLi */
	 if (exist[dummyb])
	    return VerifieMsg(CirceAndDummy);
	 if (TSTFLAG(PieSpExFlags, Neutral))     /* V3.0  TLi */
	    totalortho= false;
   }
   if (flagmaxi)                            /* V3.62 SE */
   {
   	CondFlag[blmax]= (maincamp==blanc);
      CondFlag[whmax] = (maincamp==noir);
      if (maincamp==blanc)
      {
      	bl_ultra= CondFlag[ultra];
         bl_exact= CondFlag[exact];
         black_length= len_max;
      /*   white_length= len_normal;	*/	/* V3.62  SE */
         flagblackmummer= true;
         flagwhitemummer= false;
      }
      else
      {
      	wh_ultra= CondFlag[ultra];
         wh_exact= CondFlag[exact];
         white_length= len_max;
      /*   black_length= len_normal;	*/	/* V3.62  SE */
         flagwhitemummer= true;
         flagblackmummer= false;
      }
   }
   if (flagultraschachzwang)       /* V3.62 SE */
   {
   	CondFlag[blackultraschachzwang]= (maincamp==blanc);
      CondFlag[whiteultraschachzwang]= (maincamp==noir);
   }
   if (CondFlag[blackultraschachzwang] || CondFlag[whiteultraschachzwang])
   	totalortho= false;

   if (CondFlag[cavaliermajeur]) {
	 if (nbpiece[cb] + nbpiece[cn] > 0)
	    return VerifieMsg(CavMajAndKnight);
	 exist[cb]= false;
	 exist[nb]= true;                        /* V2.60  NG */
      flagfee= true;                          /* V2.60  NG */
	}

	if (CondFlag[republican]) {               /* V3.50 SE */

#ifdef NODEF				/* v3.53 SE - removed */
 		if (stipulation != stip_mate ||
			(FlowFlag(Reci) && ReciStipulation != stip_mate))
			return VerifieMsg(RepublicanandnotMate); 
		if (rb || rn)
			return false;
#endif	/* NODEF */

		OptFlag[sansrb]= OptFlag[sansrn]= True;
		totalortho= False;
		flag_dontaddk=false;
	}

	if (OptFlag[sansrb] && rb)                      /* V2.60  NG */
         OptFlag[sansrb]= False;
   if (OptFlag[sansrn] && rn)      /* V2.1c  NG,  V2.60  NG */
         OptFlag[sansrn]= False;
   if ((rb == initsquare) && ! OptFlag[sansrb])    /* V2.60  NG */
      ErrorMsg(MissingKing);
   if ((rn == initsquare) && ! OptFlag[sansrn])    /* V2.1c  NG, V2.60  NG */
      ErrorMsg(MissingKing);
   if (rex_circe) {               /* V3.37  TLi */
	 /* why not royal pieces in PWC ??? TLi */
      /* Because we loose track of the royal attribute somewhere and I didn't find where ... NG */
	if (CondFlag[circeequipollents] || CondFlag[circeclone] || CondFlag[couscous] ||
		CondFlag[circeclonemalefique])
	/* V3.55 SE disallowed because of the call to (*circerenai) in echecc */
	/*          would require knowledge of id. Other forms now allowed    */
	 if (((! OptFlag[sansrb]) && (e[rb] != roib)) ||
          ((! OptFlag[sansrn]) && (e[rn] != roin)))   /* V2.1c  NG */
         return VerifieMsg(RoyalPWCRexCirce);
   }
   if (TSTFLAG(PieSpExFlags, Neutral)) {
		initneutre(blanc);
      flag_nk= rb!=initsquare && TSTFLAG(spec[rb], Neutral);  /* V3.50 SE */
   }
   /*****  V3.1  TLi  begin  *****/
   if (TSTFLAG(PieSpExFlags, Neutral) && CondFlag[bicolores])
      return VerifieMsg(NeutralAndBicolor);
   /*****  V3.1  TLi  end  *****/

   eval_2= eval_white= eval_ortho;                         /* V2.80  TLi */
	if (flaglegalsquare= (flaglegalsquare ||                /* V2.70  TLi */
               CondFlag[bichro] || CondFlag[monochro])) {
      eval_white= legalsquare;                        /* V2.80  TLi */
      eval_2= eval_ortho;
      if (CondFlag[monochro] && CondFlag[bichro])
         return VerifieMsg(MonoAndBiChrom);
      if ((CondFlag[koeko] || CondFlag[newkoeko] ||
          TSTFLAG(PieSpExFlags, Jigger))      /* V3.1  TLi */
          && anycirce                         /* V3.1  TLi */
          && TSTFLAG(PieSpExFlags, Neutral))
         return VerifieMsg(KoeKoCirceNeutral);
   }
   if (flaglegalsquare || TSTFLAG(PieSpExFlags,Neutral))   /* V2.70  TLi */
      if (CondFlag[volage])
         return VerifieMsg(SomeCondAndVolage);
   if (TSTFLAG(PieSpExFlags,Paralyse) &&                 /* V3.32  TLi */
       !(CondFlag[patrouille]
            || CondFlag[beamten]
            || TSTFLAG(PieSpExFlags, Beamtet))) {	/* V3.53  TLi */
      eval_2= eval_white;
      eval_white= paraechecc;
   }
   if (TSTFLAG(PieSpExFlags, Kamikaze)) {
      totalortho= false;                              /* V3.0  TLi */
      if (CondFlag[haanerchess])
         return VerifieMsg(KamikazeAndHaaner);
      if (anycirce /* CondFlag[circe] */ )            /* V3.1  TLi */
         /* No Kamikaze and Circe with fairy pieces; taking and
            taken piece could be reborn on the same square! */
         if (flagfee || CondFlag[volage])
            return VerifieMsg(KamikazeAndSomeCond);                /* V2.90  NG */
   }
   if ((CondFlag[supercirce] || CondFlag[april]) &&
       (CondFlag[koeko] || CondFlag[newkoeko]))
      return  VerifieMsg(SuperCirceAndOthers);

   if (CondFlag[supercirce] && CondFlag[april])
      return  VerifieMsg(SuperCirceAndOthers);

   if (CondFlag[patrouille] ||                        /* V2.80  TLi */
		 CondFlag[beamten] ||                           /* V3.32  TLi */
		 CondFlag[central] ||                           /* V3.50 SE */
                 TSTFLAG(PieSpExFlags, Beamtet)) {		/* V3.53  TLi */
		eval_2= eval_white;
      eval_white= soutenu;
   }

	if (CondFlag[nowhcapture] && CondFlag[noblcapture])	/* V3.64  NG */
		CondFlag[nocapture]= true;
	if (CondFlag[nocapture]) {				/* V3.64  NG */
		CondFlag[nowhcapture]= false;
		CondFlag[noblcapture]= false;
	}

   /* if (CondFlag[isardam] && CondFlag[madras])         V3.44  SE/TLi */
   if (CondFlag[isardam] && flag_madrasi)         /* V3.60  TLi */
      return VerifieMsg(IsardamAndMadrasi);

   if (anymars) {                  /* V3.46  SE/TLi */
	totalortho= false;
/*        empilegenre= true;	*/	/* set also below ...  V3.64  NG */
	if (CondFlag[whtrans_king] || CondFlag[whrefl_king] ||
            CondFlag[bltrans_king] || CondFlag[blrefl_king] ||   
            CondFlag[bicolores] || CondFlag[sting] ||
          flagsimplehoppers || flagveryfairy )
         return VerifieMsg(MarsCirceAndOthers);
   }

   /* if (CondFlag[madras] || CondFlag[isardam]) {       V3.44  SE/TLi */
   if (flag_madrasi || CondFlag[isardam]) {       /* V3.60  TLi */
      if (CondFlag[imitators] ||
          TSTFLAG(PieSpExFlags,Paralyse))
         return VerifieMsg(MadrasiParaAndOthers);
      if (!(CondFlag[patrouille]
             || CondFlag[beamten]                     /* V3.32  TLi */
             || TSTFLAG(PieSpExFlags, Beamtet))) {    /* V3.53  TLi */
         eval_2= eval_white;
         /* eval_white= CondFlag[madras] ? eval_madrasi : eval_isardam;
                            V3.44  SE/TLi */
         eval_white= CondFlag[isardam] ?
                           eval_isardam : eval_madrasi; /* V3.60  TLi */
      }
   }
   if (CondFlag[woozles]) {          /* V3.55  TLi */
       /* if (CondFlag[madras] || CondFlag[isardam] || */
       if (flag_madrasi || CondFlag[isardam] ||    /* V3.60  TLi */
           CondFlag[imitators] || TSTFLAG(PieSpExFlags,Paralyse))
           return VerifieMsg(MadrasiParaAndOthers);
       totalortho= false;
       eval_2= eval_white;
       eval_white= eval_wooheff;
   }
   if (CondFlag[brunner])             /* V3.50 SE */
      eval_white=eval_isardam;
   if (CondFlag[isardam] && IsardamB) /* V3.50 SE */
      eval_white=eval_ortho;
   if (CondFlag[shieldedkings])       /* V3.62 SE */
   	eval_white=eval_shielded;

	if (flagAssassin)                  /* V3.50 SE */
      if (TSTFLAG(PieSpExFlags,Neutral) ||      /* Neutrals not implemented */
         CondFlag[bicolores])               /* others? */
         return VerifieMsg(AssassinandOthers);

       eval_black= eval_white;                                 /* V2.80  TLi */
   /* nachfolgend ueberarbeitet da neu rex_mad/cir/imm   V2.70  TLi */
   if (rex_circe || rex_immun) {
      if (rex_circe && rex_immun)
         return VerifieMsg(RexCirceImmun);
      if (anyanticirce)                       /* V3.1  TLi */
			/* an additional pointer to evaluate-functions is
            required  TLi */
         return VerifieMsg(SomeCondAndAntiCirce);
      eval_2= eval_white;
      if (rex_circe) {                  
	    eval_white= rbcircech;
	    eval_black= rncircech;
	    cirrenroib= (*circerenai)(roib, spec[rb], initsquare, initsquare, noir);        /* V3.1  TLi */
	    cirrenroin= (*circerenai)(roin, spec[rn], initsquare, initsquare, blanc);       /* V3.1  TLi */
	 } else {
	    eval_white= rbimmunech;
	    eval_black= rnimmunech;
	    immrenroib= (*immunrenai)(roib, spec[rb], initsquare, initsquare, noir);        /* V3.1  TLi */
	    immrenroin= (*immunrenai)(roin, spec[rn], initsquare, initsquare, blanc);       /* V3.1  TLi */
	 }
   }

   if (anyanticirce) {     /* V3.1  TLi */
      if (CondFlag[couscous] ||
          CondFlag[koeko] ||
          CondFlag[newkoeko] ||
          sbtype1 ||  /* V3.71 TM */
          TSTFLAG(PieSpExFlags, Kamikaze))
         return VerifieMsg(SomeCondAndAntiCirce);
      totalortho= false;
      eval_2= eval_white;
      eval_white= rbanticircech;
      eval_black= rnanticircech;
   }

   if (sbtype1) {     /* V3.71 TM */
     if (flagfee)
       return VerifieMsg(SingleBoxAndFairyPieces);
      totalortho= false;
      eval_2= eval_white;
      eval_white= rbsingleboxtype1ech;
      eval_black= rnsingleboxtype1ech;
   }

   if (sbtype3) {     /* V3.71 TM */
     if (flagfee || sbtype1)
       return VerifieMsg(SingleBoxAndFairyPieces);
      totalortho= false;
      rnechec = &singleboxtype3_rnechec;
      rbechec = &singleboxtype3_rbechec;
      gen_wh_piece = &singleboxtype3_gen_wh_piece;
      gen_bl_piece = &singleboxtype3_gen_bl_piece;
   }

   if (sbtype2)      /* V3.71 TM */
     if (sbtype3 || sbtype1)
       return VerifieMsg(SingleBoxAndFairyPieces);
     
   if (wh_ultra && !CondFlag[whcapt]) {            /* V3.1  TLi */
      eval_2= eval_white;
      eval_black= rnultraech;
      if (TSTFLAG(PieSpExFlags, Neutral))
         return VerifieMsg(OthersNeutral);
   }
   if (bl_ultra && !CondFlag[blcapt]) {            /* V3.1  TLi */
      eval_2= eval_white;
      eval_white= rbultraech;
      if (TSTFLAG(PieSpExFlags, Neutral))
         return VerifieMsg(OthersNeutral);
   }

   if (CondFlag[whmin] + CondFlag[whmax] + CondFlag[whcapt] +
       (CondFlag[whforsqu] || CondFlag[whconforsqu]) +     /* V3.20  TLi */
       CondFlag[whfollow] + CondFlag[duellist] > 1 ||      /* V2.90c  TLi */
       CondFlag[blmin] + CondFlag[blmax] + CondFlag[blcapt] +
       (CondFlag[blforsqu] || CondFlag[blconforsqu]) +     /* V3.20  TLi */
       CondFlag[blfollow] + CondFlag[duellist] > 1)        /* V3.0  TLi */
      return VerifieMsg(TwoMummerCond);
#ifndef DATABASE  /* TLi */
   if (    (CondFlag[whmin] || CondFlag[blmin] ||
       CondFlag[whmax] || CondFlag[blmax] || CondFlag[heffalumps]  /* V3.55  TLi */
      )
       &&
      (exist[roseb] || exist[csb] || exist[ubib] ||
       exist[hamstb] || exist[mooseb] || exist[eagleb] || exist[sparrb] ||
       exist[archb] || exist[reffoub] || exist[cardb] ||
       exist[bscoutb] || exist[gscoutb] ||
       exist[dcsb] || exist[refcb] || exist[catb] ||
       exist[rosehopperb] || exist[roselionb] || exist[raob] ||		/* V3.63  NG */
       exist[rookmooseb] || exist[rookeagleb] || exist[rooksparrb] ||
       exist[bishopmooseb] || exist[bishopeagleb] || exist[bishopsparrb] ||
       exist[doublegb]
      )
      )
		return VerifieMsg(SomePiecesAndMaxiHeffa);
#endif

	if (flagdiastip && (CondFlag[frischauf] ||    /* V3.50 SE */
			CondFlag[sentinelles] || CondFlag[imitators]))
		return VerifieMsg(DiaStipandsomeCond);

	jouegenre =                     /* V3.1  TLi */
			CondFlag[republican] ||       /* V3.50 SE */
			anycirce ||
         CondFlag[sentinelles] ||
         CondFlag[duellist] ||
         anyanticirce ||
         sbtype1 || /* V3.71  TM */
         CondFlag[blroyalsq] ||
         CondFlag[whroyalsq]; /* V3.50  TLi */


   change_moving_piece=            /* V3.1  TLi */
         TSTFLAG(PieSpExFlags, Kamikaze) ||
         CondFlag[tibet] ||
         CondFlag[andernach] ||
         CondFlag[antiandernach] ||
         CondFlag[magic] ||
         TSTFLAG(PieSpExFlags, Chameleon) ||
         CondFlag[einstein] ||
         CondFlag[volage] ||
         TSTFLAG(PieSpExFlags, Volage) ||
         CondFlag[degradierung] ||
         CondFlag[norsk] ||
         CondFlag[traitor] ||
         CondFlag[linechamchess] ||        /* V3.64  TLi */
         CondFlag[chamchess];        /* V3.32  TLi */

   repgenre =      /* CondFlag[tibet] ||  V3.1  TLi */
         CondFlag[sentinelles] ||
	    CondFlag[imitators] || anycirce ||      /* V3.1  TLi */
         TSTFLAG(PieSpExFlags, Neutral) ||
         sbtype1 || /* V3.71  TM */
         anyanticirce;                           /* V3.1  TLi */


   empilegenre=    flaglegalsquare ||              /* V2.60  NG */
         CondFlag[patrouille] || CondFlag[imitators] ||
         CondFlag[beamten] ||               /* V3.32  TLi */
         TSTFLAG(PieSpExFlags, Beamtet) ||  /* V3.53  TLi */
	 CondFlag[central] ||               /* V3.50 SE */
	 anyimmun || CondFlag[nocapture] ||     /* V3.1  TLi */
	 CondFlag[nowhcapture] ||		/* V3.64  NG */
	 CondFlag[noblcapture] ||		/* V3.64  NG */
         /* TSTFLAG(PieSpExFlags, Kamikaze) && CondFlag[circe] || */
         TSTFLAG(spec[rb], Kamikaze) || TSTFLAG(spec[rn], Kamikaze) ||   /* V3.1  TLi */
         flagwhitemummer || flagblackmummer ||
         TSTFLAG(PieSpExFlags, Paralyse) || CondFlag[vogt] ||
         anyanticirce || anymars ||      /* V3.46  SE/TLi */
         sbtype1 || /* V3.71  TM */
         CondFlag[messigny] ||           /* V3.55  TLi */
         CondFlag[woozles] ||           /* V3.55  TLi */
         CondFlag[nowhiteprom] ||	/* V3.64 NG */
         CondFlag[noblackprom] ||	/* V3.64 NG */
         CondFlag[norsk];                /* V3.1  TLi */
                     /* V2.90, 3.03  TLi */

	if (TSTFLAG(PieSpExFlags, Jigger) ||            /* V3.1  TLi */
       CondFlag[newkoeko] ||                       /* V3.1  TLi */
       CondFlag[koeko] ||                          /* V3.0  TLi */
       CondFlag[parrain] ||                        /* V3.02  TLi */
       flagwhitemummer || flagblackmummer ||
       CondFlag[vogt] ||                           /* V3.03  TLi */
       (eval_white != eval_ortho &&
        eval_white != legalsquare) ||
       (rb != initsquare && abs(e[rb]) != King) || /* V3.1  TLi */
       (rn != initsquare && abs(e[rn]) != King) ||
       TSTFLAG(PieSpExFlags, Chameleon) ||         /* V3.1  TLi */
       CondFlag[einstein] ||                       /* V3.1  TLi */
       CondFlag[degradierung] ||                   /* V3.1  TLi */
       CondFlag[norsk] ||                          /* V3.1  TLi */
       CondFlag[messigny] ||                       /* V3.55 TLi */
       CondFlag[linechamchess] ||                  /* V3.64 TLi */
       CondFlag[chamchess] ||                      /* V3.32 TLi */
       TSTFLAG(PieSpExFlags, HalfNeutral))         /* V3.57 SE */
      totalortho= false;

   pp= cp= 0;      /* init promotioncounter and checkcounter  V2.60  NG */
   for (p= db; p <= derbla; p++) {                         /* V2.60  NG */
      getprompiece[p]= vide;
      if (promonly[p])                                /* V3.42  NG */
         exist[p]= True;
      if (exist[p]) {
         if (p != dummyb && p != pbb &&
             p != bspawnb && p != spawnb &&
             (!CondFlag[promotiononly] || promonly[p])) {        /* V3.42, V3.44  NG */
            getprompiece[pp]= p;
            pp= p;
         }
         if (p > fb && p != dummyb) {    /* only fairy pieces until now ! V2.60  NG */
            totalortho= false;      /* V3.0  TLi, NG */
            if (p != hamstb)
               checkpieces[cp++]= p;
         }
      }
   }
   checkpieces[cp]= vide;

   tp= op= 0;
   for (p= roib; p <= derbla; p++)
      if (exist[p] && p != dummyb && p != hamstb) {
         transmpieces[tp++]= p;
         if (p != Orphan && p != Friend
             && (exist[Orphan] || exist[Friend]))          /* V3.65  TLi */
         orphanpieces[op++]= p;
      }
   transmpieces[tp]= vide;
   if (CondFlag[whrefl_king] || CondFlag[blrefl_king])     /* V3.47  NG */
      totalortho= false;
   orphanpieces[op]= vide;

   if ((CondFlag[whrefl_king] && 
	(rb != initsquare && (e[rb] != roib || CondFlag[sting]))) || /* V3.39  TLi, V3.47  NG, V3.63  NG */
       (CondFlag[blrefl_king] && 
	(rn != initsquare && (e[rn] != roin || CondFlag[sting]))))   /* V3.39  TLi, V3.47  NG, V3.63  NG */
      return VerifieMsg(TransmRoyalPieces);

   if ((orph_refking= exist[Orphan] || exist[Friend] ||    /* V3.65  TLi */
       CondFlag[whrefl_king] || CondFlag[blrefl_king]) &&  /* V3.47  NG */
       TSTFLAG(PieSpExFlags, Neutral))
      return VerifieMsg(NeutralAndOrphanReflKing);

   if ((eval_white==eval_isardam) && CondFlag[vogt])     /* v3.50 SE */
      return VerifieMsg(VogtlanderandIsardam);

   for (n= 2; n <= maxply; inum[n++]= inum[1]);           /* V2.4d  TM */

   if ((CondFlag[chamchess] || CondFlag[linechamchess])   /* V3.64  TLi */
                    && TSTFLAG(PieSpExFlags, Chameleon))  /* V3.32,V3.64  TLi */
      return VerifieMsg(ChameleonPiecesAndChess);

#ifdef NODEF	/* V3.62  TLi */
   /* option nontrivial */
   NonTrivialNumber += enonce + 2 - NonTrivialLength;    /* V3.32  TLi */
#endif

   if (TSTFLAG(PieSpExFlags, ColourChange)) {           /* V3.64 SE */
        checkhopim = true;
        totalortho = false;
        jouegenre = true;
   }
   checkhopim |= CondFlag[imitators];

   totalortho &= !CondFlag[sentinelles];
                 /* a nasty drawback */

   FlagMoveOrientatedStip = ReciStipulation == stip_target ||
			ReciStipulation == stip_ep ||
			ReciStipulation == stip_capture ||
			ReciStipulation == stip_steingewinn ||
			ReciStipulation == stip_castling ||   /* V3.35  TLi */
			NonReciStipulation == stip_target ||
			NonReciStipulation == stip_ep ||
			NonReciStipulation == stip_capture ||
			NonReciStipulation == stip_steingewinn ||
			NonReciStipulation == stip_castling;   /* V3.35  TLi */

   if (stipulation == stip_doublemate &&              /* V3.33  TLi */
       (SortFlag(Self) || SortFlag(Direct) ||
        (FlowFlag(Reci) && CounterMate && ReciStipulation == stip_doublemate)))
      return VerifieMsg(StipNotSupported);

   /* check castling possibilities */              /* V3.35  NG */
   castling_flag[0]= 0x00;     /* TLi */
   /* castling_supported has to be adjusted if there are any problems */
   /* with castling and fairy conditions/pieces */
   if (castling_supported= !(
/* Let's see if transmuting kings can castle without problems ... */ /* V3.51  NG */
/* Unfortunately they can't ! So I had to exclude them again ...  */ /* V3.55  NG */
/* A wK moving from anywhere to e1 and then like a queen from e1 to g1 */
/* would get the castling right when this last move is retracted  (:-( */
/* transmuting kings and castling enabled again                   */ /* V3.55  TLi */
#ifdef NODEF
		CondFlag[whrefl_king] || 
		CondFlag[blrefl_king] || 
#endif
		CondFlag[patience] || 			/* V3.53  NG */
		CondFlag[parrain] || 
		CondFlag[haanerchess] || 
		CondFlag[imitators]
      )) {       /* V3.35, V3.51  NG */
   if (e[204]== roib)
      castling_flag[0]|= 0x40;
   if (e[207]== tb)
      castling_flag[0]|= 0x10;
   if (e[200]== tb)
      castling_flag[0]|= 0x20;
   if (e[372]== roin)
      castling_flag[0]|= 0x04;
   if (e[375]== tn)
      castling_flag[0]|= 0x01;
   if (e[368]== tn)
      castling_flag[0]|= 0x02;
   } /* castling_supported */
   if (stipulation == stip_castling && !castling_supported)	/* V3.53  NG */
	return VerifieMsg(StipNotSupported);
   castling_flag[0]&= no_castling;				/* V3.55  NG */
   castling_flag[2]= castling_flag[1]= castling_flag[0];   /* At which ply do we begin ??  NG */
		/* no_castling added  V3.55  NG */
   testcastling= (castling_flag[0]&0x60&no_castling) == 0x60 ||
                 (castling_flag[0]&0x50&no_castling) == 0x50 ||
                 (castling_flag[0]&0x06&no_castling) == 0x06 ||
                 (castling_flag[0]&0x05&no_castling) == 0x05;

#ifndef DATABASE  /* V3.39  TLi */
   if (SortFlag(Proof))                     /* V3.36  TLi */
      return ProofVerifie();
#endif
   /* a small hack to enable ep keys     V3.37  TLi */
   trait[1]= 2;

   if (CondFlag[isardam] || CondFlag[ohneschach]   /* V3.44  SE/TLi */
       || CondFlag[exclusive]) {                    /* V3.45  TLi */
       flag_testlegality= true;
       totalortho= false;
	}

	if (!CondFlag[patience])      /* needed because of twinning */
		PatienceB= false;

	jouetestgenre= flag_testlegality || flagAssassin ||   /* V3.50 SE */
			(stipulation==stip_doublemate || ReciStipulation==stip_doublemate) ||
			CondFlag[patience] || CondFlag[republican]
         || CondFlag[blackultraschachzwang] || CondFlag[whiteultraschachzwang] ;    /* V3.62 SE */

	supergenre=    CondFlag[supercirce]
                || CondFlag[april]
                || CondFlag[republican];  /* V3.50 */

	if (CondFlag[extinction] || flagAssassin)	/* V3.52  NG */
		totalortho= false;

	if (CondFlag[actrevolving] || CondFlag[arc]) {		/* V3.53, 3.62  TLi, SE */
		jouegenre= true;
		totalortho= false;
	}
   if (anytraitor) totalortho= false;
#ifdef DEBUG	/* V3.52  NG */
printf("int: %s, mate: %s, stalemate: %s, castling: %s, fee: %s, orth: %s, help: %s, direct: %s, series: %s\n",
OptFlag[intelligent]?"true":"false",
stipulation == stip_mate?"true":"false",
stipulation == stip_stale?"true":"false",
testcastling?"true":"false",
flagfee?"true":"false",
totalortho?"true":"false",
SortFlag(Help)?"true":"false",
SortFlag(Direct)?"true":"false",
FlowFlag(Series)?"true":"false");
#endif	/* DEBUG */

    if (OptFlag[intelligent] &&              /* V3.52  NG, V3.62  TLi, NG */
        ((stipulation != stip_mate && (stipulation != stip_stale /* || testcastling V3.62  TLi */))
	|| flagfee 
/*	|| !totalortho */
	|| !(SortFlag(Help) || (SortFlag(Direct) && FlowFlag(Series)))))
		return VerifieMsg(IntelligentRestricted);

    if (InitChamCirce) {                 /* V3.45  TLi */
       if (CondFlag[leofamily]) {
      NextChamCircePiece[Leo]= Mao;
      NextChamCircePiece[Pao]= Leo;
      NextChamCircePiece[Vao]= Pao;
      NextChamCircePiece[Mao]= Vao;
       } else {
      piece actknight= CondFlag[cavaliermajeur] ? NightRider : Knight;
      NextChamCircePiece[actknight]= Bishop;
      NextChamCircePiece[Bishop]= Rook;
      NextChamCircePiece[Rook]= Queen;
      NextChamCircePiece[Queen]= actknight;
       }
   }

   RB_[1]= rb;
   RN_[1]= rn;

   return true;
} /* verifieposition */

numecoup sic_coup;
ply sic_ply;    /* V3.51 SE */

void current(coup *mov)
{
   square sq = ca[nbcou];

   /*------------------------------------- modified. V1.2c  NG */
   mov->tr=                trait[nbply];                   /* V3.1  TLi */
   mov->cdzz =             cd[nbcou];
   mov->cazz=              sq;
   mov->cpzz=              cp[nbcou];
   mov->pjzz=              pjoue[nbply];
   mov->norm_prom=         norm_prom[nbply];               /* V3.02  TLi */
   mov->ppri=              pprise[nbply];
   mov->sqren=             sqrenais[nbply];
   mov->cir_prom=          cir_prom[nbply];                /* V3.02  TLi */

	if ((bl_exact && mov->tr == noir) ||
		 (wh_exact && mov->tr == blanc))
		mov->echec=     false;          /* A quick and dirty hack. But echecc */
						/* destroys the 'current()' entry     */ /* V3.20  NG */
	else if (CondFlag[isardam] || CondFlag[brunner] )     /* V 3.51 SE */
		if (flag_writinglinesolution) {
			numecoup tempcoup=nbcou;
			ply tempply=nbply;
			nbcou=sic_coup;
			nbply=sic_ply;
			mov->echec=		echecc(advers(mov->tr));
			nbcou=tempcoup;
			nbply=tempply;		/* Not such a quick hack, but probably dirtier! */
						/* May work for the above exact conditions too  */
		} else
			mov->echec=  echecc(advers(mov->tr));
	else
		mov->echec=     echecc(advers(mov->tr));
   mov->renkam=            crenkam[nbply];
   mov->promi=             Iprom[nbply];                   /* V2.4d  TM */
   mov->numi=              inum[nbply] - (mov->promi ? 1 : 0);
   /* Promoted imitator will be output 'normally' from the next move on. V2.4d  TM */
   mov->sum=               isquare[0] - im0;               /* V2.4d  TM */
   mov->speci=             jouespec[nbply];
   /* hope the following works with parrain too   TLi */
   mov->ren_spec=          spec[sqrenais[nbply]];          /* V3.50  TLi */
   mov->bool_senti=        senti[nbply];                   /* V2.90  TLi */
   mov->ren_parrain=       ren_parrain[nbply];             /* V3.02  TLi */
   mov->bool_norm_cham_prom=                               /* V3.1  TLi */
            norm_cham_prom[nbply];
   mov->bool_cir_cham_prom=                                /* V3.1  TLi */
            cir_cham_prom[nbply];
   mov->pjazz=             jouearr[nbply];                 /* V3.1  TLi */
   mov->repub_k=				super[nbply]; /* V3.50 SE */
   mov->new_spec=				spec[sq];     /* V3.57 SE */
   mov->hurdle=                                 chop[nbcou];  /* V3.64 SE */
   mov->sb3where = sb3[nbcou].where; /* V3.71 TM */
   mov->sb3what = sb3[nbcou].what; /* V3.71 TM */
   if (mov->sb3what!=vide && mov->sb3where==mov->cdzz)
     mov->pjzz = mov->pjazz = mov->sb3what;
   mov->sb2where = sb2[nbply].where;
   mov->sb2what = sb2[nbply].what;
   /*------------------------------------- modified. V1.2c  NG */
}

void alloctab(smallint *n)
{
   *n= ++(tabsol.nbr);
   tabsol.cp[*n]= tabsol.cp[*n-1];
}

void freetab(void)                                      /* H.D. 10.02.93 */
{
   --(tabsol.nbr);
}

void pushtabsol(smallint n)
{
   if (++(tabsol.cp[n]) > toppile)
      ErrorMsg(TooManySol);
   else
      current(&(tabsol.liste[tabsol.cp[n]]));
   coupfort();
}

smallint tablen(smallint t)
{
   return tabsol.cp[t]-tabsol.cp[t-1];
}

boolean WriteSpec(Flags sp, boolean printcolours)            /* V3.62  TLi */
{
   boolean ret= False;
   PieSpec spname= Neutral;                /* V3.50  TLi */

   if (printcolours /* p != vide */ && !TSTFLAG(sp, Neutral))	/* V3.62  TLi */
      spname= White;                  /* V3.50  TLi */
   for (; spname < PieSpCount; spname++)   /* V3.50  TLi */
      if ((spname != Volage || !CondFlag[volage]) &&
          TSTFLAG(sp, spname)) {
      StdChar(tolower(*PieSpString[ActLang][spname]));
      ret= True;
   }
   return ret;
} /* WriteSpec */

#ifdef DATABASE
extern boolean two_same_pieces;
#endif

void editcoup(coup *mov) {
   char    BlackChar= *GetMsgString(BlackColor);
   char    WhiteChar= *GetMsgString(WhiteColor);
   short   icount, diff;                                   /* V2.4d  TM */
   square sq;

   /* Did we castle ?? */          /* V3.35  NG */
      if (mov->cpzz == maxsquare+2 ||
          mov->cpzz == maxsquare+3) {    /* V3.55  TLi */
          /* castling */
          StdString("0-0");
          if (mov->cpzz == maxsquare+3)
              StdString("-0");
          if (CondFlag[einstein]) {
                StdChar('=');
             if (CondFlag[reveinstein])
                WritePiece(db);
             else
                WritePiece(fb);
          }
      } else {

      if (mov->cpzz == maxsquare+1) {             /* V3.55  TLi */
         /* Messigny Chess */
         WritePiece(mov->pjzz);
         WriteSquare(mov->cdzz);
         StdString("<->");
         WritePiece(mov->ppri);
         WriteSquare(mov->cazz);
      } else {
        if (mov->sb3what!=vide)             /* V3.71  TM */
        {
          StdString("[");
          WriteSquare(mov->sb3where);
          StdString("=");
          WritePiece(mov->sb3what);
          StdString("]");
        }
         if (WriteSpec(mov->speci, vide) || (mov->pjzz != pb && mov->pjzz != pn))        /* V3.31  NG */
             WritePiece(mov->pjzz);
#ifdef DATABASE
   if (two_same_pieces) {
      WriteSquare(mov->cdzz);
      if (mov->ppri == vide)
         StdChar('-');
      else
         StdString("\\x ");
   } else {
      if (mov->ppri != vide)
            StdString("\\x ");
   }
#else
   WriteSquare(mov->cdzz);
   if (mov->ppri == vide)
      StdChar('-');
   else
      StdChar('*');
#endif /* DATABASE */
   if (mov->cpzz != mov->cazz) {           /* V3.01  NG */
      if (is_pawn(mov->pjzz)) {       /* V3.31  NG */
         WriteSquare(mov->cazz);
         StdString(" ep.");
      } else {
         WriteSquare(mov->cpzz);
         StdChar('-');
         WriteSquare(mov->cazz);
      }
   } else
      WriteSquare(mov->cazz);
   }

   if (mov->bool_norm_cham_prom)                   /* V3.1  TLi */
      SETFLAG(mov->speci, Chameleon);

#ifdef	NODEF			/* V3.62  TLi */
   if ((mov->pjzz != mov->pjazz) || TSTFLAG(mov->new_spec, HalfNeutral)) {          /* V3.1  TLi */
      if (mov->pjazz == vide) {
         if (mov->promi)         /* V2.4d  TM */
            StdString ("=I");
      } else {
         if (abs(mov->pjzz) != abs(mov->pjazz)) {
            StdChar('=');
            WriteSpec(mov->speci, vide);
            WritePiece(mov->pjazz);
         }
         if (TSTFLAG(mov->new_spec, HalfNeutral)) {	/* V3.57  SE */
#ifdef NODEF						/* V3.57  NG */
   			StdString("=(");
      		if (TSTFLAG(mov->new_spec, Neutral))
      			StdString("n");
      		else
      			StdString(mov->tr==blanc ? "w" : "b");	
      		StdString(")");
#endif	/* NODEF */
                sprintf(GlobalStr, "=(%c)", TSTFLAG(mov->new_spec, Neutral) ? 'n' : 	/* V3.57  NG */
					    (mov->tr == blanc) ? WhiteChar : BlackChar);	/* V3.57  NG */
                StdString(GlobalStr);							/* V3.57  NG */
   	 } else
         if ((mov->pjzz < vide) ^ (mov->pjazz < vide)) {
            sprintf(GlobalStr, "=(%c)", (mov->tr == noir) ? WhiteChar : BlackChar);
            StdString(GlobalStr);
         }
      }
   }
#endif
   if ((mov->pjzz != mov->pjazz) || ((mov->speci != mov->new_spec) && (mov->new_spec != 0))) {
            /* V3.62  TLi */
      if (mov->pjazz == vide) {
         if (mov->promi)         /* V2.4d  TM */
            StdString ("=I");
      } else {
            StdChar('=');
            WriteSpec(mov->new_spec, mov->speci != mov->new_spec);
            WritePiece(mov->pjazz);
      }
   }

   if (mov->sqren != initsquare) {                 /* V2.70  TLi */
      piece   p= CondFlag[antieinstein] ? inc_einstein(mov->ppri) :   /* V3.50  TLi */
            CondFlag[parrain] ? mov->ren_parrain :       /* V3.1  TLi */
            CondFlag[chamcirce] ? ChamCircePiece(mov->ppri) :  /* V3.45  TLi */
            (anyclone && abs(mov->pjzz) != roib) ? -mov->pjzz :
            (anytraitor && abs(mov->ppri) >= roib) ? -mov->ppri : mov->ppri;
		StdString(" [+");
		WriteSpec(mov->ren_spec, p);            /* V3.1  TLi */
		WritePiece(p);

		WriteSquare(mov->sqren);
      if (mov->bool_cir_cham_prom)                    /* V3.1  TLi */
         SETFLAG(mov->ren_spec, Chameleon);
      if (mov->cir_prom) {                    /* V1.6c  NG */
         StdChar('=');
         WriteSpec(mov->ren_spec, p);
         WritePiece(mov->cir_prom);
      }
      /* V3.1  TLi */
      if (TSTFLAG(mov->ren_spec, Volage) &&
          SquareCol(mov->cpzz) != SquareCol(mov->sqren)) {
         sprintf(GlobalStr, "=(%c)", (mov->tr == blanc) ? WhiteChar : BlackChar);
         StdString(GlobalStr);
      }
      StdChar(']');
   }

   if (mov->sb2where!=initsquare) {  /* V3.71 TM */
#ifdef ASSERT
     assert(mov->sb2what!=vide);
#endif	/* ASSERT */
     StdString(" [");
     WriteSquare(mov->sb2where);
     StdString("=");
     WritePiece(mov->sb2what);
     StdString("]");
   }
   
	/* V3.50 SE */
	if (CondFlag[republican] && (sq= mov->repub_k) <= haut && sq >= bas) {
		SETFLAG(mov->ren_spec, mov->tr==blanc ? Black : White);
		StdString("[+");
		WriteSpec(mov->ren_spec, roib);
		WritePiece(roib);
		WriteSquare(sq);
		StdChar(']');
	}

   if (mov->renkam) {                      /* V2.4d  TM, V2.60  NG, V3.1  TLi */
      StdChar('[');
      WriteSpec(mov->speci, mov->pjazz);
      WritePiece(mov->pjazz);
      WriteSquare(mov->cazz);
      StdString("->");
      WriteSquare(mov->renkam);
      if (mov->norm_prom != vide && !anyanticirce) {        /* V3.1, V3.62  TLi */
         StdChar('=');
         WriteSpec(mov->speci, mov->tr == blanc ? 1 : -1);
         WritePiece(mov->norm_prom);
      }
      StdChar(']');
   }
   if (mov->numi && CondFlag[imitators]) {         /* V2.4d  TM */
      diff = im0 - isquare[0];
      StdChar('[');
      for (icount = 1; icount <= mov->numi;) {
         StdChar('I');
         WriteSquare(isquare[icount-1] + mov->sum + diff);
         if (icount++ < mov->numi)
            StdChar(',');
      }
      StdChar(']');
   }
   if (mov->bool_senti) {                          /* V2.90 TLi */
      StdString("[+");
      StdChar((!SentPionNeutral || !TSTFLAG(mov->speci, Neutral)) ?
         ((mov->tr == blanc) ^ SentPionAdverse ? WhiteChar : BlackChar) :
         'n');
                /* V3.1  TLi */ /* V3.50 SE */
      WritePiece(sentinelb); WriteSquare(mov->cdzz);
      StdChar(']');
   }
   if (TSTFLAG(mov->speci, ColourChange) && (abs(e[mov->hurdle])>roib)) {  /* V3.64 SE */
      couleur hc= e[mov->hurdle] < vide ? noir : blanc;
      StdString("[");
      WriteSquare(mov->hurdle);
      StdString("=");
      StdChar(hc == blanc ? WhiteChar : BlackChar);
      StdString("]");
   }
   } /* No castling */             /* V3.35  NG */
   if (flende)                             /* V2.90  TLi */
   {	
   	if (stipulation == stip_mate_or_stale)        /* V3.60 SE */
      {
      	if (mate_or_stale_patt)
         	strcpy(AlphaEnd, " =");
         else
         	strcpy(AlphaEnd, " #");
      }
      StdString(AlphaEnd);
   }
   else
      if (mov->echec)
         StdString(" +");
   flende= false;
   StdChar(bl);
} /* editcoup */

boolean nowdanstab(smallint n)
{
   smallint i;
   coup mov;

   current(&mov);
   for (i = tabsol.cp[n-1]+1; i <= tabsol.cp[n]; i++)
      if ((mov.cdzz == tabsol.liste[i].cdzz) &&
	  (mov.cazz == tabsol.liste[i].cazz) &&
	  (mov.norm_prom == tabsol.liste[i].norm_prom) &&
          (mov.cir_prom == tabsol.liste[i].cir_prom) &&
          (mov.bool_cir_cham_prom == tabsol.liste[i].bool_cir_cham_prom) &&   /* V3.1  TLi */
	  (mov.bool_norm_cham_prom == tabsol.liste[i].bool_norm_cham_prom) &&
          (mov.sb3where==tabsol.liste[i].sb3where) &&  /* V3.71 TM */
          (mov.sb3what==tabsol.liste[i].sb3what) &&  /* V3.71 TM */
          (mov.sb2where==tabsol.liste[i].sb2where) &&  /* V3.71 TM */
          (mov.sb2what==tabsol.liste[i].sb2what) &&  /* V3.71 TM */
		(!supergenre ||                    /* V3.50 SE */
		 (!(CondFlag[supercirce] || CondFlag[april]) || mov.sqren == tabsol.liste[i].sqren) &&
		  (!CondFlag[republican] || mov.repub_k == tabsol.liste[i].repub_k)))
        return true;
   return false;
}

void ecritcoup(void)
{
   coup mov;

   current(&mov);
   editcoup(&mov);
}

void videtabsol(smallint t)                     
{
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

void linesolution(void)
{
   smallint        num= 0;
		couleur         camp;

	sic_coup= nbcou;
	sic_ply= nbply;

#ifndef DATABASE
   if (OptFlag[intelligent]) {   /* V3.45  TLi */
       if (SolAlreadyFound())
      return;
       StoreSol();    
   }
#endif

   flag_writinglinesolution= true;      /* V3.44  TLi */
   repere[nbply + 1]= nbcou;
   camp= trait[nbply= 2];
   ResetPosition();
#ifdef NODEF	/* syntax error */
   if ( (flag_atob && SatzFlag) || ((!flag_atob && OptFlag[appseul]) || SatzFlag ) && !FlowFlag(Intro)) ) {
#endif	/* NODEF */
   if ( ((!flag_atob && OptFlag[appseul]) || SatzFlag ) && !FlowFlag(Intro) ) { /* V3.44  TLi, V3.70  SE,NG */
      StdString("  1...");
      num= 1;
      if (OptFlag[appseul] && SatzFlag && !flag_atob)
         StdString("  ...");
      else
         camp= advers(camp);
   }

   /* seriesmovers with introductory move  V3.31 TLi */
   if (FlowFlag(Intro) && !SatzFlag) {
      StdString("  1.");
      num= 1;
      nbcou= repere[nbply+1];
      jouecoup();
      ecritcoup();
      nbply++;
      camp= advers(camp);
   }

   while (nbply <= sic_ply) {
      if (FlowFlag(Intro) && trait[nbply] != camp   /* V3.44  TLi */
                          && nbply < sic_ply) {  /* V3.56  TLi */
         camp= advers(camp);
         num= 0;
      }
      if (trait[nbply] == camp) {             /* V3.44  TLi */
         sprintf(GlobalStr,"%3d.",++num);
         StdString(GlobalStr);
      }
      flende= sic_ply == nbply;
      nbcou= repere[nbply + 1];
      jouecoup();
      ecritcoup();
      nbply++;
   }
   Message(NewLine);
   nbcou= sic_coup;
   nbply= sic_ply;

   flag_writinglinesolution= false;     /* V3.44  TLi */
} /* end of linesolution */

EXTERN smallint WhMovesLeft, BlMovesLeft;      /* V3.45  TLi */

#ifndef DATABASE
boolean last_h_move(couleur camp)
{
   couleur ad= advers(camp);
   boolean flag= false;

   /* double mate   V3.32  TLi */
   if (DoubleMate) {
       if (CounterMate) {
      if (!stip_mate(ad))
          return false;
       } else {
      if (patt(camp))
          return false;
       }
   }

   if (!(SortFlag(Self) && SortFlag(Help)))  /* V3.0  TLi */
      GenMatingMove(camp);
   else
      genmove(camp);

   if (camp == blanc) 
       WhMovesLeft--;
   else
       BlMovesLeft--;

   while (encore()) {
      if (jouecoup()          /* V3.44  SE/TLi */
          && (!OptFlag[intelligent] || MatePossible())) {
                 /* V3.45  TLi */
         if (SortFlag(Self) && SortFlag(Help)) {
            if (! echecc(camp) && dsr_e(ad,1)) {
               GenMatingMove(ad);              /* V3.37  NG */
               while (encore()) {
                  if (jouecoup() && (*stipulation)(ad)) {  /* V3.44  SE/TLi */
                     flag = true;  /* V3.13  TLi */
                     linesolution();
                     solutions++;
                  }
                  repcoup();
               }
               finply();
            }
         } else {
            if ((*stipulation)(camp)) {
               flag= true;
               linesolution();
               solutions++;
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
   integer zae = 0, ntcount=0;

   if ((!FlowFlag(Exact) || enonce == 1) &&
      SortFlag(Direct) && (*stipulation)(ad))      /* to find short solutions */
      return -1;

   if (SortFlag(Reflex) && matant(camp,1))      /* V3.73  TLi */
      return 0;

   if ((n > droh) && !echecc(camp) && !((droh > 0) && dsr_ant(ad,droh)))
         return(maxdefen + 1);

   if (n > 2 && OptFlag[solflights]) {             /* V3.12  TLi */
      integer zae = maxflights + 1;
      square  x = camp == noir ? rn : rb;
      genmove(camp);
      while (encore() && (zae > 0)) {
         if (jouecoup() && (x != (camp == noir ? rn : rb))) /* V3.44  SE/TLi */
            if (!echecc(camp))
               zae--;
         repcoup();
      }
      finply();
      if (zae == 0)
         return (maxdefen + 1);
   }

   /* Check whether black has more non trivial moves than he is
   ** allowed to have. The number of such moves allowed (NonTrivialNumber)
   ** is entered using the nontrivial option. */
   if (n > NonTrivialLength) {   /* V3.12  TLi */
      ntcount= -1;
      /* Initialise the counter. It is counted down. */

      /* generate a ply */
      genmove(camp);

      /* test all possible moves */
      while (encore() && NonTrivialNumber >= ntcount) {
         /* Test whether the move is legal and not trivial. */
         if (jouecoup() && !echecc(camp)
                        && !((NonTrivialLength > 0) &&
                              dsr_ant(ad, NonTrivialLength))) /* V3.44 SE/TLi */
         /* The move is legal and not trivial. Hence increment the  counter. */
         ntcount++;

         repcoup();
      }
      finply();

      if (NonTrivialNumber < ntcount)
          return (maxdefen+1);
      NonTrivialNumber -= ntcount;
   } /* nontrivial */

   if ((n > 2) && !(  TSTFLAG(PieSpExFlags, Neutral)    /* V3.02  TLi */
                    || flag_testlegality                /* V3.45  TLi */
                    || anymars                          /* V3.62  TLi */
                    || CondFlag[brunner]))              /* V3.60  TLi */
      optimize= true;

   genmove(camp);
   optimize= false;

   while (encore() && tablen(t) <= maxdefen) {
      if (jouecoup() && !echecc(camp)) {   /* V3.44  SE/TLi */
         pat= false;
         if(!dsr_ant(ad,n))
            pushtabsol(t);
      }
      repcoup();
   }
   finply();

   if (n > NonTrivialLength)
      NonTrivialNumber += ntcount;

   return pat ? (maxdefen + 1) : tablen(t);
} /* dsr_def */

boolean dsr_parmena(couleur camp, smallint n, smallint t)
{
   smallint zaehler= 0;
   boolean flag= false;
   couleur ad= advers(camp);

   if (!tablen(t))
      return true;
   genmove(camp);
   while (encore() && !flag){
      if (jouecoup() && nowdanstab(t) && !echecc(camp)) {   /* V3.44  TLi */
         if (flag= !(n == 1 &&
                SortFlag(Direct) ? (*stipulation)(camp) : dsr_e(ad, n))) {
            coupfort();
         } else
            zaehler++;
      }
      repcoup();
   }
   finply();
   return (zaehler < tablen(t));
}

void dsr_vari(couleur camp, smallint n, smallint par, boolean appa) {
   couleur ad= advers(camp);
   smallint mats, mena, y, nrmena= 1, i, ntcount;
   boolean indikator;

   if (!SortFlag(Direct) && (n == 1 || (appa && dsr_e(ad,1)))) {
      genmove(ad);
      while(encore()) {
         if (jouecoup() && (*stipulation)(ad)) {  /* V3.44  SE/TLi */
            StdString("\n");
            Tabulate();
            sprintf(GlobalStr,"%3d...",zugebene);
            if ((zugebene == 1) && OptFlag[maxsols]) {
               solutions++;
            }
            StdString(GlobalStr);
            flende= true;
            ecritcoup();
         }
         repcoup();
	if (zugebene == 1) {				/* V3.53  NG */
        	if ((OptFlag[maxsols] && (solutions >= maxsolutions)) ||
		    (FlagTimeOut))			/* V3.54  NG */
			break;
        }
      }
      finply();
      StdString("\n");
      return;
   }
   if (!OptFlag[solvariantes] || SortFlag(Direct) && n == 1) {
      Message(NewLine);
      return;
   }

   n--;
   alloctab(&mena);
   if (appa || OptFlag[nothreat] || echecc(ad))
      StdString("\n");
   else {
      y = n > droh ? droh : n;
      DrohFlag= true;
      marge+= 4;
      for (i= 1;i <= y;i++){
         dsr_sol(camp,i,mena,False);  /* V3.44  TLi */
         if (tablen(mena)){
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
        ntcount= -1;              /* V3.63  TLi */
        /* generate a ply */
        genmove(ad);

        /* test all possible moves */
        while (encore()) {
            /* Test whether the move is legal and not trivial. */
            if (jouecoup() && !echecc(ad)
                   && !((NonTrivialLength > 0)
                        && dsr_ant(camp, NonTrivialLength)))  /* V3.44 SE/TLi */
                /* The move is legal and not trivial.
                ** Increment the counter. */
                ntcount++;
            repcoup();
        }
        finply();

        NonTrivialNumber -= ntcount;
    } /* nontrivial */

   genmove(ad);
   while(encore()){
      if (jouecoup() && !echecc(ad) && (!nowdanstab(par))) {
         indikator= appa ? dsr_ant(camp,n) :       /* V3.39  TLi */
            OptFlag[noshort] ? !dsr_ant(camp,n-1) :
               nrmena < 2 || !dsr_ant(camp,nrmena-1);

         if (!SortFlag(Direct) && indikator)
            indikator= !(*stipulation)(ad);
         if (indikator && dsr_parmena(camp,nrmena,mena)){
            Tabulate();
            sprintf(GlobalStr,"%3d...",zugebene);
            StdString(GlobalStr);
            ecritcoup();
            StdString("\n");
            marge+= 4;
            for (i= FlowFlag(Exact) ? n : nrmena; i <= n; i++){
#ifdef NODEF /* V3.63  TLi */
               if (n > NonTrivialLength)        /* V3.32 */
                   NonTrivialNumber -= n-i;
#endif
               alloctab(&mats);
               dsr_sol (camp,i,mats, False);   /* V3.44  TLi */
               freetab();
#ifdef NODEF /* V3.63  TLi */
               if (n > NonTrivialLength)        /* V3.32 */
                   NonTrivialNumber += n-i;
#endif
               if (tablen(mats))
                  break;
            }
            if (!tablen(mats)) {               /* V3.41  TLi */
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

   if (n > NonTrivialLength)
       NonTrivialNumber += ntcount;
} /* dsr_vari */

void dsr_sol(couleur camp, smallint n, smallint t, boolean restartenabled) {  /* V3.44  TLi */
   couleur ad= advers(camp);
   smallint nbd, def;

   if ((n == enonce) && !FlowFlag(Semi) && SortFlag(Reflex)) {  /* V3.44  TLi */
      if (matant(camp,1)) {
      	if (stipulation == stip_mate_or_stale)    /* V3.60 SE */
         {
         	if (mate_or_stale_patt)
            	strcpy(AlphaEnd, " =");
            else
            	strcpy(AlphaEnd, " #");
         }
         sprintf(GlobalStr, "%s1:\n", AlphaEnd);    /* V3.37  TLi */
         StdString(GlobalStr);                      /* V3.37  TLi */
         StipFlags|= SortBit(Direct);
         StipFlags|= FlowBit(Semi);
         alloctab(&def);
         dsr_sol(camp,1,def,False);           /* V3.44  TLi */
         freetab();
         return;
      }
   }

   zugebene++;
   genmove(camp);
   while (encore()) {
      if (jouecoup() &&
          !(restartenabled && MoveNbr < RestartNbr) &&   /* V3.44  TLi */
          (!echecc(camp)) && (!nowdanstab(t))) {  /* V3.44  SE/TLi */
         alloctab(&def);
         if (n == 1 && SortFlag(Direct))
            nbd= (*stipulation)(camp) ? 0 : maxdefen + 1;
         else if (n == 1 && OptFlag[quodlibet] && (*stipulation)(camp))        /* V3.60 SE */
            nbd = 0;
         else
            nbd= zugebene == 1 ? dsr_def(ad,n-1,def) :          /* V3.64  TLi */
               dsr_e(ad,n) ? 0 : maxdefen+1;
#ifdef NODEF    /* V3.64  TLi */
            nbd= zugebene == 1 && 1 < enonce ? dsr_def(ad,n-1,def) :
               dsr_e(ad,n) ? 0 : maxdefen+1;
#endif
         if (nbd <= maxdefen) {
            flende= n == 1 && SortFlag(Direct) || nbd == -1
            	|| (n == 1 && OptFlag[quodlibet] && (*stipulation)(camp));    /* V3.60 SE */
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
                  solutions++;
               } else {
                  StdString("? ");
               }
            }
            marge+= 4;
            dsr_vari(camp,n,def,false);
            if (zugebene == 1)
               videtabsol(def);
            marge-= 4;
         }
         freetab();
         if (nbd <= maxdefen)
            pushtabsol(t);
      }
      if (restartenabled)                 /* V3.44  TLi */
          IncrementMoveNbr();
      repcoup();
	if (zugebene == 1) {				/* V3.53  NG */
        	if (   (OptFlag[maxsols] && (solutions >= maxsolutions))
		    || (FlagTimeOut)			/* V3.54  NG */
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

void SolveSeriesProblems(couleur camp) {        /* V3.32   TLi */
  boolean         is_exact= FlowFlag(Exact);
  int i;

  OptFlag[appseul]= False;      /* V3.44  TLi -- no meaning in series movers
      would only distort output */

  if (SortFlag(Help))           /* V3.44  TLi */
    camp= advers(camp);         /* V3.44  TLi */

  if (FlowFlag(Intro)) {
    /* seriesmovers with introductory move  V3.31  TLi */

    camp= advers(camp);                 /* V3.44  TLi */
    /* check whether a king can be captured */
    if ((OptFlag[solapparent] && echecc(camp)) || echecc(advers(camp))) {
      ErrorMsg(KingCapture);
      return;
    }
    introseries(camp, introenonce, OptFlag[movenbr]);
  } else {
    if (OptFlag[solapparent] && !SortFlag(Direct) && !OptFlag[restart]) { 
                   /* V3.44  TLi */
      SatzFlag= True;
      if (echecc(camp))
   ErrorMsg(KingCapture);
      else
   if (SortFlag(Help)) {
     last_h_move(advers(camp));
   } else {
     zugebene++;
     dsr_vari(camp, 1, 0, True);
     zugebene--;
   }
      SatzFlag= False;
      Message(NewLine);
    }

    if (echecc(advers(camp)))
      ErrorMsg(KingCapture);
    else {
      int starti= FlowFlag(Exact) || OptFlag[restart] ? enonce : 1;  /* V3.44  TLi */
      if (OptFlag[intelligent]) {
   for (i = starti; i <= enonce; i++)                      /* V3.44  TLi */
     if (SortFlag(Help) ?
         Intelligent(1, i, shsol, camp, i) :
         Intelligent(i, 0, ser_dsrsol, camp, i)) {
		StipFlags |= FlowBit(Exact);
		if (OptFlag[stoponshort] && (i < enonce)) {	/* V3.60  NG */
			FlagShortSolsReached= true;
			break;
		}
     }	/* SortFlag(Help) */
      } else {
   for (i = starti; i <= enonce; i++) {                    /* V3.44  TLi */
     boolean restartenabled= OptFlag[movenbr] && i == enonce;
     if (SortFlag(Help) ? shsol(camp, i, restartenabled)
			: ser_dsrsol(camp, i, restartenabled)) {
		StipFlags |= FlowBit(Exact);
		if (OptFlag[stoponshort]&& (i < enonce)) {	/* V3.60  NG */
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

void SolveHelpProblems(couleur camp) {        /* V3.32   TLi */
  smallint        n= 2*enonce, i;
  boolean         is_exact= FlowFlag(Exact);

  if (SortFlag(Self)) {
    n--;
    camp= advers(camp);
  }

  if (OptFlag[appseul]){
    n--;
    camp= advers(camp);
  }

  if (OptFlag[solapparent]) {
    SatzFlag= True;
    if (echecc(advers(camp)))
      ErrorMsg(KingCapture);
    else {
      /* mataide(camp, n-1);   V3.44  TLi */
      /* we are looking for shortest set plays only */
      int starti= FlowFlag(Exact) || OptFlag[restart] ? n-1 : ((n-1)&1 ? 1 : 2);

      if (OptFlag[intelligent]) {
   WhMovesLeft= BlMovesLeft= starti/2;
   if (starti & 1)
       WhMovesLeft++;

   for (i= starti  ; i <= n-1; i+=2) {     /* V3.44  TLi */
     boolean flag= Intelligent(WhMovesLeft, BlMovesLeft, mataide, camp, i);

     WhMovesLeft++; BlMovesLeft++;   /* V3.45  TLi */

     if (flag)
       break;
   } /* for i */
      } else {
   for (i= starti  ; i <= n-1; i+=2)     /* V3.44  TLi */
     if (mataide(camp, i, OptFlag[movenbr] && i == n-1))
       break;

#ifdef NODEF	/* Test, if new hashing resolves this problem !  V3.60  NG */
/* I think due to hashing problems, sometimes the solution is */
/* not found if  the setplay was calculated before.  NG       */
   closehash();                    /* V3.42  NG */
   inithash();                     /* V3.42  NG */
#endif	/* NODEF */

      } /* intelligent */
    } /* echecs(advers(camps)) */
    StdChar('\n');
    SatzFlag= False;
  } /* OptFlag[solapparent] */

  if (echecc(camp))
    ErrorMsg(KingCapture);
  else {
    /* mataide(advers(camp), n);  V3.44  TLi */
    int starti= FlowFlag(Exact) || OptFlag[restart] ? n : (n&1 ? 1 : 2);
    if (OptFlag[intelligent]) {
      WhMovesLeft= BlMovesLeft= starti/2;
      if (starti & 1)
   WhMovesLeft++;

      for (i= starti  ; i <= n; i+=2)  {    /* V3.44  TLi */
   if (Intelligent(WhMovesLeft, BlMovesLeft, mataide, advers(camp), i)) {
	StipFlags |= FlowBit(Exact);
        if (OptFlag[stoponshort] && (i < n)) {	/* V3.60  NG */
                FlagShortSolsReached= true;
                break;
        }
   }
   WhMovesLeft++; BlMovesLeft++;   /* V3.45  TLi */
      } /* for i */
    } else {
      for (i= starti  ; i <= n; i+=2)     /* V3.44  TLi */
   if (mataide(advers(camp), i, OptFlag[movenbr] && i == n)) {
     /* Exact has to be set to find ALL longer solutions */
        StipFlags |= FlowBit(Exact);
        if (OptFlag[stoponshort] && (i < n)) {	/* V3.60  NG */
                FlagShortSolsReached= true;
                break;
        }
    }
    } /* OptFlag[intelligent] */
  }

  if (!is_exact)
    StipFlags &= ~FlowBit(Exact);
} /* SolveHelpProblems */

void SolveDirectProblems(couleur camp) {            /* V3.32  TLi */
  smallint lsgn;

  if ((OptFlag[solapparent] && (enonce > 1)) ||
      OptFlag[postkeyplay]) {           /* V3.41  TLi */
    if (echecc(camp))
      ErrorMsg(SetAndCheck);
    else {
      alloctab(&lsgn);
      zugebene++;
      /* dsr_vari(camp, enonce, lsgn, true);    V3.41  TLi */
      dsr_vari(camp, enonce, lsgn, !OptFlag[postkeyplay]);  /* V3.41  TLi */
      zugebene--;
      freetab();
      Message(NewLine);
    } /* echecc(camp) */
  }
  if (!OptFlag[postkeyplay]) {     /* V3.41  TLi */
    if (echecc(advers(camp)))
      ErrorMsg(KingCapture);
    else {  
      alloctab(&lsgn);
      dsr_sol(camp, enonce, lsgn, OptFlag[movenbr]);   /* V3.44  TLi */
      freetab();
    }
  }
} /* SolveDirectProblems */

void initduplex(void) {    /* V3.50  TLi */
/*
** A hack to make the intelligent mode work with duplex.
** But anyway I have to think about the intelligent mode again 
*/
square *bnp, rsq;

    rsq= rb%24+24*(23-rb/24);
    rb= rn%24+24*(23-rn/24);
    rn= rsq;
    for (bnp= boardnum; *bnp; bnp++)
   if (!TSTFLAG(spec[*bnp], Neutral) && e[*bnp] != vide) {
       e[*bnp]= -e[*bnp];
       spec[*bnp]^= BIT(White)+BIT(Black);
   }
    for (bnp= boardnum; *bnp < (bas+haut)/2; bnp++) {
   square sq2= *bnp%24+24*(23-*bnp/24);

   piece p= e[sq2];
   Flags sp= spec[sq2];

   e[sq2]= e[*bnp];
   spec[sq2]= spec[*bnp];

   e[*bnp]= p;
   spec[*bnp]= sp;
    }
} /* initduplex */

main (int argc, char *argv[])
{
   Token   tk= BeginProblem;
   int     i,l;            /* V3.15  ElB */
   boolean flag_starttimer;
   char    *ptr, ch= 'K';	/* V3.62  TLi, V3.63  NG */

   i=1;
   MaxMemory= 0;
   while (i<argc) {
       if (strcmp(argv[i], "-maxpos")==0) {
      /* MaxPositions= atoi(argv[++i]);      V3.52  TLi */
      MaxPositions= atol(argv[++i]);      /* V3.52  TLi */
      i++;
      continue;
       }
       else if (strcmp(argv[i], "-maxmem")==0) {
      /* MaxMemory= atoi(argv[++i])*1024;    V3.52  TLi */
      /* MaxMemory= atol(argv[++i])*1024;    V3.52  TLi */
      MaxMemory= strtol(argv[++i], &ptr, 10);
      if (argv[i] == ptr) {
          MaxMemory= 0;
      } else {
          MaxMemory= MaxMemory<<10;
          if (*ptr == 'M') {
              MaxMemory= MaxMemory<<10;
	      ch= 'M';
          } else if (*ptr == 'G') {
                   MaxMemory= MaxMemory<<20;
	           ch= 'G';
		 } 

#ifdef NODEF	/* V3.63  NG */
	/* useless to shift (10+30=40) bits, as long as an  unsigned long has 32 bits ! */
          if (*ptr == 'T')
              MaxMemory= MaxMemory<<30;
#endif /* NODEF */

      }

#ifdef NODEF
	/*  //  is NOT an  ANSI-C  comment !!!	Grrrhhhh, V3.63  NG */
      //MaxMemory= (unsigned long)(atol(argv[++i]))*(unsigned long)1024;	/* V3.62  NG */
#endif /* NODEF */

      i++;
      continue;
       } /* "-maxmem" */
       else 
          break;
   } /* while */

   if (!MaxMemory)
#ifdef DOS				/* V3.37  NG */
#if defined(__TURBOC__)
   /* MaxMemory= farcoreleft(); */
      MaxMemory= (unsigned long)coreleft();                /* TLi, V3.62  NG */
#else /*! __TURBOC__*/
      /* DOS-default  256 KB */
      MaxMemory= (unsigned long)256*1024;    /* V3.52, V3.62  NG */
#endif /*__TURBOC__*/
#else /* ! DOS */
#if defined(WIN16)               /* TBa */
      MaxMemory= (unsigned long)1024*1024;   /* TBa */
#else  /* !WIN16 */              /* TBa */
      /* UNIX-default   2 MB */ 
      MaxMemory= (unsigned long)2048*1024;   /* V3.52, V3.62  NG */
#endif /* !WIN16 */              /* TBa */
#endif /* ! DOS */

   if (i < argc)
      OpenInput(argv[i]);
   else
      OpenInput(" ");

   /* if we are running in an environment which supports
    * signals, we initialize the signal handling here
    */
#if defined(SIGNALS)
   /* Set the timer (interrupt handling needs this !)  V3.60  NG */
   StartTimer();
   pyInitSignal();
#endif /*SIGNALS*/

   /* We do not issue our startup message via the language
    * dependant Msg-Tables, since there the version is
    * too easily changed, or not updated.
    * StartUp is defined in pydata.h.
    *                                              12Apr1992 ElB.
    */
   if ((MaxMemory>>10) < 1024 || ch == 'K')		/* V3.63  NG */
       sprintf(MMString, " (%ld KB)\n", MaxMemory/1024);
   else {
       if ((MaxMemory>>20) < 1024 || ch == 'M')	/* V3.62  TLi, V3.63  NG */
           sprintf(MMString, " (%ld MB)\n", MaxMemory>>20);
       else
           sprintf(MMString, " (%ld GB)\n", MaxMemory>>30);
   }


   pyfputs(StartUp, stdout);
   pyfputs(MMString, stdout);              /* V3.37  NG */

   /* For the very first time we try to open any *.str
    * When we reach the last remainig language, we force
    * the existence of a *.str file. If none is found,
    * InitMsgTab will exit with an appropriate message.
    * Later, when the first Token is read, we reinitialize.
    * See ReadBeginSpec in pyio.c
    *                                              12Apr1992 ElB.
    */
   l= 0;
   while  (l<LangCount-1 && InitMsgTab(l, False) == False)
      l++;
   if (l == LangCount-1)
      InitMsgTab(l, True);

   InitCheckDir();                /* V3.40  TLi */

   do {                                    /* V2.4c  NG */
       boolean printa= true;               /* V3.50  TLi */
       /* hardinit();    V3.40 TLi */
       InitBoard();    /* V3.40  TLi */
       InitCond();     /* V3.40  TLi */
       InitOpt();      /* V3.40  TLi */
       InitStip();     /* V3.40  TLi */

/* reset MaxTime timer mechanisms */
#if defined(UNIX) && defined(SIGNALS)   /* V3.54  NG */
	alarm(0);
#endif	/* defined(UNIX) && defined(SIGNALS) */
	FlagTimeOut= false;             /* V3.54  NG */
 	FlagTimerInUse= false;		/* V3.54  NG */
	FlagMaxSolsReached= false;	/* V3.60  NG */
	FlagShortSolsReached= false;	/* V3.60  NG */

	/* New problem, so reset the timer and the solutions ...  V3.60  NG */
	flag_starttimer= true;		/* V3.60  NG */

       do {
	InitAlways();  /* V3.40  TLi */

	tk= ReadProblem(tk);

      if (tk == ZeroPosition) {  /* V3.41  TLi */
          if (!OptFlag[noboard])
		WritePosition();
          tk= ReadProblem(tk);
          if (LaTeXout)               /* V3.50  TLi */
		LaTeXBeginDiagram();
          printa= false;              /* V3.50  TLi */
      }

      if (flag_starttimer) {		/* V3.60  NG */
	/* Set the timer for real calculation time */
	StartTimer();
        flag_starttimer= false;
      }

/* Now set our timers for option  MaxTime */	/* moved to this place.    V3.60  NG */
	if (OptFlag[maxtime] && !FlagTimerInUse && !FlagTimeOut) {	/* V3.54  NG */
		FlagTimerInUse= true;
#if defined(UNIX) && defined(SIGNALS)		/* V3.54  NG */
		alarm(maxsolvingtime);
#endif	/* defined(UNIX) && defined(SIGNALS) */
#ifdef WIN32					/* V3.54  NG */
		GlobalThreadCounter++;
		_beginthread((void(*)(void*))WIN32SolvingTimeOver, 0, &maxsolvingtime);
#endif	/* WIN32 */
#if defined(DOS)				/* V3.56  NG */
		VerifieMsg(NoMaxTime);
		FlagTimeOut= true;
#endif	/* DOS */
	}
         maincamp= OptFlag[halfduplex] ? noir : blanc;   /* V3.39, V3.42  NG, moved V3.62  SE */

      if (verifieposition()) {
   /*      maincamp= OptFlag[halfduplex] ? noir : blanc;   */	/* V3.39, V3.42  NG, V3.62  SE */
         if (!OptFlag[noboard])
            WritePosition();

         if (printa) {                   /* V3.50  TLi */
             if (LaTeXout)
		LaTeXBeginDiagram();
             if (tk == TwinProblem)
		StdString("a)\n\n");
         }

         StorePosition();                /* V2.70  TLi */
         if (SortFlag(Proof)) {
            ProofInitialise();
            inithash();
         /* no DUPLEX for SPG's !  V3.42  NG */
             if (FlowFlag(Alternate)) {
             	maincamp = flag_atob ? (OptFlag[appseul] ? blanc : noir) : blanc;
             	if (flag_atob && OptFlag[solapparent] && enonce>1)
             	{
             		SatzFlag= true;
             		ProofSol(advers(maincamp), enonce-1, 1);
             		SatzFlag=false;
             	}             		
		ProofSol(maincamp, enonce, OptFlag[movenbr]);
             } else {                    /* V3.42  NG */
		SeriesProofSol(enonce, OptFlag[movenbr]);
             }
             closehash();
             Message(NewLine);
         } else {
             do {
            inithash();
            if (FlowFlag(Alternate)) {
               if (SortFlag(Help)) {
                  if (OptFlag[duplex] && OptFlag[intelligent])   /* V3.50  TLi */
                      SolveHelpProblems(blanc);
                  else
                      SolveHelpProblems(maincamp);
               } else
                  SolveDirectProblems(maincamp);
            } else
               if (OptFlag[duplex] && OptFlag[intelligent])   /* V3.50  NG */
                  SolveSeriesProblems(blanc);
               else
                  SolveSeriesProblems(maincamp);  /* V3.32  TLi */
            Message(NewLine);
	    if (OptFlag[duplex]) {		/* V3.60  NG */
		maincamp= advers(maincamp);	/* Set next side to calculate for duplex "twin" */
		if (   (OptFlag[maxsols] && (solutions >= maxsolutions)) 
		    || (OptFlag[stoponshort] && FlagShortSolsReached)
		   ) {
			FlagMaxSolsReached= true;
			solutions= 0;		/* restart calculation of maxsolution after "twinning" */
		}
#if defined(HASHRATE)
		HashStats(1, "\n\n");
#endif  /* HASHRATE */
		if (OptFlag[intelligent]) {  /* V3.50  TLi */
			initduplex();
			if (!verifieposition())
				break;
		}	/* OptFlag[intelligent] */
	    }	/* OptFlag[duplex] */

#ifdef NODEF	/* V3.60  NG */
            maincamp= advers(maincamp);
            if (OptFlag[duplex] && OptFlag[intelligent]) {  /* V3.50  TLi */
                initduplex();
                if (!verifieposition())
               break;
            }
#if defined(HASHRATE)
            if (OptFlag[duplex]) {
               HashStats(1, "\n\n");
            }
#endif  /* HASHRATE */
#endif	/* NODEF */

            closehash();

             } while (OptFlag[duplex] && maincamp == noir && verifieposition());     /* V3.40, V3.62  TLi, SE */
         }
      } /* verifieposition */
      printa= false;                  /* V3.50  TLi */
      if (   (OptFlag[maxsols] && (solutions >= maxsolutions)) 
	  || (OptFlag[stoponshort] && FlagShortSolsReached)
	 ) {							/* V3.60  NG */
		FlagMaxSolsReached= true;
		solutions= 0;           /* restart calculation of maxsolution after "twinning" */
      }
      } while (tk == TwinProblem);        /* V3.40  TLi */

      if (   (FlagMaxSolsReached) 
	  || (OptFlag[intelligent] && maxsol_per_matingpos)	/* V3.60  NG */
	  || (FlagTimeOut)			/* V3.54  NG */
	 )
	      PrintTime(InterMessage);		/* V3.46  NG */
      else
	      PrintTime(FinishProblem);		/* V2.90  NG */

       StdString("\n\n\n");

       if (LaTeXout)                       /* V3.46  TLi */
           LaTeXEndDiagram();              /* V3.50  TLi */

   } while (tk == NextProblem);            /* V2.4c  NG */

   CloseInput();                           /* V2.4c  StHoe */

   if (LaTeXout)                           /* V3.46  TLi */
       LaTeXClose();

   exit(0);
} /*main */

#ifdef NOMEMSET
void memset(char *poi, char val, int len)
{
   while (len--)
      *poi++ = val;
}
#endif

#ifdef NODEF
void introseries(couleur camp, smallint n, boolean restartenabled) {  /* V3.44  TLi */
  couleur ad = advers(camp);

  /* set play */
  /* if (n < introenonce || OptFlag[solapparent]) */
  if (OptFlag[solapparent] || (FlowFlag(Exact) ? n == 0 : n < introenonce)) {
    boolean         is_exact= FlowFlag(Exact);
    int i;

    SatzFlag= True;
    closehash();
    inithash();
    for (i = FlowFlag(Exact) ? enonce : 1; i <= enonce; i++)  /* V3.44  TLi */
      if (SortFlag(Help) ? shsol(ad, i, False)
          : ser_dsrsol(ad, i, False))   /* V3.44  TLi */
   StipFlags |= FlowBit(Exact);

    if (n == introenonce)
       Message(NewLine);
    SatzFlag= False;
    if (!is_exact)
      StipFlags &= ~FlowBit(Exact);
  }

  if (n && !echecc(ad)) {
  /* generate a single move */
    genmove(camp);
    while (encore()) {
      if (jouecoup() && !echecc(camp) &&
     !(restartenabled && MoveNbr < RestartNbr))  /* V3.44  SE/TLi */
   introseries(camp, n-1, False);
      if (restartenabled)   /* V3.44  TLi */
   IncrementMoveNbr();
      repcoup();
      if (FlagTimeOut)		/* V3.54  NG */
	break;
    }
    finply();
  }
} /* introseries */
#endif
#endif  /* ! DATABASE */
