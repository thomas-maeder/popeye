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
 ** 2007/06/01 SE   New piece: Radial knight (invented: C.J.Feather)
 **
 ** 2007/11/08 SE   New conditions: Vaulting kings (invented: J.G.Ingram)
 **                 Transmuting/Reflecting Ks now take optional piece list
 **                 turning them into vaulting types
 **
 ** 2007/12/26 SE   New condition: Protean Chess
 **
 ** 2008/01/11 SE   New variant: Special Grids 
 **
 ** 2008/01/24 SE   New variant: Gridlines  
 **
 ** 2008/02/24 SE   Bugfix: Koeko + Parrain  
 **
 ** 2008/02/19 SE   New condition: AntiKoeko  
 **
 ** 2008/02/25 SE   New piece type: Magic  
 **
 **************************** End of List ******************************/

#if defined(macintosh)	      /* is always defined on macintosh's  SB */
#	define SEGM1
#	include "pymac.h"
#endif
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

  ep2[nbply]=
    ep[nbply]= initsquare;

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
    cir_prom[nbply]= vide;
  norm_cham_prom[nbply]=
    cir_cham_prom[nbply]=
    Iprom[nbply] = false;

  /*
    Supercirce rebirths are implemented similarly to promotions ...
  */
  super[nbply]= superbas;

  /*
    start with the castling rights of the upper level
  */
  castling_flag[nbply]= castling_flag[nbply - 1];
  WhiteStrictSAT[nbply]= WhiteStrictSAT[nbply-1];
  BlackStrictSAT[nbply]= BlackStrictSAT[nbply-1];
}

#if defined(NODEF)
void nextply(void)
{
  debut = nbcou;
  nbply++;
  repere[nbply]= nbcou;
  norm_prom[nbply]=
    cir_prom[nbply]= vide;
  ep2[nbply]=
    ep[nbply]= initsquare;
  norm_cham_prom[nbply]=				
    cir_cham_prom[nbply]= false;
  super[nbply]= superbas;
  castling_flag[nbply]= castling_flag[nbply - 1];
  WhiteStrictSAT[nbply]= WhiteStrictSAT[nbply-1];
  BlackStrictSAT[nbply]= BlackStrictSAT[nbply-1];
}
#endif  /* NODEF */

void InitCond(void) {
  square *bnp, i, j;
  piece p;

  flag_madrasi= false;
  wh_exact= wh_ultra= bl_exact= bl_ultra= false;
  anyclone= anycirprom= anycirce= anyimmun= anyanticirce= anytraitor= false;
  anymars= anyantimars= anygeneva= false;
  blacknull= whitenull= nullgenre= false;

  immrenroib= immrenroin= cirrenroib= cirrenroin= initsquare;

  antirenai= rennormal;
  circerenai= rennormal;
  immunrenai= rennormal;
  marsrenai= rennormal;
  genevarenai= rennormal;

  bl_royal_sq= wh_royal_sq= initsquare;

  flagAssassin= false;
  flagwhitemummer= flagblackmummer= false;
  flagmaxi= flagultraschachzwang= false;
  flagparasent= false;
  rex_mad= rex_circe= rex_immun= rex_phan= rex_geneva=
    rex_mess_ex= rex_wooz_ex= false;
  rex_protean_ex = false;
  calctransmute= false;

  for (p= vide; p < PieceCount; p++)
	NextChamCircePiece[p]= p;
  InitChamCirce= True;

  max_pn= max_pb= 8;
  max_pt=16;
  sentinelb= pb;
  sentineln= pn;

  ClrDiaRen(PieSpExFlags);

  gridvar = grid_normal;
  numgridlines = 0;
  for (bnp= boardnum; *bnp; bnp++) {
	int const file= *bnp%onerow - nr_of_slack_files_left_of_board;
	int const row= *bnp/onerow - nr_of_slack_rows_below_board;

	CLEARFL(sq_spec[*bnp]);
	ClrDiaRen(spec[*bnp]);
    sq_num[*bnp]= (int)(bnp-boardnum);

	/* initialise sq_spec and set grid number */
	sq_spec[*bnp] += ((file/2)+4*(row/2)) << Grid;
	if (file!=0 && file!=nr_files_on_board-1
        && row!=0 && row!=nr_rows_on_board-1)
      SETFLAG(sq_spec[*bnp], NoEdgeSq);
  }

  for (i= bas; i < haut; i+= onerow) {
	if (i > bas)
      if (!TSTFLAG(sq_spec[i+dir_down], SqColor))
		SETFLAG(sq_spec[i], SqColor);
    for (j= i+1; j < i+nr_files_on_board; j++)
      if (!TSTFLAG(sq_spec[j+dir_left], SqColor))
		SETFLAG(sq_spec[j], SqColor);
  }

  for (i= 0; i < CondCount; CondFlag[i++]= False) {
	;
  }

  for (i= maxply; i > 0; inum[i--]= 0)
	;

  memset((char *) promonly, 0, sizeof(promonly));
  memset((char *) isapril,0,sizeof(isapril));
  checkhopim = false;
  koekofunc= nokingcontact;
  antikoekofunc= nokingcontact;
  OscillatingKingsTypeB[blanc]= false;
  OscillatingKingsTypeB[noir]= false;
  OscillatingKingsTypeC[blanc]= false;
  OscillatingKingsTypeC[noir]= false;

  black_length = NULL;
  white_length = NULL;

  BGL_white= BGL_black= BGL_infinity;
  BGL_whiteinfinity= BGL_blackinfinity= true;
  BGL_global= false;

  calc_whtrans_king= calc_bltrans_king= 
  calc_whrefl_king= calc_blrefl_king= false;
  whitenormaltranspieces = blacknormaltranspieces = true;
} /* InitCond */

void InitOpt(void) {
  int i;

  maxflights= 64;
  maxdefen= 0;
  MoveNbr= RestartNbr= 0;

  droh= NonTrivialLength= maxply;
  ep2[nbply]= ep[nbply]= initsquare;
  no_castling= bl_castlings|wh_castlings;

  for (i= 0; i < OptCount; OptFlag[i++]= False);
  flag_appseul= false;
}

void InitCheckDir(void) {
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

void InitBoard(void) {
  square i, *bnp;

  ActTitle[0]= ActAuthor[0]= ActOrigin[0]=
    ActTwin[0]= ActAward[0] = ActStip[0]= '\0';

  for (i= maxsquare - 1; i >= 0; i--) {
    e[i]= e_ubi[i]= e_ubi_mad[i]= obs;
    spec[i]= BorderSpec;
  }

  /* dummy squares for Messigny chess and castling -- must be empty */
  e[messigny_exchange]= e[kingside_castling]= e[queenside_castling]= vide;
  CLEARFL(spec[messigny_exchange]);
  CLEARFL(spec[kingside_castling]);
  CLEARFL(spec[queenside_castling]);

  for (bnp= boardnum; *bnp; bnp++)
	e[*bnp]= vide;

  rb= rn= initsquare;

  CLEARFL(PieSpExFlags);

  nrhuntertypes = 0;
} /* InitBoard */

void InitStip(void) {
  FlagGenMatingMove= FlagMoveOrientatedStip= False;
  enonce= 0;
}

void InitAlways(void) {
  square i;

  memset((char *) exist, 0, sizeof(exist));

  move_generation_mode= move_generation_optimized_by_killer_move;

  nbcou = 1;
  nbply = 1;

  nbmagic = 0;

  flagAssassin=
    flag_writinglinesolution=
    flag_testlegality= k_cap=
    flende =
    flagfee =
    flagriders	    =
    flagleapers     =
    flagsimplehoppers     =
    flagsimpledecomposedleapers =
    flagsymmetricfairy  =
    flagveryfairy   = false;

  marge = 0;
  solutions= 0;

  PrintReciSolution = False;

  for (i= maxply; i > 0; i--) {
	whduell[i]= blduell[i]=
      kpilcd[i]= kpilca[i]=
      sqrenais[i]= initsquare;
	trait[i]= blanc;
	crenkam[i]= initsquare;
	pwcprom[i]=
      senti[i]=
      Iprom[i]= false;
	att_1[i]= true;
    oscillatedKs[i]= false;
    colour_change_sp[i]= colour_change_stack;
  }

  initneutre(blanc);
  tabsol.nbr=
    tabsol.cp[0]= 0;
  tabsol.liste[0].push_top= push_colour_change_stack;
  flag_atob= false;
  dont_generate_castling=false;
  
  takemake_departuresquare= initsquare;
  takemake_capturesquare= initsquare;
} /* InitAlways */

void initneutre(couleur c) {
  /* I don't know why, but the solution below is not slower */
  /* than the double loop solution of genblanc(). NG */ 

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
  return 75 + (onerow*(((i/onerow)+3)/2) + (((i%onerow)+3)/2));
}

boolean leapcheck(square	 sq_king,
                  numvec	 kanf,
                  numvec	 kend,
                  piece	 p,
                  evalfunction_t *evaluate)
{
  /* detect "check" of leaper p */
  numvec  k;

  square sq_departure;

  for (k= kanf; k<=kend; k++) {
	sq_departure= sq_king+vec[k];
	if (e[sq_departure]==p
        && evaluate(sq_departure,sq_king,sq_king)
        && imcheck(sq_departure,sq_king))
      return true;
  }
  
  return false;
}

boolean leapleapcheck(
  square	 sq_king,
  numvec	 kanf,
  numvec	 kend,
  int hurdletype,
  piece	 p,
  evalfunction_t *evaluate)
{
  /* detect "check" of leaper p */
  numvec  k, k1;
  square  sq_departure, sq_hurdle;

  for (k= kanf; k<= kend; k++) {
    sq_hurdle= sq_king + vec[k];
    if (hurdletype==0 && abs(e[sq_hurdle])>obs && e[sq_hurdle]*p<0)
    {
      for (k1= kanf; k1<= kend; k1++) {
        sq_departure = sq_hurdle + vec[k1];
        if (e[sq_departure]==p && sq_departure!=sq_king
            && (*evaluate)(sq_departure,sq_king,sq_king)
            && imcheck(sq_departure,sq_king))
        {
          return true;
        }
      }
    }
  }

  return false;
}

boolean riderhoppercheck(square	 sq_king,
                         numvec	 kanf,
                         numvec	 kend,
                         piece	 p,
                         int	 run_up,
                         int	 jump,
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
		int jumped= jump;
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
          int ran_up= run_up;
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

boolean ridcheck(square	sq_king,
                 numvec	kanf,
                 numvec	kend,
                 piece	p,
                 evalfunction_t	*evaluate)
{
  /* detect "check" of rider p */
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
		return true;
	}
  }
  return false;
}

boolean nogridcontact(square j)
{
  square  j1;
  numvec  k;
  piece   p;

  for (k= 8; k > 0; k--) {
	p= e[j1= j + vec[k]];
	if (p != vide && p != obs && GridLegal(j1, j)) {
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
         && p != vide)
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


boolean nocontact(square sq_departure, square sq_arrival, square sq_capture, nocontactfunc_t nocontactfunc) {
  boolean	Result;
  square	cr;
  piece	pj, pp, pren;
  piece	pc= obs;
  boolean	flag_castling= false;
  square sq_castle_from, sq_castle_to;

  VARIABLE_INIT(cr);

  pj= e[sq_departure];
  pp= e[sq_capture];
  /* does this work with neutral pieces ??? */
  if (CondFlag[haanerchess]) {
	e[sq_departure]= obs;
  }
  else if (CondFlag[sentinelles]
           && sq_departure>=square_a2 && sq_departure<=square_h7
           && !is_pawn(pj))
  {
	if ((pj<=roin) != SentPionAdverse) {
      if (nbpiece[sentineln] < max_pn
	      && nbpiece[sentinelb]+nbpiece[sentineln] < max_pt
	      && (!flagparasent
              || (nbpiece[sentineln]
                  <= nbpiece[sentinelb]+(pp==sentinelb?1:0))))
      {
		e[sq_departure]= sentineln;
      }
      else {
		e[sq_departure]= vide;
      }
	}
	else { /* we assume  pj >= roib */
      if (nbpiece[sentinelb] < max_pb
	      && nbpiece[sentinelb]+nbpiece[sentineln] < max_pt
	      && (!flagparasent
              || (nbpiece[sentinelb]
                  <= nbpiece[sentineln]+(pp==sentineln?1:0))))
      {
        e[sq_departure]= sentinelb;
      }
      else {
		e[sq_departure]= vide;
      }
      /* don't think any change as a result of Sentinelles */
      /* PionNeutral is needed as piece specs not changed  */
	}
  }
  else {
	e[sq_departure]= vide;
	/* e[sq_departure] = CondFlag[haanerchess] ? obs : vide;	   */
  }

  if (sq_capture == messigny_exchange) {
	e[sq_departure]= e[sq_arrival];
  }
  else {
	/* the pieces captured and reborn may be different: */
	/* Clone, Chameleon Circe			    */
    pp= e[sq_capture];

	/* the pieces can be reborn at the square where it has been
	 * captured. For example, when it is taken by a locust or a
	 * similarly moving piece
	 */
	e[sq_capture]= vide;

  if (CondFlag[parrain] && pprise[nbply-1] != vide) {
    cr= move_generation_stack[repere[nbply]].capture + sq_arrival - sq_departure;
		if ((pc= e[cr]) == vide) {
          e[cr]= pprise[nbply-1];
		}
  }

	if (pp != vide && pp != obs) {
      if (anycirce && abs(pp) > roib && !CondFlag[parrain]) {
		/* This still doesn't work with neutral pieces.
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
            : pp;
		}

		if (CondFlag[couscous]) {
          cr= (*circerenai)(pj, spec[sq_departure], sq_capture, sq_departure, sq_arrival, pp > vide ? blanc : noir);
		}
		else {
          cr= (*circerenai)(pren, spec[sq_capture], sq_capture, sq_departure, sq_arrival, pp > vide ? noir : blanc);
		}

		if ((pc= e[cr]) == vide) {
          e[cr]= pren;
		}
      } /* anycirce && abs(pp) > roib */
	} /* pp != vide && pp != obs */
	else { /* no capture move */
      if (abs(pj) == King)
      {
        if (castling_supported) {
		      if (sq_capture == kingside_castling) {
            flag_castling= true;
            sq_castle_from = sq_arrival+dir_right;
            sq_castle_to = sq_arrival+dir_left;
		      }
		      else if (sq_capture == queenside_castling) {
            sq_castle_from = sq_arrival+2*dir_left;
            sq_castle_to = sq_arrival+dir_right;
            flag_castling= true;
		      }
        }
        else if (CondFlag[castlingchess] && sq_capture > maxsquare + bas)
        {
          sq_castle_to = (sq_arrival + sq_departure) / 2;
          sq_castle_from = sq_capture - maxsquare;
          flag_castling= true;
        }
        if (flag_castling)
        {
          e[sq_castle_to]= e[sq_castle_from];
          e[sq_castle_from]= vide;
        }
      }
    }
  }

  if (CondFlag[contactgrid]) {
	Result= nogridcontact(sq_arrival);
  }
  else {
	Result= (*nocontactfunc)(sq_arrival);
  }

  if (pc != obs) {
	e[cr]= pc;
  }

  e[sq_capture]= pp;
  e[sq_departure]= pj;
  if (flag_castling) {
	  e[sq_castle_from]= e[sq_castle_to];
    e[sq_castle_to] = vide;
  }

  return Result;
} /* nocontact */

/* new versions of StorePosition() and ResetPosition() */

Flags		sic_spec[64];
piece		sic_e[64];
int	sic_inum1;
imarr		sic_isquare;
square		sic_im0, rn_sic, rb_sic;
long int sic_BGL_W, sic_BGL_b;

void StorePosition(void) {
  int	    i;

  rn_sic= rn; rb_sic= rb;
  for (i= 0; i < 64; i++) {
	sic_e[i]= e[boardnum[i]];
	sic_spec[i]= spec[boardnum[i]];
  }

  /* imitators */
  sic_inum1= inum[1];
  for (i= 0; i < maxinum; i++) {
	sic_isquare[i]= isquare[i];
  }

  sic_im0= im0;
  sic_BGL_W= BGL_white;
  sic_BGL_b= BGL_black;
}

void ResetPosition(void) {
  int	    i;

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

  for (i= 0; i < maxinum; i++)
    isquare[i]= sic_isquare[i];
  im0= sic_im0;
  neutcoul= blanc;
  BGL_white= sic_BGL_W;
  BGL_black= sic_BGL_b;
}

boolean ooorphancheck(square sq_king,
                      piece porph,
                      piece p,
                      evalfunction_t *evaluate) {
  boolean	flag= false;
  square	olist[63], *bnp;
  unsigned int j, k, nrp, co;

  if ((*checkfunctions[abs(porph)])(sq_king,porph,evaluate))
	return true;

  nrp= nbpiece[p];
  if (nrp == 0)
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
  int	k, j, co= 0;

  for (porph= orphanpieces; *porph; porph++) {
	if (nbpiece[*porph]>0 || nbpiece[-*porph]>0) {
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
}

boolean fffriendcheck(square	sq_king,
                      piece	pfr,
                      piece	p,
                      evalfunction_t *evaluate)
{
  boolean	flag= false;
  square	flist[63], *bnp;
  unsigned int j, k, nrp, cf= 0;

  if ((*checkfunctions[abs(pfr)])(sq_king, pfr, evaluate))
    return true;

  nrp= nbpiece[p]-1;
  if (nrp == 0)
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
  evalfunction_t *evaluate)
{
  piece	*pfr, cfr;
  boolean	flag= false;
  boolean	initialized= false;
  square	flist[63], *bnp;
  int	k, j, cf= 0;

  for (pfr= orphanpieces; *pfr; pfr++) {
	cfr= p == friendb ? *pfr : -*pfr;
	if (nbpiece[cfr]>0) {
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

boolean CrossesGridLines(square dep, square arr)
{
  int i, x1, y1, x2, y2, X1, Y1, X2, Y2, dx, dy, dX, dY, u1, u2, v;

  X1= ((dep<<1) -15) % 24;
  Y1= ((dep/24)<<1) - 15;
  X2= ((arr<<1) -15) % 24;
  Y2= ((arr/24)<<1) - 15;
  dX= X2-X1;
  dY= Y2-Y1;
  for (i= 0; i < numgridlines; i++)
  {
    x1= gridlines[i][0];
    y1= gridlines[i][1];
    x2= gridlines[i][2];
    y2= gridlines[i][3];
    dx= x2-x1;
    dy= y2-y1;
    v=dY*dx-dX*dy;
    if (!v)
      continue;
    u1= dX*(y1-Y1)-dY*(x1-X1);
    if (v<0? (u1>0 || u1<v) : (u1<0 || u1>v))
      continue;
    u2= dx*(y1-Y1)-dy*(x1-X1);
    if (v<0? (u2>0 || u2<v) : (u2<0 || u2>v))
      continue;
    return true;
  }
  return false;
}

void GetRoseAttackVectors(square from, square to)
{
  numvec  k;
  for (k= vec_knight_start; k<=vec_knight_end; k++) {
    if (detect_rosecheck_on_line(to,e[from],
                                 k,0,+1,
                                 eval_fromspecificsquare))
      PushMagic(to, DiaRen(spec[to]), DiaRen(spec[from]), 200+vec[k] )
    if (detect_rosecheck_on_line(to,e[from],
                                 k,vec_knight_end-vec_knight_start+1,-1,
                                 eval_fromspecificsquare))
      PushMagic(to, DiaRen(spec[to]), DiaRen(spec[from]), 300+vec[k])
  }
}

void GetRoseLionAttackVectors(square from, square to)
{
  numvec  k;
  for (k= vec_knight_start; k <= vec_knight_end; k++) {
    if (detect_roselioncheck_on_line(to,e[from],
                                     k,0,+1,
                                     eval_fromspecificsquare))
      PushMagic(to, DiaRen(spec[to]), DiaRen(spec[from]), 200+vec[k] )
    if (detect_roselioncheck_on_line(to,e[from],
                                        k,vec_knight_end-vec_knight_start+1,-1,
                                        eval_fromspecificsquare))
      PushMagic(to, DiaRen(spec[to]), DiaRen(spec[from]), 300+vec[k])
  }
}

void GetRoseHopperAttackVectors(square from, square to) {
  numvec  k;
  square sq_hurdle;

  for (k= vec_knight_start; k <= vec_knight_end; k++) {
    sq_hurdle= to+vec[k];
    if (e[sq_hurdle]!=vide && e[sq_hurdle]!=obs) {
        /* k1==0 (and the equivalent
         * vec_knight_end-vec_knight_start+1) were already used for
         * sq_hurdle! */
      if (detect_rosehoppercheck_on_line(to,sq_hurdle,e[from],
                                         k,1,+1,
                                         eval_fromspecificsquare))
        PushMagic(to, DiaRen(spec[to]), DiaRen(spec[from]), 200+vec[k] );
      if (detect_rosehoppercheck_on_line(to,sq_hurdle,e[from],
                                         k,vec_knight_end-vec_knight_start,-1,
                                         eval_fromspecificsquare))
        PushMagic(to, DiaRen(spec[to]), DiaRen(spec[from]), 300+vec[k]);
    }
  }
}

void GetRoseLocustAttackVectors(square from, square to) {
  /* detects check by a rose locust */
  numvec  k;
  square sq_arrival;

  for (k= vec_knight_start; k <= vec_knight_end; k++) {
    sq_arrival= to-vec[k];
    if (e[sq_arrival]==vide) {
        /* k1==0 (and the equivalent
         * vec_knight_end-vec_knight_start+1) were already used for
         * sq_hurdle! */
      if (detect_roselocustcheck_on_line(to,sq_arrival,e[from],
                                         k,1,+1,
                                         eval_fromspecificsquare))
        PushMagic(to, DiaRen(spec[to]), DiaRen(spec[from]), 200+vec[k] );
      if (detect_roselocustcheck_on_line(to,sq_arrival,e[from],
                                         k,vec_knight_end-vec_knight_start,-1,
                                         eval_fromspecificsquare))
        PushMagic(to, DiaRen(spec[to]), DiaRen(spec[from]), 300+vec[k]);
    }
  }
}

void GetRMHopAttackVectors(square from, square to, numvec kend, numvec kanf, angle_t angle) {
  square sq_hurdle;
  numvec k, k1;
  piece hopper;

  square sq_departure;

  for (k= kend; k>=kanf; k--) {
    sq_hurdle= to+vec[k];
    if (abs(e[sq_hurdle])>=roib) {
      k1= 2*k;
      finligne(sq_hurdle,mixhopdata[angle][k1],hopper,sq_departure);
      if (hopper==e[from]) {
        if (eval_fromspecificsquare(sq_departure,to,to))
          PushMagic(to, DiaRen(spec[to]), DiaRen(spec[from]), vec[k] )
      }
      finligne(sq_hurdle,mixhopdata[angle][k1-1],hopper,sq_departure);
      if (hopper==e[from]) {
        if (eval_fromspecificsquare(sq_departure,to,to))
          PushMagic(to, DiaRen(spec[to]), DiaRen(spec[from]), vec[k] )
      }
    }
  }
}

void GetMooseAttackVectors(square from, square to) {
  GetRMHopAttackVectors(from, to, vec_queen_end, vec_queen_start, angle_45);
}

void GetRookMooseAttackVectors(square from, square to) {
  GetRMHopAttackVectors(from, to, vec_rook_end, vec_rook_start, angle_45);
}

void GetBishopMooseAttackVectors(square from, square to) {
  GetRMHopAttackVectors(from, to, vec_bishop_end, vec_bishop_start, angle_45);
}

void GetEagleAttackVectors(square from, square to) {
  GetRMHopAttackVectors(from, to, vec_queen_end, vec_queen_start, angle_90);
}

void GetRookEagleAttackVectors(square from, square to) {
  GetRMHopAttackVectors(from, to, vec_rook_end, vec_rook_start, angle_90);
}

void GetBishopEagleAttackVectors(square from, square to) {
  GetRMHopAttackVectors(from, to, vec_bishop_end, vec_bishop_start, angle_90);
}

void GetSparrowAttackVectors(square from, square to) {
  GetRMHopAttackVectors(from, to, vec_queen_end, vec_queen_start, angle_135);
}

void GetRookSparrowAttackVectors(square from, square to) {
  GetRMHopAttackVectors(from, to, vec_rook_end, vec_rook_start, angle_135);
}

void GetBishopSparrowAttackVectors(square from, square to) {
  GetRMHopAttackVectors(from, to, vec_bishop_end, vec_bishop_start, angle_135);
}

void GetMargueriteAttackVectors(square from, square to) {
  GetRMHopAttackVectors(from, to, vec_queen_end, vec_queen_start, angle_45);
  GetRMHopAttackVectors(from, to, vec_queen_end, vec_queen_start, angle_90);
  GetRMHopAttackVectors(from, to, vec_queen_end, vec_queen_start, angle_135);
  if (scheck(to, e[from], eval_fromspecificsquare)) {
    numvec attackVec;
    if (to < from)
      attackVec = move_vec_code[from - to];
    else
      attackVec = -move_vec_code[to - from];
    if (attackVec)
      PushMagic(to, DiaRen(spec[to]), DiaRen(spec[from]), attackVec)      
  }
}

void GetZigZagAttackVectors(square from, square to,
               numvec  k,
               numvec  k1)
{
  square sq_departure= to+k;
  square sq_arrival= to;
  square sq_capture= to;

  while (e[sq_departure] == vide) {
    sq_departure+= k1;
    if (e[sq_departure] != vide)
      break;
    else
      sq_departure+= k;
  }

  if (e[sq_departure]==e[from]
      && eval_fromspecificsquare(sq_departure,sq_arrival,sq_capture))
    PushMagic(to, DiaRen(spec[to]), DiaRen(spec[from]), vec[500+k] );

  sq_departure = to+k;
  while (e[sq_departure]==vide) {
    sq_departure-= k1;
    if (e[sq_departure]!=vide)
      break;
    else
      sq_departure+= k;
  }

  if (e[sq_departure]==e[from]
      && eval_fromspecificsquare(sq_departure,sq_arrival,sq_capture))
    PushMagic(to, DiaRen(spec[to]), DiaRen(spec[from]), vec[400+k] );
}

void GetBoyscoutAttackVectors(square from, square to) {
  numvec  k;

  for (k= vec_bishop_start; k <= vec_bishop_end; k++) {
    GetZigZagAttackVectors(from, to, vec[k], vec[13 - k]);
  }
}

void GetGirlscoutAttackVectors(square from, square to) {
  numvec  k;

  for (k= vec_rook_start; k <= vec_rook_end; k++) {
    GetZigZagAttackVectors(from, to, vec[k], vec[5 - k]);
  }
}

void GetSpiralSpringerAttackVectors(square from, square to) {
  numvec  k;

  for (k= vec_knight_start; k <= vec_knight_end; k++) {
    GetZigZagAttackVectors(from, to, vec[k], vec[25 - k]);
  }
}

void GetDiagonalSpiralSpringerAttackVectors(square from, square to) {
  numvec  k;

  for (k= vec_knight_start; k <= 14; k++) {
    GetZigZagAttackVectors(from, to, vec[k], vec[23 - k]); 
  }
  for (k= 15; k <= vec_knight_end; k++) {
    GetZigZagAttackVectors(from, to, vec[k], vec[27 - k]); 
  }
}

/* should never get called if validation works
(disallow magic + piecetype) */
void __unsupported_uncalled_attackfunction(square from, square to) {}

void PushMagicViews(void)
{
  square *bnp, *bnp1, *royal, royal_save;
  piece p;
  
  if (!flag_magic)
    return;
    
  /*new stack */
  nbmagic = magictop[nbply - 1];
    
  for (bnp= boardnum; *bnp; bnp++) 
  {
    if (TSTFLAG(spec[*bnp], Magic))
    {
      /* for each magic piece */
      p= e[*bnp];
      royal = (p <= roin) ? &rb : &rn;
      royal_save= *royal;
      fromspecificsquare= *bnp;
      for (bnp1= boardnum; *bnp1; bnp1++) 
      {
        if (abs(e[*bnp1]) > obs 
              && !TSTFLAG(spec[*bnp1], Magic)
              && !TSTFLAG(spec[*bnp1], Royal)) {

          /* for each non-magic piece 
          (n.b. check *bnp != *bnp1 redundant above) */
          *royal= *bnp1;

          
          if (!attackfunctions[abs(p)])
          {
            /* if single attack at most */
            if ((*checkfunctions[abs(p)])(*royal, p, eval_fromspecificsquare))
            {
                numvec attackVec;
                if (*royal < *bnp)
                  attackVec = move_vec_code[*bnp - *royal];
                else
                  attackVec = -move_vec_code[*royal - *bnp];
                if (attackVec)
                  PushMagic(*royal, DiaRen(spec[*royal]), DiaRen(spec[fromspecificsquare]), attackVec)
            }
          }
          else
          {
            /* call special function to determine all attacks */
            (*attackfunctions[abs(p)])(fromspecificsquare, *royal);
          }
        }
      }
      *royal= royal_save;  
    }
  }   
  magictop[nbply] = nbmagic;
}

void ChangeMagic(int ply, boolean push)
{
  square *bnp;
  int i, j, k;
  boolean dochange, newvec;

  for (bnp= boardnum; *bnp; bnp++) 
  {
    for (i= magictop[ply - 1]; i < magictop[ply]; i++)
    {
      if (magicviews[i].piecesquare == *bnp)  
        break;    /* a magic piece observes a non-magic */
    }
    if (i < magictop[ply])
    {
      dochange= false;

      /* now check the rest of the nbply-stack for other attacks of same piece */
      for (j= i; j <  magictop[ply]; j++)
      {
        if (magicviews[j].piecesquare == *bnp)
        {
          int currid= magicviews[j].pieceid;
          int currmagid= magicviews[j].magicpieceid;
          numvec currvec= magicviews[j].vecnum;
          newvec= true;

          /* and check (nbply-1)-stack to see if this is a new attack */
          for (k= magictop[ply - 2]; k < magictop[ply - 1]; k++)
          {
            if (magicviews[k].pieceid == currid &&
                magicviews[k].magicpieceid == currmagid &&
                magicviews[k].vecnum == currvec)
            {
               newvec= false;
               break;
            }
          }
          /* only changes if attackee suffers odd-no. new attacks */
          if (newvec)
            dochange = dochange ^ true; 
        }
      }
      if (dochange)
      {
        ChangeColour(*bnp);
        /* don't store colour change of moving piece - it might undergo other changes */
        if (push && *bnp != move_generation_stack[nbcou].arrival)
          PushChangedColour(colour_change_sp[nbply], colour_change_stack_limit, *bnp, e[*bnp]);
      }
    }
  }               
}

#ifdef DEBUG
WriteMagicViews(int ply)
{
  int i;
  for (i= magictop[ply - 1]; i < magictop[ply]; i++)
  {
    char buf[10];
    WriteSquare(magicviews[i].piecesquare);
    StdChar(' ');
    WriteSquare(magicviews[i].pieceid);
    StdChar(' ');
    WriteSquare(magicviews[i].magicpieceid);
    StdChar(' ');
    sprintf(buf, "%i", magicviews[i].vecnum);
    StdString(buf);
    StdChar('\n');           
  }
}
#endif

void ChangeColour(square sq)
{
  change(sq);
  CHANGECOLOR(spec[sq]);
  if (e[sq] == tb && sq == square_a1)
    SETFLAGMASK(castling_flag[nbply],ra1_cancastle);  
  if (e[sq] == tb && sq == square_h1)
    SETFLAGMASK(castling_flag[nbply],rh1_cancastle);  
  if (e[sq] == tn && sq == square_a8)
    SETFLAGMASK(castling_flag[nbply],ra8_cancastle);  
  if (e[sq] == tn && sq == square_h8)
    SETFLAGMASK(castling_flag[nbply],rh8_cancastle); 
}
