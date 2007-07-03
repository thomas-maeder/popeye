/********************* MODIFICATIONS to py1.c **************************
**
** Date       Who  What
**
** 2001/01/14 SE   new specification : ColourChanging
**
** 2001/02/05 TLi  new piece: Friend
**
** 2001/05/08 SE   new conditions: SentBerolina
**
** 2001/05/20 SE   new stipulation: atob
**
** 2001/10/02 TLi  marin pieces/locust bug fixed in marincheck
**
** 2002/05/18 NG   bug fix: flagAssassin and marsrenai initialized
**                          DiaCirce 'flags and specs' reset
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
    norm_prom[nbply]=
    cir_prom[nbply]= vide;				/* V3.02  TLi */
    ep2[nbply]=						/* V3.22  TLi */
    ep[nbply]= initsquare;				/* V3.0  TLi */
    norm_cham_prom[nbply]=				
    cir_cham_prom[nbply]= false;			/* V3.1  TLi */
    super[nbply]= bas - 1;				/* V3.1  TLi */
    castling_flag[nbply]= castling_flag[nbply - 1];	/* V3.35  NG */
}

void InitCond(void) {	  /* V3.40  TLi */
    square *bnp, i, j, k;
    piece p;

    flag_madrasi= false;				/* V3.60  TLi */
    wh_exact= wh_ultra= bl_exact= bl_ultra= false;
    anyclone= anycirprom= anycirce= anyimmun= anyanticirce= anytraitor= false;
    anymars= is_phantomchess= false;

    immrenroib= immrenroin= cirrenroib= cirrenroin= initsquare;

    antirenai= rennormal;
    circerenai= rennormal;
    immunrenai= rennormal;
    marsrenai= rennormal;			/* V3.76  NG */

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
	j= *bnp % 24;				/* V1.4c NG */
	k= *bnp / 24;				/* V1.4c NG */

	CLEARFL(sq_spec[*bnp]);			/* V3.20  TLi, V3.29  NG */
	ClrDiaRen(spec[*bnp]);			/* V3.76  NG */

	/* initialise sq_spec and set grid number  V3.22  TLi */
	sq_spec[*bnp] += (((j-8)/2)+4*((k-8)/2)) << Grid;
	if (j != 8 && j != 15 && k != 8 && k != 15)
	    SETFLAG(sq_spec[*bnp], NoEdgeSq);
    }

    for (i= bas; i < haut; i+= 24) {	    /* V3.22  TLi, V3.29  NG */
	if (i > bas)
	    if (!TSTFLAG(sq_spec[i-24], SqColor))
		SETFLAG(sq_spec[i], SqColor);
	for (j= i + 1; j <= i + 7; j++)
	    if (!TSTFLAG(sq_spec[j-1], SqColor))
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

} /* InitCond */

void InitOpt(void) {					/* V3.40  TLi */
    int i;

    maxflights= 64;
    maxdefen= 0;					/* V2.90  TLi */
    MoveNbr= RestartNbr= 0;				/* V3.44  TLi */

    droh= NonTrivialLength= maxply;
    ep2[nbply]= ep[nbply]= initsquare;			/* V3.50  TLi */
    no_castling= 0x77;					/* V3.55  NG */

    for (i= 0; i < OptCount; OptFlag[i++]= False);	/* V2.70 TLi,
							   V2.90 NG */
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
    e[maxsquare+1]= e[maxsquare+2]= e[maxsquare+3]= vide;
    CLEARFL(spec[maxsquare+1]);
    CLEARFL(spec[maxsquare+2]);
    CLEARFL(spec[maxsquare+3]);

    for (bnp= boardnum; *bnp; bnp++)
	e[*bnp]= vide;

    rb= rn= initsquare;

    CLEARFL(PieSpExFlags);				 /* V2.90 TLi */
} /* InitBoard */

void InitStip(void) {	/* V3.40  TLi */
    FlagGenMatingMove= FlagMoveOrientatedStip= False;	 /* V3.33 TLi */
    enonce= 0;
} /* InitStip */

    void InitAlways(void) {    /* V3.40  TLi */
    square i;

    memset((char *) exist, 0, sizeof(exist));	  /* V3.43  NG */

    otc= 0;
    optimize= false;

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
    }

    initneutre(blanc);				     /* V3.40  TLi */
    tabsol.nbr=
    tabsol.cp[0]= 0;				     /* V2.70  TLi */
    flag_atob= false;				     /* 3.70  SE */
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
    return 75 + (24*(((i/24)+3)/2) + (((i%24)+3)/2));	 /* V2.60  NG */
}

boolean leapcheck(
    square	 i,
    numvec	 kanf,
    numvec	 kend,
    piece	 p,
    boolean	 (*evaluate)(square,square,square))
{
    /* detect "check" of leaper p */			/* V2.60  NG */
    numvec  k;
    square  j;

    for (k= kanf; k<= kend; k++) {
	j= i + vec[k];
	if (e[j] == p
	  && (*evaluate)(j, i, i)			/* V3.02  TLi */
	  && imcheck(j, i))
	{
	    return true;
	}
    }
    return false;
}

/*****	V3.34  TLi  ***** begin *****/
boolean riderhoppercheck(
    square	 i,
    numvec	 kanf,
    numvec	 kend,
    piece	 p,
    smallint	 run_up,
    smallint	 jump,
    boolean	 (*evaluate)(square,square,square))
{
    /* detect "check" of a generalised rider-hopper p that runs up
       run_up squares and jumps jump squares. 0 indicates an
       potentially infinite run_up or jump.
       examples:  grasshopper:	     run_up: 0	 jump: 1
		  grasshopper2:      run_up: 0	 jump: 2
		  contragrasshopper: run_up: 1	 jump: 0
		  lion:		     run_up: 0	 jump: 0
    ********/

    piece   p1;
    square  j, j1;
    numvec  k;

    for (k= kanf; k <= kend; k++) {
	if (jump) {
	    j= i;
	    if (jump > 1) {
		smallint jumped= jump;
		while (--jumped && e[j += vec[k]] == vide)
		    ;
		if (jumped)
		    continue;
	    }
	    p1= e[j += vec[k]];
	}
	else {
	    /* e.g. lion, contragrashopper */
	    finligne(i, vec[k], p1, j);
	}
	if (abs(p1) >= roib) {
	    if (run_up) {
		/* contragrashopper */
		j1= j;
		if (run_up > 1) {
		    smallint ran_up= run_up;
		    while (--ran_up && e[j += vec[k]] == vide)
			;
		    if (ran_up)
			continue;
		}
		p1= e[j1 += vec[k]];
	    }
	    else {
		/* grashopper, lion */
		finligne(j, vec[k], p1, j1);
	    }
	    if (p1 == p
	      && (*evaluate)(j1, i, i)
	      && hopimcheck(j1, i, j,-vec[k]))
	    {
		return true;
	    }
	}
    }
    return false;
} /* end of riderhoppercheck */
/*****	V3.34  TLi  *****  end	*****/

boolean ridcheck(
    square	i,
    numvec	kanf,
    numvec	kend,
    piece	p,
    boolean	(*evaluate)(square, square,square))
{
    /* detect "check" of rider p */			/* V2.60  NG */
    piece p1;
    square j1;
    numvec  k;

    for (k= kanf; k<= kend; k++) {
	finligne(i, vec[k], p1, j1);
	if (p1 == p
	  && (*evaluate)(j1, i, i)
	  && ridimcheck(j1, i, vec[k]))
	{
		return true;
	}
    }
    return false;
}

boolean marincheck(
    square	i,
    numvec	kanf,
    numvec	kend,
    piece	p,
    boolean	(*evaluate)(square,square,square))
{
/* detect "check" of marin piece p or a locust */
    piece p1;
    square j1;
    numvec  k;

    for (k= kanf; k<= kend; k++) {
	if (e[i - vec[k]] == vide) {
	    finligne(i, vec[k], p1, j1);
	    if (p1 == p && (*evaluate)(j1, i - vec[k], i)) {
						       /* V3.70  TLi */
		return true;
	    }
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

boolean nocontact(square id, square ia, square ip) {
    /* id - case dep., ia - case arr., ip - case pri. */
    boolean	Result= true;
    square	cr;
    piece	pj, pp, p, pren;
    piece	pc= obs;				/* V3.05  NG */
    numvec	k;
    short	flag_castling= 0;			/* V3.35  NG */
    /* 0: NO, 1: SHORT, 2: LONG  castling */

    VARIABLE_INIT(cr);

    pj= e[id];
    pp= e[ip];
    /* does this work with neutral pieces ??? */	/* V3.37  NG */
    if (CondFlag[haanerchess]) {
	e[id]= obs;
    }
    else if (CondFlag[sentinelles]
      && id > bas+8
      && id < haut-8
      && !is_pawn(pj))				    /* V3.64  NG,TLi */
    {
	if ((pj <= roin) ^ SentPionAdverse) {		 /* V3.50 SE */
	    if (nbpiece[sentineln] < max_pn	    /* V3.60,3.63 SE */
	      && nbpiece[sentinelb]+nbpiece[sentineln] < max_pt
	      && (!flagparasent
		  || (nbpiece[sentineln]
			<= nbpiece[sentinelb]+(pp==sentinelb?1:0))))	/* V3.62 SE */
	    {
		e[id]= sentineln;
	    }
	    else {
		e[id]= vide;
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
		    e[id]= sentinelb;
	    }
	    else {
		e[id]= vide;
	    }
	    /* don't think any change as a result of Sentinelles
	       PionNeutral is needed as piece specs not changed v3.50
								 SE
	     ****/
	}
    }
    else {
	e[id]= vide;
	/* e[id] = CondFlag[haanerchess] ? obs : vide;	   */
    }

    if (ip == maxsquare+1) {				 /* V3.62 SE */
	e[id]= e[ia];
    }
    else {
	/*****	TLi  V3.31  *****  begin  *****/
	/* the pieces captured and reborn may be different: */
	/* Clone, Chameleon Circe			    */
	pren= pp= e[ip];

	/* the pieces can be reborn at the square where it has been
	 * captured. For example, when it is taken by a locust or a
	 * similarly moving piece
	 */
	e[ip]= vide;

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
		if (anyclone && id != rn && id != rb) {
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
		    cr= (*circerenai)(pj,
			    spec[id], ip, id, pp > vide ? blanc : noir);
		}
		else {
		    cr= (*circerenai)(pren,
			    spec[ip], ip, id, pp > vide ? noir : blanc);
		}

		if ((pc= e[cr]) == vide) {		/* V2.90  NG */
		    e[cr]= pren;
		}
	    } /* anycirce && abs(pp) > roib */
	} /* pp != vide && pp != obs */			/* V3.35  NG */
	/*****	TLi  V3.31  *****  end	*****/
	else { /* no capture move */			/* V3.35  NG */
	    if (abs(pj) == King && castling_supported) {
		if (ip == maxsquare+2) {	/* 0-0 *//* V3.63  NG */
		    flag_castling= 1;
		    e[ia - 1]= e[ia + 1];
		    e[ia + 1]= vide;
		}
		else if (ip == maxsquare+3) { /* 0-0-0 *//* V3.63  NG */
		    flag_castling= 2;
		    e[ia + 1]= e[ia - 2];
		    e[ia - 2]= vide;
		}
	    }
	}
    }

    if (CondFlag[contactgrid]) {			/* V2.4c  NG */
	Result= nogridcontact(ia);			/* V2.5c  NG */
    }
    else {
	for (k= 8; k > 0; k--) {
	    if ( (p= e[ia + vec[k]]) != obs
	      && p != vide)				 /* 3.02 TLi */
	    {
		/* this is faster than a call to abs() */
		Result = false;
		break;
	    }
	}
    }

    if (pc != obs) {					/* V3.02  TLi,
							   V3.05  NG */
	e[cr]= pc;
    }

    e[ip]= pp;						/* V3.02  TLi */
    e[id]= pj;
    if (flag_castling == 1) {				/* V3.35  NG */
	e[ia + 1]= e[ia - 1];
	e[ia - 1]= vide;
    }
    else if (flag_castling == 2) {
	e[ia - 2]= e[ia + 1];
	e[ia + 1]= vide;
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
} /* ResetPosition */

/*****	V3.12  TLi  *****  end	*****/

boolean ooorphancheck(square i, piece porph, piece p, boolean (* evaluate)(square,square,square)) {
    boolean	flag= false;
    square	olist[63], *bnp;
    int		k, j, nrp, co;

    if ((*checkfunctions[abs(porph)])(i, porph, evaluate))
	return true;

    if ((nrp= nbpiece[p]) == 0)
	return false;

    nbpiece[-p]--;
    e[i]= dummyb;
    co= 0;
    for (bnp= boardnum; co < nrp; bnp++) {
	if (e[*bnp] == p) {
	    olist[co++]= *bnp;
	}
    }
    for (k= 0; k < co; k++) {
	j= 0;
	while (j < co) {
	    e[olist[j]]= (k == j) ? p : dummyb;
	    j++;
	}
	if ((*checkfunctions[abs(porph)])(i, p, evaluate)) {
	    for (j= 0; j < co; e[olist[j++]]= p)
		;
	    flag= ooorphancheck(olist[k], -porph, -p, evaluate);
	    if (flag)
		break;
	}
	else {
	    for (j= 0; j < co; e[olist[j++]]= p)
		;
	} 

    }

    nbpiece[-p]++;
    e[i]= -p;
    return flag;
}

boolean orphancheck(
    square	 i,
    piece	 p,
    boolean	 (*evaluate)(square,square,square))
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
		if ((*checkfunctions[*porph])(i, p, evaluate)) {
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

boolean fffriendcheck(
    square	i,
    piece	pfr,
    piece	p,
    boolean	(*evaluate)(square,square,square))	/* V3.65  TLi */
{
    boolean	flag= false;
    square	flist[63], *bnp;
    int		k, j, nrp, cf= 0;

    if ((*checkfunctions[abs(pfr)])(i, pfr, evaluate))
       return true;

    if ((nrp= nbpiece[p]-1) == 0)
       return false;

    nbpiece[p]--;
    e[i]= dummyb;
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
	if ((*checkfunctions[abs(pfr)])(i, p, evaluate)) {
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
    e[i]= p;
    return flag;
} /* fffriendcheck */

boolean friendcheck(
    square	i, piece p,
    boolean	(*evaluate)(square,square,square))	/* V3.65  TLi */
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
