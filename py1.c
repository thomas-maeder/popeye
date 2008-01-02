/********************* MODIFICATIONS to py1.c **************************
 **
 ** Date       Who  What
 **
 ** 2006/05/01 SE   New Koeko conditions: GI-Koeko, AN-Koeko
 **
 ** 2006/05/09 SE   New conditions: SAT, StrictSAT, SAT X Y (invented L.Salai sr.)
 **
 ** 2006/06/30 SE   New condition: BGL (invented P.Petkov)
 **
 ** 2006/07/30 SE   New condition: Schwarzschacher  
 **
 ** 2007/01/28 SE   New condition: Annan Chess 
 **
 **************************** End of List ******************************/

#ifdef macintosh	      /* is always defined on macintosh's  SB */
#	define SEGM1
#	include "pymac.h"
#endif
 
#include <stdio.h>
#include <string.h>	       /* H.D. 10.02.93 prototype fuer memset */

#include "py.h"
#include "py1.h"
#include "pyproc.h"
#include "pydata.h"

void finply()
{
  nbcou = repere[nbply];
  nbply--;
}

void nextply(void)
{
  debut = nbcou;
  nbply++;
  repere[nbply]= nbcou;

  ep2[nbply]=                        /* V3.22  TLi */
    ep[nbply]= initsquare;            /* V3.0  TLi */

  /*
    The current implementation of promotions works as follows:

    - if jouecoup() moves a pawn to its promotion rank, it is promoted
    to the first available promotion piece (typically Q).

    - if repcoup() takes back a promotion, and there are other
    promotion pieces available, the piece to be promoted into next is
    saved in one of the *_prom arrays at position nbply

    - this next promotion may never take place, e.g. because the
    previous move led to the goal we are looking for

    - as a consequence, we have to clear the position nbply in the
    abovementioned arrays, either in finply() or here
  */
  norm_prom[nbply]=
    cir_prom[nbply]= vide;               /* V3.02  TLi */
  norm_cham_prom[nbply]=
    cir_cham_prom[nbply]=                /* V3.1  TLi */
    Iprom[nbply] = false;                 /* V4.03  ThM */

  /*
    Supercirce rebirths are implemented similarly to promotions ...
  */
  super[nbply]= superbas;                /* V3.1  TLi */

  /*
    start with the castling rights of the upper level
  */
  castling_flag[nbply]= castling_flag[nbply - 1];    /* V3.35  NG */
  WhiteStrictSAT[nbply]= WhiteStrictSAT[nbply-1];	/* V4.03  SE */
  BlackStrictSAT[nbply]= BlackStrictSAT[nbply-1];	/* V4.03  SE */
}

#ifdef NODEF    /* V4.03 ThM */
void nextply(void)
{
  debut = nbcou;
  nbply++;
  repere[nbply]= nbcou;
  norm_prom[nbply]=
    cir_prom[nbply]= vide;				/* V3.02  TLi */
  ep2[nbply]=						/* V3.22  TLi */
    ep[nbply]= initsquare;				/* V3.0  TLi */
  norm_cham_prom[nbply]=				
    cir_cham_prom[nbply]= false;			/* V3.1  TLi */
  super[nbply]= superbas;				/* V3.1  TLi */
  castling_flag[nbply]= castling_flag[nbply - 1];	/* V3.35  NG */
  WhiteStrictSAT[nbply]= WhiteStrictSAT[nbply-1];
  BlackStrictSAT[nbply]= BlackStrictSAT[nbply-1];
}
#endif  /* NODEF */

void InitCond(void) {	  /* V3.40  TLi */
  square *bnp, i, j;
  piece p;

  flag_madrasi= false;				/* V3.60  TLi */
  wh_exact= wh_ultra= bl_exact= bl_ultra= false;
  anyclone= anycirprom= anycirce= anyimmun= anyanticirce= anytraitor= false;
  anymars= anyantimars= anygeneva= false;		/* V4.38  NG */
  blacknull= whitenull= nullgenre= false;               /* V3.70 SE */

  immrenroib= immrenroin= cirrenroib= cirrenroin= initsquare;

  antirenai= rennormal;
  circerenai= rennormal;
  immunrenai= rennormal;
  marsrenai= rennormal;			/* V3.76  NG */
  genevarenai= rennormal;		/* V4.38  NG */

  bl_royal_sq= wh_royal_sq= initsquare;	/* V3.50  TLi */

  flagAssassin= false;			/* V3.76  NG */
  flagwhitemummer= flagblackmummer= false;
  flagmaxi= flagultraschachzwang= false;
  flagparasent= false;
  rex_mad= rex_circe= rex_immun= rex_phan=
    rex_mess_ex= rex_wooz_ex= false;		/* V3.55  TLi */
  calctransmute= false;

  for (p= vide; p < PieceCount; p++)		/* V3.45  TLi */
	NextChamCircePiece[p]= p;
  InitChamCirce= True;

  max_pn= max_pb= 8;				/* V3.60 SE */
  max_pt=16;					/* V3.63 SE */
  sentinelb= pb;
  sentineln= pn;				/* V3.70 SE */

  ClrDiaRen(PieSpExFlags);			/* V3.76  NG */

  for (bnp= boardnum; *bnp; bnp++) {
	smallint const file= *bnp%onerow - nr_of_slack_files_left_of_board; /* V1.4c NG */
	smallint const row= *bnp/onerow - nr_of_slack_rows_below_board;	/* V1.4c NG */

	CLEARFL(sq_spec[*bnp]);			/* V3.20  TLi, V3.29  NG */
	ClrDiaRen(spec[*bnp]);			/* V3.76  NG */
    sq_num[*bnp]= (smallint)(bnp-boardnum);

	/* initialise sq_spec and set grid number  V3.22  TLi */
	sq_spec[*bnp] += ((file/2)+4*(row/2)) << Grid;
	if (file!=0 && file!=nr_files_on_board-1
        && row!=0 && row!=nr_rows_on_board-1)
      SETFLAG(sq_spec[*bnp], NoEdgeSq);
  }

  for (i= bas; i < haut; i+= onerow) {	    /* V3.22  TLi, V3.29  NG */
	if (i > bas)
      if (!TSTFLAG(sq_spec[i+dir_down], SqColor))
		SETFLAG(sq_spec[i], SqColor);
    for (j= i+1; j < i+nr_files_on_board; j++)
      if (!TSTFLAG(sq_spec[j+dir_left], SqColor))
		SETFLAG(sq_spec[j], SqColor);
  }

  for (i= 0; i < CondCount; CondFlag[i++]= False) {
    /* V2.70 TLi,V2.90 NG */
	;
  }

  for (i= maxply; i > 0; inum[i--]= 0)		/* V3.41  TLi */
	;

  memset((char *) promonly, 0, sizeof(promonly));	/* V3.43
                                                       V3.44  NG */
  memset((char *) isapril,0,sizeof(isapril));
  checkhopim = false;					/* V3.64  SE */
  nocontactfunc= nokingcontact;
  OscillatingKingsTypeB[blanc]= false;  /* V3.80  SE */
  OscillatingKingsTypeB[noir]= false;
  OscillatingKingsTypeC[blanc]= false;
  OscillatingKingsTypeC[noir]= false;

  black_length = NULL;				/* V3.80  SE */
  white_length = NULL;				/* V3.80  SE */

  BGL_white= BGL_black= BGL_infinity;  /* V4.06 SE */
  BGL_whiteinfinity= BGL_blackinfinity= true;
  BGL_global= false;
} /* InitCond */

void InitOpt(void) {					/* V3.40  TLi */
  int i;

  maxflights= 64;
  maxdefen= 0;					/* V2.90  TLi */
  MoveNbr= RestartNbr= 0;				/* V3.44  TLi */

  droh= NonTrivialLength= maxply;
  ep2[nbply]= ep[nbply]= initsquare;			/* V3.50  TLi */
  no_castling= bl_castlings|wh_castlings;	/* V3.55  NG */

  for (i= 0; i < OptCount; OptFlag[i++]= False);	/* V2.70 TLi,
                                                       V2.90 NG */
  flag_appseul= false;
} /* InitOpt */

void InitCheckDir(void) {   /* V3.40  TLi */
  int i, j;

  for (i= -haut+bas; i <= haut-bas; i++)
	CheckDirQueen[i]=
      CheckDirRook[i]=
      CheckDirBishop[i]=
      CheckDirKnight[i]= 0;

  /* knight */
  for (i= 9; i <= 16; i++)
	CheckDirKnight[vec[i]]= vec[i];

  /* rook + queen */
  for (i= 1; i <= 4; i++)
	for (j= 1; j <= 7; j++)
      CheckDirQueen[j*vec[i]]=
	    CheckDirRook[j*vec[i]]= vec[i];

  /* bishop + queen */
  for (i= 5; i <= 8; i++)
	for (j= 1; j <= 7; j++)
      CheckDirQueen[j*vec[i]]=
	    CheckDirBishop[j*vec[i]]= vec[i];
} /* InitCheckDir */

void InitBoard(void) {	   /* V3.40  TLi */
  square i, *bnp;

  ActTitle[0]= ActAuthor[0]= ActOrigin[0]=
    ActTwin[0]= ActAward[0] = ActStip[0]= '\0';

  for (i= maxsquare - 1; i >= 0; i--) {		/* V2.60  NG,
                                               V3.22  TLi */
    e[i]= e_ubi[i]= e_ubi_mad[i]= obs;		/* V3.29  NG */
    spec[i]= BorderSpec;			/* V3.33  TLi */
  }

  /* dummy squares for Messigny chess and castling
     -- must be empty, V3.55	TLi
  */
  e[messigny_exchange]= e[kingside_castling]= e[queenside_castling]= vide;
  CLEARFL(spec[messigny_exchange]);
  CLEARFL(spec[kingside_castling]);
  CLEARFL(spec[queenside_castling]);

  for (bnp= boardnum; *bnp; bnp++)
	e[*bnp]= vide;

  rb= rn= initsquare;

  CLEARFL(PieSpExFlags);				 /* V2.90 TLi */

  nrhuntertypes = 0;
} /* InitBoard */

void InitStip(void) {	/* V3.40  TLi */
  FlagGenMatingMove= FlagMoveOrientatedStip= False;	 /* V3.33 TLi */
  enonce= 0;
} /* InitStip */

void InitAlways(void) {    /* V3.40  TLi */
  square i;

  memset((char *) exist, 0, sizeof(exist));	  /* V3.43  NG */

  move_generation_mode= move_generation_optimized_by_killer_move;

  nbcou = 1;
  nbply = 1;

  flagAssassin=				     /* V3.50 SE */
    flag_writinglinesolution=
    flag_testlegality= k_cap=			     /* V3.44  SE/TLi */
    flende =					     /* V2.70 TLi */
    flagfee =
    flagriders	    =				     /* V3.12  TM */
    flagleapers     =				     /* V3.12  TM */
    flagsimplehoppers     =			/* V3.74  NG */
    flagsimpledecomposedleapers =  /* V3.81 SE */
    flagsymmetricfairy  =
    flagveryfairy   = false;			     /* V3.12  TM */

  /* numenroute = 0;					V3.44  TLi */
  marge = 0;
  solutions= 0;				     /* V3.12  StHoe */

  PrintReciSolution = False;			     /* V3.31  TLi */

  for (i= maxply; i > 0; i--) {		      /* V1.5c NG */
	whduell[i]= blduell[i]=			      /* V3.0  TLi */
      kpilcd[i]= kpilca[i]=
      sqrenais[i]= initsquare;
	trait[i]= blanc;			      /* V2.60 NG */
	crenkam[i]= initsquare;			      /* V2.4d TM */
	pwcprom[i]=				      /* V3.1  TLi */
      senti[i]=
      Iprom[i]= false;			      /* V2.4d TM */
	att_1[i]= true;				      /* V3.70 SE */
    oscillatedKs[i]= false;          /* V3.81 SE */
  }

  initneutre(blanc);				     /* V3.40  TLi */
  tabsol.nbr=
    tabsol.cp[0]= 0;				     /* V2.70  TLi */
  flag_atob= false;				     /* 3.70  SE */
  dont_generate_castling=false;
  
  takemake_departuresquare= initsquare;
  takemake_capturesquare= initsquare;
} /* InitAlways */

void initneutre(couleur c) {
  /* I don't know why, but the solution below is not slower */
  /* than the double loop solution of genblanc(). V3.14  NG */ 

  square *bnp;

  if (neutcoul != c) {
	neutcoul= c;
	for (bnp= boardnum; *bnp; bnp++) {
      if (TSTFLAG(spec[*bnp], Neutral)) {
		change(*bnp);
      }
	}
  }
}

square coinequis(square i)
{
  return 75 + (onerow*(((i/onerow)+3)/2) + (((i%onerow)+3)/2));	 /* V2.60  NG */
}

boolean leapcheck(square	 sq_king,
                  numvec	 kanf,
                  numvec	 kend,
                  piece	 p,
                  evalfunction_t *evaluate)
{
  /* detect "check" of leaper p */			/* V2.60  NG */
  numvec  k;

  square sq_departure;

  for (k= kanf; k<=kend; k++) {
	sq_departure= sq_king+vec[k];
	if (e[sq_departure]==p
        && evaluate(sq_departure,sq_king,sq_king)			/* V3.02  TLi */
        && imcheck(sq_departure,sq_king))
      return true;
  }
  
  return false;
}

/*****	V3.34  TLi  ***** begin *****/
boolean riderhoppercheck(square	 sq_king,
                         numvec	 kanf,
                         numvec	 kend,
                         piece	 p,
                         smallint	 run_up,
                         smallint	 jump,
                         evalfunction_t *evaluate)
{
  /* detect "check" of a generalised rider-hopper p that runs up
     run_up squares and jumps jump squares. 0 indicates an
     potentially infinite run_up or jump.
     examples:  grasshopper:	     run_up: 0	 jump: 1
     grasshopper2:      run_up: 0	 jump: 2
     contragrasshopper: run_up: 1	 jump: 0
     lion:		     run_up: 0	 jump: 0
  ********/

  piece   hurdle, hopper;
  square  sq_hurdle;
  numvec  k;

  square sq_departure;

  for (k= kanf; k <= kend; k++) {
	if (jump) {
      sq_hurdle= sq_king;
      if (jump>1) {
		smallint jumped= jump;
		while (--jumped) {
          sq_hurdle+= vec[k];
          if (e[sq_hurdle]!=vide)
            break;
        }
      
		if (jumped)
          continue;
      }
      sq_hurdle+= vec[k];
      hurdle= e[sq_hurdle];
	}
	else
      /* e.g. lion, contragrashopper */
      finligne(sq_king,vec[k],hurdle,sq_hurdle);

	if (abs(hurdle)>=roib) {
      if (run_up) {
		/* contragrashopper */
		sq_departure= sq_hurdle;
		if (run_up>1) {
          smallint ran_up= run_up;
          while (--ran_up) {
            sq_hurdle+= vec[k];
            if (e[sq_hurdle]!=vide)
              break;
          }
          if (ran_up)
			continue;
		}
        sq_departure+= vec[k];
		hopper= e[sq_departure];
      }
      else
		/* grashopper, lion */
		finligne(sq_hurdle,vec[k],hopper,sq_departure);

      if (hopper==p
	      && evaluate(sq_departure,sq_king,sq_king)
	      && hopimcheck(sq_departure,sq_king,sq_hurdle,-vec[k]))
		return true;
	}
  }
  return false;
} /* end of riderhoppercheck */
/*****	V3.34  TLi  *****  end	*****/

boolean ridcheck(square	sq_king,
                 numvec	kanf,
                 numvec	kend,
                 piece	p,
                 evalfunction_t	*evaluate)
{
  /* detect "check" of rider p */			/* V2.60  NG */
  piece rider;
  numvec  k;

  square sq_departure;

  for (k= kanf; k<= kend; k++) {
	finligne(sq_king,vec[k],rider,sq_departure);
	if (rider==p
        && evaluate(sq_departure,sq_king,sq_king)
        && ridimcheck(sq_departure,sq_king,vec[k]))
      return true;
  }
  return false;
}

boolean marincheck(square	sq_king,
                   numvec	kanf,
                   numvec	kend,
                   piece	p,
                   evalfunction_t *evaluate)
{
  /* detect "check" of marin piece p or a locust */
  piece marine;
  numvec  k;

  square sq_departure;
  square sq_arrival;

  for (k= kanf; k<= kend; k++) {
    sq_arrival= sq_king-vec[k];
	if (e[sq_arrival]==vide) {
      finligne(sq_king,vec[k],marine,sq_departure);
      if (marine==p && evaluate(sq_departure,sq_arrival,sq_king))
        /* V3.70  TLi */
		return true;
	}
  }
  return false;
} /* marincheck */

boolean nogridcontact(square j)			/* V2.4c  NG */
{
  square  j1;
  numvec  k;
  piece   p;

  for (k= 8; k > 0; k--) {
	p= e[j1= j + vec[k]];
	if (p != vide && p != obs && GridNum(j1) != GridNum(j)) {
      return false;
	}
  }
  return true;
}

boolean noleapcontact(square ia, numvec kanf, numvec kend)
{
  numvec k;
  piece p;
  for (k= kanf; k <= kend; k++) {
    if ( (p= e[ia + vec[k]]) != obs
         && p != vide)				 /* 3.02 TLi */
    {
      /* this is faster than a call to abs() */
      return false;
    }
  }
  return true;
}

boolean nokingcontact(square ia)
{
  return noleapcontact(ia, 1, 8);
}

boolean nowazircontact(square ia)
{
  return noleapcontact(ia, 1, 4); 
}

boolean noferscontact(square ia)
{
  return noleapcontact(ia, 5, 8);
}

boolean noknightcontact(square ia)
{
  return noleapcontact(ia, 9, 16);
}

boolean nocamelcontact(square ia)
{
  return noleapcontact(ia, 33, 40);
}

boolean noalfilcontact(square ia)
{
  return noleapcontact(ia, 65, 68);
}

boolean nodabbabacontact(square ia)
{
  return noleapcontact(ia, 61, 64);
}

boolean nozebracontact(square ia)
{
  return noleapcontact(ia, 25, 32);
}

boolean nogiraffecontact(square ia)
{
  return noleapcontact(ia, 41, 48);
}

boolean noantelopecontact(square ia)
{
  return noleapcontact(ia, 49, 56);
}


boolean nocontact(square sq_departure, square sq_arrival, square sq_capture) {
  boolean	Result;
  square	cr;
  piece	pj, pp, pren;
  piece	pc= obs;				/* V3.05  NG */
  short	flag_castling= 0;			/* V3.35  NG */
  /* 0: NO, 1: SHORT, 2: LONG  castling */

  VARIABLE_INIT(cr);

  pj= e[sq_departure];
  pp= e[sq_capture];
  /* does this work with neutral pieces ??? */	/* V3.37  NG */
  if (CondFlag[haanerchess]) {
	e[sq_departure]= obs;
  }
  else if (CondFlag[sentinelles]
           && sq_departure>=square_a2 && sq_departure<=square_h7
           && !is_pawn(pj))				    /* V3.64  NG,TLi */
  {
	if ((pj<=roin) != SentPionAdverse) {		 /* V3.50 SE */
      if (nbpiece[sentineln] < max_pn	    /* V3.60,3.63 SE */
	      && nbpiece[sentinelb]+nbpiece[sentineln] < max_pt
	      && (!flagparasent
              || (nbpiece[sentineln]
                  <= nbpiece[sentinelb]+(pp==sentinelb?1:0))))	/* V3.62 SE */
      {
		e[sq_departure]= sentineln;
      }
      else {
		e[sq_departure]= vide;
      }
	}
	else { /* we assume  pj >= roib */
      if (nbpiece[sentinelb] < max_pb	    /* V3.60,3.63 SE */
	      && nbpiece[sentinelb]+nbpiece[sentineln] < max_pt
	      && (!flagparasent
              || (nbpiece[sentinelb]
                  <= nbpiece[sentineln]+(pp==sentineln?1:0))))
        /* V3.62 SE */
      {
        e[sq_departure]= sentinelb;
      }
      else {
		e[sq_departure]= vide;
      }
      /* don't think any change as a result of Sentinelles */
      /* PionNeutral is needed as piece specs not changed  */
      /* V3.50 SE */
	}
  }
  else {
	e[sq_departure]= vide;
	/* e[sq_departure] = CondFlag[haanerchess] ? obs : vide;	   */
  }

  if (sq_capture == messigny_exchange) {				 /* V3.62 SE */
	e[sq_departure]= e[sq_arrival];
  }
  else {
	/*****	TLi  V3.31  *****  begin  *****/
	/* the pieces captured and reborn may be different: */
	/* Clone, Chameleon Circe			    */
    pp= e[sq_capture];

	/* the pieces can be reborn at the square where it has been
	 * captured. For example, when it is taken by a locust or a
	 * similarly moving piece
	 */
	e[sq_capture]= vide;

	if (pp != vide && pp != obs) {	/* V1.6c, V3.05  NG,
                                       V3.1  TLi,
                                       V3.35  NG */
      if (anycirce && abs(pp) > roib) {	/* V3.1  TLi,
                                           V3.37  NG */
		/* correction by TLi V3.31
		** It still doesnot work with neutral pieces.
		** Eventually we must add the colour of the side making
		** the move or potentially giving the check to the
		** argument list!
		*/
		if (anyclone && sq_departure != rn && sq_departure != rb) {
          /* Circe Clone */
          pren = (pj * pp < 0) ? -pj : pj;
		}
		else {
          /* Chameleon Circe or ordinary Circe type */
          pren= CondFlag[chamcirce]
            ? ChamCircePiece(pp)
            : pp;				/* V3.45  TLi */
		}

		if (CondFlag[couscous]) {
          cr= (*circerenai)(pj, spec[sq_departure], sq_capture, sq_departure, sq_arrival, pp > vide ? blanc : noir);	/* V3.80  NG */
		}
		else {
          cr= (*circerenai)(pren, spec[sq_capture], sq_capture, sq_departure, sq_arrival, pp > vide ? noir : blanc);	/* V3.80  NG */
		}

		if ((pc= e[cr]) == vide) {		/* V2.90  NG */
          e[cr]= pren;
		}
      } /* anycirce && abs(pp) > roib */
	} /* pp != vide && pp != obs */			/* V3.35  NG */
	/*****	TLi  V3.31  *****  end	*****/
	else { /* no capture move */			/* V3.35  NG */
      if (abs(pj) == King && castling_supported) {
		if (sq_capture == kingside_castling) {	/* V3.63  NG */
          flag_castling= 1;
          e[sq_arrival+dir_left]= e[sq_arrival+dir_right];
          e[sq_arrival+dir_right]= vide;
		}
		else if (sq_capture == queenside_castling) { /* V3.63  NG */
          flag_castling= 2;
          e[sq_arrival+dir_right]= e[sq_arrival+2*dir_left];
          e[sq_arrival+2*dir_left]= vide;
		}
      }
	}
  }

  if (CondFlag[contactgrid]) {			/* V2.4c  NG */
	Result= nogridcontact(sq_arrival);			/* V2.5c  NG */
  }
  else {
	Result= (*nocontactfunc)(sq_arrival);
  }

  if (pc != obs) {					/* V3.02  TLi,
                                       V3.05  NG */
	e[cr]= pc;
  }

  e[sq_capture]= pp;						/* V3.02  TLi */
  e[sq_departure]= pj;
  if (flag_castling == 1) {				/* V3.35  NG */
	e[sq_arrival+dir_right]= e[sq_arrival+dir_left];
	e[sq_arrival+dir_left]= vide;
  }
  else if (flag_castling == 2) {
	e[sq_arrival+2*dir_left]= e[sq_arrival+dir_right];
	e[sq_arrival+dir_right]= vide;
  } 

  return Result;
} /* nocontact */

/*****	V3.12  TLi  *****  begin  *****/

/* new versions of StorePosition() and ResetPosition() */

Flags		sic_spec[64];
piece		sic_e[64];
smallint	sic_inum1;
imarr		sic_isquare;
square		sic_im0, rn_sic, rb_sic;
long int sic_BGL_W, sic_BGL_b;

void StorePosition(void) {
  smallint	    i;

  rn_sic= rn; rb_sic= rb;
  for (i= 0; i < 64; i++) {
	sic_e[i]= e[boardnum[i]];
	sic_spec[i]= spec[boardnum[i]];
  }

  /* imitators */
  sic_inum1= inum[1];
  for (i= 0; i < maxinum; i++) {			/* V2.90  NG */
	sic_isquare[i]= isquare[i];
  }

  sic_im0= im0;
  sic_BGL_W= BGL_white;
  sic_BGL_b= BGL_black;
} /* StorePosition */

void ResetPosition(void) {
  smallint	    i;

  for (i= dernoi; i <= derbla; i++)
	nbpiece[i]= 0;

  rn= rn_sic; rb= rb_sic;

  for (i= 0; i < 64; i++) {
	nbpiece[e[boardnum[i]]= sic_e[i]]++;
	spec[boardnum[i]]= sic_spec[i];
  }

  /* imitators */
  for (i= 1; i <= maxply; inum[i++]= sic_inum1)
	;

  for (i= 0; i < maxinum; i++)	    /* V2.90  NG */
    isquare[i]= sic_isquare[i];
  im0= sic_im0;
  neutcoul= blanc;			    /* V2.90  TLi */
  BGL_white= sic_BGL_W;
  BGL_black= sic_BGL_b;
} /* ResetPosition */

/*****	V3.12  TLi  *****  end	*****/

boolean ooorphancheck(square sq_king,
                      piece porph,
                      piece p,
                      evalfunction_t *evaluate) {
  boolean	flag= false;
  square	olist[63], *bnp;
  int		k, j, nrp, co;

  if ((*checkfunctions[abs(porph)])(sq_king,porph,evaluate))
	return true;

  if ((nrp= nbpiece[p]) == 0)
	return false;

  nbpiece[-p]--;
  e[sq_king]= dummyb;
  co= 0;
  for (bnp= boardnum; co < nrp; bnp++) {
	if (e[*bnp] == p) {
      olist[co++]= *bnp;
	}
  }
  for (k= 0; k < co; k++) {
	j= 0;
	while (j<co) {
      e[olist[j]]= k==j ? p : dummyb;
      j++;
	}
	if ((*checkfunctions[abs(porph)])(sq_king,p,evaluate)) {
      for (j= 0; j<co; j++)
		e[olist[j]]= p;
      flag= ooorphancheck(olist[k],-porph,-p,evaluate);
      if (flag)
		break;
	}
	else
      for (j= 0; j<co; j++)
		e[olist[j]]= p;
  }

  nbpiece[-p]++;
  e[sq_king]= -p;
  return flag;
}

boolean orphancheck(square	 sq_king,
                    piece	 p,
                    evalfunction_t *evaluate)
{
  piece	*porph;
  boolean	flag= false;
  boolean	inited= false;
  square	olist[63], *bnp;
  smallint	k, j, co= 0;

  for (porph= orphanpieces; *porph; porph++) {
	if (nbpiece[*porph] || nbpiece[-*porph]) {
      if (!inited) {
		inited= true;
		for (bnp= boardnum; *bnp; bnp++) {
          if (e[*bnp] == p) {
			olist[co++]= *bnp;
          }
		}
      }
      for (k= 0; k < co; k++) {
		j= 0;
		while (j < co) {
          e[olist[j]]= (k == j) ? p : dummyb;
          j++;
		}
		if ((*checkfunctions[*porph])(sq_king, p, evaluate)) {
          piece op;
          for (j= 0; j < co; e[olist[j++]]= p)
			;
          if (p == orphanb)
			op = -*porph;
          else
			op = *porph;
          flag= ooorphancheck(olist[k], op, -p, evaluate);
          if (flag)
			break;
		}
		else {
          for (j= 0; j < co; e[olist[j++]]= p)
			;
		}
      }
      if (flag)
	    return true;
	}
  }
  return false;
} /* orphancheck */

boolean fffriendcheck(square	sq_king,
                      piece	pfr,
                      piece	p,
                      evalfunction_t *evaluate)	/* V3.65  TLi */
{
  boolean	flag= false;
  square	flist[63], *bnp;
  int		k, j, nrp, cf= 0;

  if ((*checkfunctions[abs(pfr)])(sq_king, pfr, evaluate))
    return true;

  if ((nrp= nbpiece[p]-1) == 0)
    return false;

  nbpiece[p]--;
  e[sq_king]= dummyb;
  for (bnp= boardnum; cf < nrp; bnp++) {
    if (e[*bnp] == p) {
	  flist[cf++]= *bnp;
	}
  }

  for (k= 0; k < cf; k++) {
	j= 0;
	while (j < cf) {
      e[flist[j]]= (k == j) ? p : dummyb;
      j++;
	}
	if ((*checkfunctions[abs(pfr)])(sq_king, p, evaluate)) {
      for (j= 0; j < cf; e[flist[j++]]= p)
		;
      flag= fffriendcheck(flist[k], pfr, p, evaluate);
      if (flag) {
		break;
      }
	}
	else {
      for (j= 0; j < cf; e[flist[j++]]= p)
		;
	}
  }

  nbpiece[p]++;
  e[sq_king]= p;
  return flag;
} /* fffriendcheck */

boolean friendcheck(
  square	i, piece p,
  evalfunction_t *evaluate)	/* V3.65  TLi */
{
  piece	*pfr, cfr;
  boolean	flag= false;
  boolean	initialized= false;
  square	flist[63], *bnp;
  smallint	k, j, cf= 0;

  for (pfr= orphanpieces; *pfr; pfr++) {
	cfr= p == friendb ? *pfr : -*pfr;
	if (nbpiece[cfr]) {
      if (!initialized) {
		initialized= true;
		for (bnp= boardnum; *bnp; bnp++) {
          if (e[*bnp] == p) {
			flist[cf++]= *bnp;
          }
		}
      }
      for (k= 0; k < cf; k++) {
		j= 0;
		while (j < cf) {
          e[flist[j]]= (k == j) ? p : dummyb;
          j++;
		}
		if ((*checkfunctions[*pfr])(i, p, evaluate)) {
          for (j= 0; j < cf; e[flist[j++]]= p)
			;
          flag= fffriendcheck(flist[k], cfr, p, evaluate);
          if (flag) {
			break;
          }
		}
		else {
          for (j= 0; j < cf; e[flist[j++]]= p)
			;
		}
      }
      if (flag) {
		return true;
      }
	}
  }
  return false;
} /* friendcheck */

boolean whannan(square rear, square front)
{
  if (e[rear] <= obs)
    return false;
  switch(annanvar)
  {
  case 0:
    return true;
  case 1:
    return rear != rb;
  case 2:
    return front != rb;
  case 3:
    return rear != rb && front != rb;
  }
  return true;
}

boolean blannan(square rear, square front)
{
  if (e[rear] >= vide)
    return false;
  switch(annanvar)
  {
  case 0:
    return true;
  case 1:
    return rear != rn;
  case 2:
    return front != rn;
  case 3:
    return rear != rn && front != rn;
  }
  return true;
}
