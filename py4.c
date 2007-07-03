/********************* MODIFICATIONS to py4.c **************************
 **
 ** Date       Who  What
 **
 ** 2004/01/28 SE   New pieces Equi (English & French styles) (invented P.Harris)
 **
 ** 2004/02/05 SE   New piece Querquisite (J.E.H.Creed FCR 1947) 
 **					moves as piece upon whose file it stands
 **					Also re-invented as 'Odysseus' by H.Schmid f.? c.1988
 **
 ** 2004/02/07 SE   New condition: Antikings (?inventor) in check when not attacked
 **
 ** 2004/03/05 SE   New condition : Antimars (and variants) (invented S.Emmerson)
 **                 Pieces reborn to move, capture normally
 **
 ** 2004/03/19 SE   New condition: Supertransmutingkings (?inventor)
 **
 ** 2004/03/22 SE   New condition: Ultrapatrol (only patrolled pieces can move or capture)
 **
 ** 2004/04/22 SE   Castling with Imitators
 **
 ** 2004/05/02 SE   Imitators with mao, moa
 **
 ** 2005/04/19 NG   genhunt bugfix
 **
 ** 2006/05/09 SE   New conditions: SAT, StrictSAT, SAT X Y (invented L.Salai sr.)
 **
 ** 2006/05/09 SE   New pieces Bouncer, Rookbouncer, Bishopbouncer (invented P.Wong)
 **
 ** 2006/05/14 SE   New Condition: TakeMake (invented H.Laue)
 **
 ** 2006/05/17 SE   Bug fix: querquisite
 **                 P moves to 1st rank disallowed for Take&Make on request of inventor
 **
 **************************** End of List ******************************/

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
#include <stdlib.h>		/* H.D. 10.02.93 prototype fuer qsort */
#include <string.h>
#include "py.h"
#include "py1.h"
#include "pyproc.h"
#include "pydata.h"
#include "pymsg.h"

#define MAX_OTHER_LEN 1000 /* needs to be at least the max of any value that can be returned in the len functions */

short len_max(square id, square ia, square ip)
{
  /* rewritten  V3.35  TLi */
  switch (ip) {     					/* V3.55  TLi */

  case maxsquare+1:
	/* Messigny chess type exchange --> no move, no length */
	return 0;

  case maxsquare+2:  /* short castling */
	return 16;

  case maxsquare+3:  /* long castling */
	return 25;

  default:	/* ``ordinary move'' */
	switch (abs(e[id])) {

    case Mao:    /* special MAO move.*/ 		/* V2.90  NG */
      return 6;

    case Moa:    /* special MOA move.*/ 		/* V3.0  TLi */
      return 6;

    default:
      return (move_diff_code[abs(ia - id)]);      /* V2.90  NG */
	}
  }
}

short len_min(square id, square ia, square ip) {
  /* rewritten  V3.35  TLi */
  return -len_max(id, ia, ip);
}

short len_capt(square id, square ia, square ip) {
  return (e[ip] != vide);
}

short len_follow(square id, square ia, square ip) {
  return (ia == cd[repere[nbply]]);
}

short len_whduell(square id, square ia, square ip) {
  return (id == whduell[nbply - 1]);
}

short len_blduell(square id, square ia, square ip) {
  return (id == blduell[nbply - 1]);
}

short len_alphabetic(square id, square ia, square ip) { /* V3.64  SE */
  return -((id/24) + 24*(id%24));
}


/*****	V3.20  TLi  *****  begin  *****/
short len_whforcedsquare(square id, square ia, square ip) {
  if (we_generate_exact) {
	if (TSTFLAG(sq_spec[ia], WhConsForcedSq)) {
      there_are_consmoves = true;
      return 1;
	}
	else {
      return 0;
	}
  }
  else {
	return (TSTFLAG(sq_spec[ia], WhForcedSq));
  }
}

short len_blforcedsquare(square id, square ia, square ip) {
  if (we_generate_exact) {
	if (TSTFLAG(sq_spec[ia], BlConsForcedSq)) {
      there_are_consmoves = true;
      return 1;
	}
	else {
      return 0;
	}
  }
  else {
	return (TSTFLAG(sq_spec[ia], BlForcedSq));
  }
}
/*****	V3.20  TLi  *****   end   *****/

boolean cntoppmoves(int *nbd, couleur camp) {
  boolean	    flag= false;
  numecoup	    sic_nbc= nbcou;

  while (sic_nbc == nbcou) {
	if (jouecoup()) {			    /* V3.44  SE/TLi */
      if (!flag && !echecc(camp)) {
        couleur ad= advers(camp);
        flag= true;
        *nbd= 0;
        optimize= false;
        genmove(ad);
        optimize= true;
        while (encore()) {
          if (jouecoup()) {		    /* V3.44  SE/TLi */
            if (!echecc(ad))
              (*nbd)++;
          }
          repcoup();
        }
        finply();
      }
	}
	repcoup();
  }
  nbcou= sic_nbc;
  return flag;
}

boolean empile(square id, square ia, square ip) {
  square  hcr, mren= initsquare;					/* V1.7c  NG */
  couleur traitnbply;					/* V3.1  TLi */


  if (id == ia) {					/* V2.1c  NG */
	return true;
  }

  if (empilegenre) {					/* V2.51  NG */
	if (   CondFlag[messigny]			/* V3.55  TLi */
           && ip == maxsquare+1
           /* a swapping move */
           && cp[repere[nbply]] == maxsquare+1
           /* last move was a swapping one too */
           && (   id == ca[repere[nbply]] || id == cd[repere[nbply]]
                  || ia == ca[repere[nbply]] || ia == cd[repere[nbply]]))
      /* No single piece must be involved in
       * two consecutive swappings, so reject move.
       */
	{
      return false;
	}

	if (anymars||anyantimars) {				    /* V3.46  SE/TLi */
      if (is_phantomchess) {			/* V3.47  NG */
		if (flagactive) {
          if ((id= marsid) == ia) {
			return true;
          }
		}
      }
      else {
		if ((flagpassive ^ anyantimars) && (e[ip] != vide))
          return true;
		if ((flagcapture ^ anyantimars) && (e[ip] == vide))
          return true;
		if (flagcapture) {
          mren= id;
          id=marsid;
		}
      }
	}

	if (  flaglegalsquare	       /* V3.03  TLi */
          && !legalsquare(id, ia, ip))
	{
      return true;
	}

	traitnbply= trait[nbply];

  if (CondFlag[takemake])   /* V4.03 SE */
  {
     if (generate_pass)
     {
       if (e[ip] != vide)
         return true;
       id= orig_id;
       ip= orig_ip;
       /* oh dear, unfortunate extra rule */
       if (((orig_p==pb || orig_p==pbb) && ia < bas + 24) || ((orig_p==pn || orig_p==pbn) && ia > haut - 24))
         return true;
     }
     else if (e[ip] != vide)
     {
       generate_pass= true;

       orig_ia= ia;  /* don't need, but never know... */
       orig_id= id;
       orig_ip= ip;
       orig_p= e[id];
       orig_spec= spec[id];
       orig_cap_p= e[ip];
       orig_cap_spec= spec[ip];

       e[id]= vide;    /* for sentinelles, need to calculate... */
       spec[id]= EmptySpec;
       e[ia]= orig_cap_p;
       spec[ia]= orig_cap_spec;
       if (ip != ia)
       {
        e[ip]= vide;   /* assuming e[ip] is blank at this point */
        spec[ip]= EmptySpec;
       }

       if (traitnbply == blanc)
         gen_bl_piece_aux(ia, e[ia]);
       else
         gen_wh_piece_aux(ia, e[ia]);

       e[id]= orig_p;
       spec[id]= orig_spec;
       e[ia]= vide;
       spec[ia]= EmptySpec;
       e[ip]= orig_cap_p;
       spec[ip]= orig_cap_spec;

       generate_pass= false;
       return true;
     }
  }


	if (   (   (	CondFlag[nowhiteprom]
                    && traitnbply==blanc
                    && PromSq(blanc,ia)
                 )
               /* V3.64  NG */
               || (   CondFlag[noblackprom]
                      && traitnbply==noir
                      && PromSq(noir,ia)
                 )
             )
           && is_pawn(e[id]) )
	{
      return true;
	}

	if (  TSTFLAG(spec[id], Beamtet)	    /* V3.53 TLi */
          || CondFlag[beamten]    /* V1.4c NG, V3.32	TLi, v3.50 SE */
          || CondFlag[central]
          || CondFlag[ultrapatrouille])
	{
      if (! soutenu(id, ia, ip))
		return true;
	}

	if (e[ip] != vide) {	 /* V2.4c  NG */
      if (CondFlag[woozles]      /* V3.55  TLi */
	      && !woohefflibre(ia, id))
      {
		return	true;
      }

      if (CondFlag[norsk]				/* V3.1  TLi */
	      && (id == rb || id == rn || abs(e[ip]) != abs(e[id])))
      {
		return	true;
      }

      if (CondFlag[nocapture]
	      || (CondFlag[nowhcapture]
              && traitnbply==blanc)			/* V3.64  NG */
	      || (CondFlag[noblcapture]
              && traitnbply==noir)			/* V3.64  NG */
	      || TSTFLAG(spec[id], Paralyse))		/* V1.7c  NG */
      {
		return true;
      }

      if (anyimmun) {		     /* V1.7c  NG , V3.1  TLi */
		hcr= (*immunrenai)(e[ip], spec[ip], ip, id, ia, traitnbply);
		if (hcr != id && e[hcr] != vide) {
          return true;
		}
      }
      if (CondFlag[patrouille]			/* V1.4c  NG */
          && !(CondFlag[beamten]			/* V3.53  TLi */
               || TSTFLAG(PieSpExFlags, Beamtet))) /* V3.32  TLi */
      {
		if (! soutenu(id, ia, ip)) {
          return true;
		}
      }

      if (!k_cap) {		    /* V3.44  SE/TLi */
		/* We have to avoid captures by the rb because he
		 * wouldn't be reborn! This might also be placed in
		 * the function genrb(), but here, it works for all
		 * royal pieces.		  V2.4d  TM
		 * wegen neuer Funktionen genweiss/schwarz aus
		 * gennoir/blanc hierher verschoben	V2.70c	TLi
		 */
		/* capturing kamikaze pieces without circe condition are possible now, V3.74  SE*/ 
		if (TSTFLAG(spec[id], Kamikaze)
		    &&  ((traitnbply == blanc)  /* V3.1  TLi *//* V3.55 SE *//* V3.74  SE,NG */
                 ? ((id == rb) && (!anycirce ||  (!rex_circe || e[(*circerenai)(e[rb], spec[rb], ip, id, ia, noir)] != vide)))
                 : ((id == rn) && (!anycirce ||  (!rex_circe || e[(*circerenai)(e[rn], spec[rn], ip, id, ia, blanc)] != vide)))))
		{
          return true;
		}

		if ((CondFlag[vogt]		  /* V3.03, V3.1  TLi */
             || CondFlag[antikings])        /* V3.78 SE */
		    && ((traitnbply == noir)
                ? ((ip == rb) && (!rex_circe || e[(*circerenai)(e[rb], spec[rb], ip, id, ia, noir)] != vide))     /* V3.55 SE, V3.56 TLi */
                : ((ip == rn) && (!rex_circe || e[(*circerenai)(e[rn], spec[rn], ip, id, ia, blanc)] != vide))))	/* V3.55 SE, V3.56 TLi */
		{
          return true;
		}

    if (SATCheck &&           /* V3.03, V3.1  TLi */
				 ((traitnbply == noir) ?
					 ((ip == rb) && (!rex_circe || e[(*circerenai)(e[rb], spec[rb], ip, id, ia, noir)] != vide)) :    /* V3.55 SE, V3.56 TLi */
					 ((ip == rn) && (!rex_circe || e[(*circerenai)(e[rn], spec[rn], ip, id, ia, blanc)] != vide))))   /* V3.55 SE, V3.56 TLi */
      return true;

		if (anyanticirce	      /* V3.1  TLi */
		    && ((traitnbply == blanc)
                ? (!rnanticircech(id, ia, ip))
                : (!rbanticircech(id, ia, ip))))
		{
          return true;
		}
      } /* k_cap */

	} /* e[ip] != vide */

    if (CondFlag[imitators] && ((ip == maxsquare + 2) || (ip == maxsquare + 3)) ? !castlingimok(id, ia) : !imok(id, ia)) {				/* V2.4d  TM */
      return false;
	}

	/* the following is used for conditions where checktest is
	 *  needed for generation like follow my leader,
	 * maximummer.....	      V2.90c  TLi
	 */
	if (traitnbply == noir ? flagblackmummer : flagwhitemummer) {
      /* V3.1  TLi */
      boolean	    flag= true, flag_dontaddk_sic;
      numecoup	    test;

      optimize= false;
      if (encore() || flagkil) {			/* V3.44  TLi */
		/*
		** There are moves stored. Check whether the
		** length of the new one is shorter or equal
		** to the currently longest move.
		*/
		short len, curleng;

        if ( traitnbply == noir ? black_length : white_length)
        {
          len = traitnbply == noir
            ?  (*black_length)(id, ia, ip)
            : (*white_length)(id, ia, ip);	 /* V3.01  NG */
          curleng =
		    flagkil
		    ? (traitnbply == noir
               ? (*black_length)(cdkil, cakil, cpkil)
               : (*white_length)(cdkil, cakil, cpkil))
		    : (traitnbply == noir
               ? (*black_length)(cd[nbcou], ca[nbcou], cp[nbcou])
               : (*white_length)(cd[nbcou], ca[nbcou], cp[nbcou]));
        }
        else
        {
          len= 0;
          curleng=0;
        }
        
        if (traitnbply == blanc ? CondFlag[whsupertrans_king] : CondFlag[blsupertrans_king])   /* V3.78  SE */
        {
        		
          len+= MAX_OTHER_LEN * (current_trans_gen!=vide ? 1 : 0);
          curleng+= MAX_OTHER_LEN * (ctrans[nbcou]!=vide ? 1 : 0);
        }  

		if (curleng > len) {
          return true;
		}
		if (curleng == len) {
          goto lab1;
		}
      }

      /* a new longest move */
      if (!we_generate_exact) {			/* V3.20  TLi */
		/* not exact-maxi -> test for selfcheck */
		couleur oneutcoul= neutcoul;		/* V3.53  TLi */
		test= nbcou++;
		cd[nbcou]= id;
		ca[nbcou]= ia;
		cp[nbcou]= ip;				/* V3.0  TLi */
		cmren[nbcou]= mren;			/* V3.78  SE */
        ctrans[nbcou]=current_trans_gen;
		flag_dontaddk_sic= flag_dontaddk;	/* V3.50 SE */
		flag_dontaddk=true;
		while (test < nbcou) {
          if (jouecoup()) /* V3.44  SE/TLi */
			flag= flag && echecc(traitnbply);
          repcoup();
		}
		flag_dontaddk= flag_dontaddk_sic;
		initneutre(oneutcoul);			/* V3.53  TLi */
		if (flag)
          return true;
      }

      nbcou= repere[nbply];
      flagkil= false;
	}
  }

  if (FlagGenMatingMove) {				/* V3.33  TLi */
	/* printf("here\n"); */
	if (FlagMoveOrientatedStip) {
      if (stipulation == stip_ep) {
		if (ia != ep[nbply-1] && ia != ep2[nbply-1])
          return true;
      }
      else if (stipulation == stip_target) {
		if (ia != TargetField)
          return true;
      }
      else if (stipulation == stip_capture
               || stipulation == stip_steingewinn)
      {
		if (e[ip] == vide)
          return true;
      }
      else if (stipulation == stip_castling)	/* V3.35  TLi */
      {
		if (abs(e[id]) != King || abs(id-ia) != 2)
          return true;
      }
	}
	else if (totalortho
             && TSTFLAG(spec[id], Neutral)
             && stipulation != stip_check)
	{
      /* Check if a mating move by a neutral piece can be
      ** retracted by the opponent.
      ** This works also in more general cases, but which?
      */
      if (rb == rn) {
		if (rb == id) {
          /*	 if (e[ip] != vide)    nK-bug ! */
          if (e[ip] == vide)			/* V3.39  NG */
            return true;
		}
		else if ( (e[id] != Pawn && e[id] != -Pawn)
                  || e[ip] == vide)
		{
          return true;
		}
      }
      else if (e[ip] == vide && e[id] != Pawn && e[id] != -Pawn)
		return true;
	}
  }

lab1:
  if (optimize) {				       /* V3.03  TLi */
	int	nbr;
	nbcou++;
	cd[nbcou]= id;
	ca[nbcou]= ia;
	cp[nbcou]= ip;
	if (cntoppmoves(&nbr, trait[nbply])) {
      if (cdkil == id || cakil == ia)
		nbr-= OptKill;
      ot[otc].d= id;
      ot[otc].a= ia;
      ot[otc].p= ip;
      ot[otc].nbr= nbr;
      otc++;
	}
  }
  else if (	 cdkil != id
             || cakil != ia
             || flag_testlegality
             || nonkilgenre) 
  {
	nbcou++;
	cd[nbcou]= id;
	ca[nbcou]= ia;
	cp[nbcou]= ip;					/* V3.0  TLi */
	cmren[nbcou]= mren;
	ctrans[nbcou]=current_trans_gen;
  }
  else {
	flagkil= true;
	cpkil= ip;					/* V3.0  TLi */
  }

  return true;
} /* empile */

/****************  V3.1  TLi  begin  **************/
void	gemaooarider(square i, numvec fir, numvec sec, couleur camp)
{
  square  j1, j2;

  j1 = i + fir;
  j2 = i + sec;
  while ((e[j1] == vide) && (e[j2] == vide)) {
	empile(i, j2, j2);
	j1 += sec;
	j2 += sec;
  }
  if (e[j1] == vide && rightcolor(e[j2], camp)) {
	empile(i, j2, j2);
  }
} /* gemaooarider */

void gemoarider(square i, couleur camp) {
  gemaooarider(i,  23,  47, camp);
  gemaooarider(i,  23,  22, camp);
  gemaooarider(i, -23, -47, camp);
  gemaooarider(i, -23, -22, camp);
  gemaooarider(i,  25,  26, camp);
  gemaooarider(i,  25,  49, camp);
  gemaooarider(i, -25, -26, camp);
  gemaooarider(i, -25, -49, camp);
} /* gemoarider */

void gemaorider(square i, couleur camp) {
  gemaooarider(i,   1,  26, camp);
  gemaooarider(i,   1, -22, camp);
  gemaooarider(i, -24, -47, camp);
  gemaooarider(i, -24, -49, camp);
  gemaooarider(i,  -1, -26, camp);
  gemaooarider(i,  -1,  22, camp);
  gemaooarider(i,  24,  47, camp);
  gemaooarider(i,  24,  49, camp);
} /* gemaorider */

/****************  V3.1  TLi  end  **************/

/***************   V3.64  TLi  begin ***********/
void gemaooariderlion(square i, numvec fir, numvec sec, couleur camp) {
  square  j1, j2;

  j1 = i + fir;
  j2 = i + sec;
  while ((e[j1] == vide) && (e[j2] == vide)) {
	j1 += sec;
	j2 += sec;
  }
  if (e[j1] != obs && e[j2] != obs) {
	if (e[j1] != vide && (e[j2] == vide || rightcolor(e[j2], camp)))
      empile(i, j2, j2);
	if (e[j1] == vide || e[j2] == vide) {
      j1 += sec;
      j2 += sec;
      while ((e[j1] == vide) && (e[j2] == vide)) {
		empile(i, j2, j2);
		j1 += sec;
		j2 += sec;
      }
	}
	if (e[j1] == vide && rightcolor(e[j2], camp))
      empile(i, j2, j2);
  }
}

void gemaoriderlion(square i, couleur camp) {
  gemaooariderlion(i,   1,  26, camp);
  gemaooariderlion(i,   1, -22, camp);
  gemaooariderlion(i, -24, -47, camp);
  gemaooariderlion(i, -24, -49, camp);
  gemaooariderlion(i,  -1, -26, camp);
  gemaooariderlion(i,  -1,  22, camp);
  gemaooariderlion(i,  24,  47, camp);
  gemaooariderlion(i,  24,  49, camp);
} /* gemaoriderlion */

void gemoariderlion(square i, couleur camp) {    /* V3.65  TLi */
  gemaooariderlion(i,  23,  47, camp);
  gemaooariderlion(i,  23,  22, camp);
  gemaooariderlion(i, -23, -47, camp);
  gemaooariderlion(i, -23, -22, camp);
  gemaooariderlion(i,  25,  26, camp);
  gemaooariderlion(i,  25,  49, camp);
  gemaooariderlion(i, -25, -26, camp);
  gemaooariderlion(i, -25, -49, camp);
} /* gemoariderlion */

/***************   V3.64  TLi  end *************/

void gebrid(square i, numvec kbeg, numvec kend) {	/* V2.60  NG */
  /* generate white rider moves from vec[kbeg] to vec[kend] */
  square j;
  numvec k;

  for (k= kbeg; k<= kend; k++) {
	j= i + vec[k];
	while ((e[j] == vide) && empile(i, j, j))	/* V2.4d  TM */
      j+= vec[k];
	if (e[j] <= roin)
      empile(i, j, j);
  }
}

void genrid(square i, numvec kbeg, numvec kend) {	/* V2.60  NG */
  /* generate black rider moves from vec[kbeg] to vec[kend] */
  square j;
  numvec k;

  for (k= kbeg; k <= kend; k++) {
	j= i + vec[k];
	while ((e[j] == vide) && empile(i, j, j))	/* V2.4d  TM */
      j+= vec[k];
	if (e[j] >= roib)
      empile(i, j, j);
  }
}

void genbouncer(square i, numvec kbeg, numvec kend, couleur camp) {	     /* V4.03 SE */
    numvec  k;
    piece   p1;
    square  j, j1;

    for (k= kend; k >= kbeg; k--) {	   
	finligne(i, vec[k], p1, j);
    for (j1=i-vec[k];j1 !=2*i-j && e[j1] == vide; j1-=vec[k]);
    if (j1 == 2*i-j && (e[j1] == vide || rightcolor(e[j1], camp)))
    {
        empile(i,j1,j1);
    }
    }
}

boolean testempile(square id, square ia, square ip) {
  numecoup k;

  for (k= nbcou; k > testdebut; k--) {	 /* V1.3c, V3.00  NG */
	if (ca[k] == ia) {
      return true;
	}
  }
  return empile(id, ia, ip);			     /* V1.3c  NG */
}

void gebleap(square i, numvec kbeg, numvec kend) {	/* V2.60  NG */
  /* generate white leaper moves from vec[kbeg] to vec[kend] */
  square  zw;
  numvec  k;

  for (k= kbeg; k<= kend; k++) {
	zw= i + vec[k];
	if (e[zw] <= vide)
      empile(i, zw, zw);
  }
}

void genleap(square i, numvec kbeg, numvec kend) {	/* V2.60  NG */
  /* generate black leaper moves from vec[kbeg] to vec[kend] */
  square  zw;
  numvec  k;

  for (k= kbeg; k <= kend; k++) {
	zw= i + vec[k];
	if (e[zw] == vide || e[zw] >= roib)
      empile(i, zw, zw);
  }
}


/*****	V3.34 TLi  ***** begin *****/
void geriderhopper(
  square	i,
  numvec	kbeg,
  numvec	kend,	   /* V3.34 TLi */
  smallint	run_up,
  smallint	jump,
  couleur	camp)
{
  /* generate rider-hopper moves from vec[kbeg] to vec[kend] */

  piece   p;
  square  j, j2;
  numvec  k;

  for (k= kbeg; k <= kend; k++) {
	if (run_up) {
      /* run up of fixed length */
      /* contragrashopper type */
      j2= i;
      if (run_up > 1) {
		/* The run up is longer.
		   Check if there is an obstacle between the hopper
		   and the hurdle
        */
		smallint ran_up= run_up;
		while (--ran_up && e[j2 += vec[k]] == vide)
          ;
		if (ran_up) {
          /* there is an obstacle -> next line */
          continue;
		}
      }
      p= e[j2 += vec[k]];
      if (p == vide) {
		/* there is no hurdle -> next line */
		continue;
      }
	}
	else {
      /* run up of flexible length
       * lion, grashopper type
       */
      finligne(i, vec[k], p, j2);
	}
	if (p != obs) {
      j= j2;
      if (jump) {
		/* jump of fixed length */
		/* grashopper or grashopper-2 type */
		if (jump > 1) {
          /* The jump up is a longer one.
             Check if there is an obstacle between
             the hurdle and the target field
          */
          smallint jumped= jump;
          while (--jumped && e[j += vec[k]] == vide)
			;
          if (jumped) {
			/* there is an obstacle -> next line */
			continue;
          }
		}
		j += vec[k];
		if ((rightcolor(e[j], camp) || (e[j] == vide))
            && hopimcheck(i, j, j2, vec[k]))
		{
          empile(i, j, j);
		}
      }
      else {
		/* jump of flexible length */
		/* lion, contragrashopper type */
		while (e[j += vec[k]] == vide) {
          if (hopimcheck(i, j, j2, vec[k])) {
			empile(i, j, j);
          }
		}

		if (rightcolor(e[j], camp)
            && hopimcheck(i, j, j2, vec[k]))
		{
          empile(i, j, j);
		}
      }
	}
  }
}
/*****	V3.34 TLi  *****  end  *****/

#define _rrider 0
#define _rlion 1
#define _rhopper 2
#define _rrao 3

void grose(square i, int type, couleur camp) {		/* V3.14  TLi,
                                                       V3.63  NG */
  smallint k1, k2;
  square j;

  for (k1= 9; k1 <= 16; k1++) {
	j= i;
	k2= 0;
	do {						/* V2.4d  TM */
      j+= vec[k1 + k2++];				/* V2.4d  TM */
	} while ((e[j] == vide) && testempile(i, j, j)); /* V2.4d  TM */

	if (type != _rrao && rightcolor(e[j], camp)) {	/* V3.14  TLi,
                                                       V3.63  NG */
      testempile(i, j, j);
	}
	j= i;
	k2= 8;
	do {						/* V2.4d  TM */
      j+= vec[k1 + k2--];				/* V2.4d  TM */
	}
	while ((e[j] == vide) && testempile(i, j, j));	/* V2.4d  TM */

	if (type != _rrao && rightcolor(e[j], camp)) {	/* V3.14  TLi,
                                                       V3.63  NG */
      testempile(i, j, j);
	}
  }
}

void groselh (square i, numvec k1, numvec k2, int type, couleur camp) {
  /* generate rose lion/hopper moves on the single line k1
     and direction k2					   V3.23  TLi
  */
  square j = i;
  numvec kinc = k2 ? -1 : 1;
  do {
	j+= vec[k1 + k2];
	k2+= kinc;
  } while (e[j] == vide);
  if (j != i && e[j] != obs) {
	switch (type) {

    case _rhopper:
      j+= vec[k1 + k2];
      break;

    case _rlion:
      do {
		j+= vec[k1 + k2];
		k2+= kinc;
      } while (e[j] == vide && testempile(i, j, j));
      break;

    case _rrao:					/* V3.63  NG */
      do {
		j+= vec[k1 + k2];
		k2+= kinc;
      } while (e[j] == vide);
      /* remember:  e[j] != vide  now !! */
      break;
	} /* switch */
	if (j != i && (e[j] == vide || rightcolor(e[j], camp))) {
      /* V3.42  TLi */
      testempile(i, j, j);
	}
  }
}

void ghamst(square i) {					/* V3.12  TLi */
  piece p;
  square j;
  numvec k;

  for (k= 8; k > 0; k--) {
	finligne(i, vec[k], p, j);
	if (p != obs) {
      j-= vec[k];					/* V3.02  TLi */
      empile(i, j, j);
	}
  }
}

void gmhop(
  square	i,
  numvec	kend,
  numvec	kanf,
  smallint	m,
  couleur	camp)			     /* V3.14  TLi, V3.62  NG */
{
  piece p;
  square j, j1;
  numvec k, k1;

  /* ATTENTION:
   *	m == 0: moose	 45 degree hopper
   *	m == 1: eagle	 90 degree hopper
   *	m == 2: sparrow 135 degree hopper
   *
   *	kend == 8, kanf == 1: queen types  (moose, eagle, sparrow)
   *	kend == 8, kanf == 5: bishop types
   *			      (bishopmoose, bishopeagle, bishopsparrow)
   *	kend == 4, kanf == 1: rook types
   *			      (rookmoose, rookeagle, rooksparrow)
   */

  /* different moves from one piece to the same square are possible.
   * so we have to use testempile !!
   */

  for (k= kend; k >= kanf; k--) {
	finligne(i, vec[k], p, j);
	if (p != obs) {
      k1= k<<1;
      j1= j + mixhopdata[m][k1];
      if ((e[j1] == vide) || (rightcolor(e[j1], camp)))
		testempile(i, j1, j1);
      j1= j + mixhopdata[m][k1 - 1];
      if ((e[j1] == vide) || (rightcolor(e[j1], camp)))
		testempile(i, j1, j1);
	}
  }
}

void glocust(
  square	i,
  numvec	kbeg,
  numvec	kend,
  couleur	camp)					/* V3.53  TLi */
{
  piece p;
  square j;
  numvec k;

  for (k= kbeg; k <= kend; k++) {
	finligne(i, vec[k], p, j);
	if (rightcolor(p, camp)) {
      if (e[j + vec[k]] == vide) {
		empile(i, j + vec[k], j);
      }
	}
  }
} /* glocust */

void gmarin(
  square	i,
  numvec	kbeg,
  numvec	kend,
  couleur	camp)					/* V3.14  TLi */
{
  /* generate marin-pieces moves from vec[kbeg] to vec[kend] */

  square j;
  numvec k;

  for (k= kbeg; k<= kend; k++) {
	j= i + vec[k];
	while ((e[j] == vide) && empile(i, j, j)) {
      j+= vec[k];
	}
	if (rightcolor(e[j], camp)) {
      if (e[j + vec[k]] == vide) {
		empile(i, j + vec[k], j);		/* V3.02  TLi */
      }
	}
  }
}

void gchin(
  square	i,
  numvec	kbeg,
  numvec	kend,
  couleur	camp)			     /* V2.60  NG, V3.14  TLi */
{
  /* generate chinese-rider moves from vec[kbeg] to vec[kend] */
  square  j, j2;
  piece   p;
  numvec  k;

  for (k= kbeg; k <= kend; k++) {
	j= i + vec[k];
	while (e[j] == vide) {
      empile(i, j, j);
      j+= vec[k];
	}
	if (e[j] != obs) {
      finligne(j, vec[k], p, j2);
      if (rightcolor(p, camp)) {
		empile(i, j2, j2);
      }
	}
  }
}

void gnequi(square i, couleur camp) {			/* V3.14  TLi */
  /* Non-Stop-Equihopper */
  square coin, sautoir, j;
  smallint k1, k2;

  coin= coinequis(i);
  for (k1= 3; k1 >= 0; k1--) {
	for (k2= 72; k2 >= 0; k2-= 24) {		/* V2.90  NG */
      sautoir = coin + k1 + k2;			/* V2.90  NG */
      if (e[sautoir] != vide) {
		j= (sautoir<<1) - i;
		if (  (e[j] == vide
		       || rightcolor(e[j], camp))	/* V2.1c  NG,
                                               V3.14  TLi */
              && hopimok(i,j,sautoir,sautoir-i))	/* V3.12  TM */
		{
          empile(i, j, j);
		}
      }
	}
  }
}

void gnequiapp(square i, couleur camp) {			/* V3.88  SE */
  /* Non-Stop-Equihopper */
  square coin, sautoir, j;
  smallint k1, k2;

  coin= coinequis(i);
  for (k1= 3; k1 >= 0; k1--) {
	for (k2= 72; k2 >= 0; k2-= 24) {		/* V2.90  NG */
      j = coin + k1 + k2;			/* V2.90  NG */
      sautoir = (j<<1) - i;
      if (j != i && e[sautoir] != vide) {
		if (  (e[j] == vide
		       || rightcolor(e[j], camp))	/* V2.1c  NG,
                                               V3.14  TLi */
              && hopimok(i,j,sautoir,sautoir-i))	/* V3.12  TM */
		{
          empile(i, j, j);
		}
      }
	}
  }
}

void gkang(square i, couleur camp) {
  piece p;
  square j,j1;
  numvec k;

  for (k= 8; k > 0; k--) {
	finligne(i, vec[k], p, j);
	if (p != obs) {
      finligne(j, vec[k], p, j1);
      if (p != obs) {
		j1+= vec[k];
		if ((e[j1] == vide) || (rightcolor(e[j1], camp))) {
          empile(i, j1, j1);
		}
      }
	}
  }
}

void grabbit(square i, couleur camp) {
  piece p;
  square j,j1;
  numvec k;

  for (k= 8; k > 0; k--) {
	finligne(i, vec[k], p, j);
	if (p != obs) {
      finligne(j, vec[k], p, j1);
      if (p != obs) {
		while (e[j1 += vec[k]] == vide) {
          empile(i, j1, j1);
        }
        if (rightcolor(e[j1], camp)) {
          empile(i, j1, j1);
        }
      }
	}
  }
}

void gbob(square i, couleur camp) {
  piece p;
  square j,j1;
  numvec k;

  for (k= 8; k > 0; k--) {
	finligne(i, vec[k], p, j);
	if (p != obs) {
      finligne(j, vec[k], p, j1);
      if (p != obs) {
        finligne(j1, vec[k], p, j);
		if (p != obs) {
          finligne(j, vec[k], p, j1);
          if (p != obs) {
			while (e[j1 += vec[k]] == vide) {
              empile(i, j1, j1);
			}
            if (rightcolor(e[j1], camp)) {
              empile(i, j1, j1);
			}
          }
        }
      }
	}
  }
}

void gcs(square i, numvec k1, numvec k2, couleur camp)	/* V1.5c  NG, V3.14  TLi */
{
  square j;

  j= i;
  while ( (e[j+= k1]) == vide ) {
	testempile(i, j, j);
	if ( (e[j+= k2]) == vide )
      testempile(i, j, j);
	else
      break;
  }
  if (rightcolor(e[j], camp))				/* V3.14  TLi */
	testempile(i, j, j);
  j = i;
  while ( (e[j+= k1]) == vide ) {
	testempile(i, j, j);
	if ( (e[j-= k2]) == vide )
      testempile(i, j, j);
	else
      break;
  }
  if (rightcolor(e[j], camp))		    /* V3.14  TLi */
	testempile(i, j, j);
}

void gubi(square sq, square i, couleur camp) {		/* V1.7c  NG,
                                                       V3.14  TLi */
  numvec k;
  square j;

  e_ubi[i]= obs;
  for (k= 9; k <= 16; k++) {
	j= i + vec[k];
	if (e_ubi[j] == vide) {
      empile(sq, j, j);
      gubi(sq, j, camp);
	}
	else {
      if (rightcolor(e_ubi[j], camp))
		empile(sq, j, j);
	}
	e_ubi[j]= obs;
  }
}

void grfou(
  square	i,
  square	in,
  numvec	k,
  smallint	x,
  couleur	camp,
  boolean	(*generate)(square,square,square))	/* V2.1c  NG,
                                                   V3.14  TLi */
{
  /* ATTENTION:
     if first call of x is 1 and boolnoedge[i]
     use empile() for generate,
     else
     use testempile() for generate !!
  */
  smallint k1;
  square j;

  if (e[j= in + k] == obs) {
	return;
  }
  while (e[j] == vide) {
	(* generate)(i, j, j);
	j += k;
  }
  if (rightcolor(e[j], camp)) {
	(* generate)(i, j, j);
  }
  else if (x && (e[j] == obs)) {
	j -= k;
	k1= 5;
	while (vec[k1] != k)
      k1++;
	k1= k1<<1;
	grfou(i, j, mixhopdata[1][k1], x - 1, camp, generate);
	k1--;
	grfou(i, j, mixhopdata[1][k1], x - 1, camp, generate);
  }
}

void gcard(
  square	i,
  square	in,
  numvec	k,
  smallint	x,
  couleur	camp,
  boolean	(*generate)(square,square,square))	/* V2.1c  NG,
                                                   V3.14  TLi */
{
  /* ATTENTION:
	 if first call of x is 1
     use	empile() for generate,
	 else
     use testempile() for generate !!
  */
  smallint k1;
  square j;

  j= in + k;
  while (e[j] == vide) {
	(* generate)(i, j, j);
	j += k;
  }
  if (rightcolor(e[j], camp)) {	    /* V3.14  TLi */
	(* generate)(i, j, j);
  }
  else if (x && (e[j] == obs)) {
	for (k1= 1; k1<=4; k1++) {
      if (e[j + vec[k1]] != obs) {
		break;
      }
	}
	if (k1 <= 4) {
      j += vec[k1];
      if (rightcolor(e[j], camp)) {   /* V3.14  TLi */
		(* generate)(i, j, j);
      }
      else if (e[j] == vide) {
		(* generate)(i, j, j);
		k1= 5;
		while (vec[k1] != k)
          k1++;
		k1= k1<<1;
		if (e[j + mixhopdata[1][k1]] == obs)
          k1--;
		gcard(i, j, mixhopdata[1][k1], x - 1, camp, generate);
      }
	}
  }
}

void  grefc(square i, square i1, smallint x, couleur camp) {
  /* V2.1c  NG,
     V3.14  TLi */

  square j;
  numvec k;

  /* ATTENTION:   first call of grefcn: x must be 2 !!   */

  for (k= 9; k <= 16; k++) {
	if (x) {
      if (e[j= i1 + vec[k]] == vide) {
        empile(i, j, j);
        /* if (! boolnoedge[j]) */
        if (!NoEdge(j)) {			/* V3.22  TLi */
          grefc(i, j, x - 1, camp);	/* V3.14  TLi */
        }
      }
      else if (rightcolor(e[j], camp)) {		/* V3.14  TLi */
		empile(i, j, j);
      }
	}
	else {
      for (k= 9; k <= 16; k++) {
		if ((e[j= i1 + vec[k]] == vide)
            || (rightcolor(e[j], camp)))		/* V3.14  TLi */
		{
          testempile(i, j, j);
		}
      }
	}
  }
} /* grefc */

void gequi(square i, couleur camp) {	     /* V2.60  NG, V3.14  TLi */
  /* Equihopper */
  numvec  k;
  piece   p1;
  square  j, j1;

  for (k= 8; k > 0; k--) {	    /* 0,2; 0,4; 0,6; 2,2; 4,4; 6,6; */
	finligne(i, vec[k], p1, j);
	if (p1 != obs) {
      finligne(j, vec[k], p1, j1);
      if (   abs(j1 - j) > abs(j - i)
             && hopimcheck(i,j+j-i,j,vec[k]))	 /* V3.12  TM */
      {
		empile(i, j + j - i, j + j - i);
      }
      else if (	abs(j1 - j) == abs(j - i)
                && rightcolor(p1, camp)		/* V3.14  TLi */
                && hopimcheck(i,j1,j,vec[k]))	 /* V3.12  TM */
      {
		empile(i, j1, j1);
      }
	}
  }
  for (k= 17; k <= 40; k++) {     /* 2,4; 2,6; 4,6; */ /* V2.90  NG */
	if (   abs(e[i + vec[k]]) >= roib
           && (e[j1= i + (vec[k]<<1)] == vide
               || rightcolor(e[j1], camp))
           && hopimcheck(i,j1,i+vec[k],vec[k])) /* V3.12  TM */
      empile(i, j1, j1);
  }
}

void gequiapp(square i, couleur camp) {	     /* V3.88 SE */
  /* Equihopper */
  numvec  k;
  piece   p,p1;
  square  j, j1;

  for (k= 8; k > 0; k--) {	    /* 0,2; 0,4; 0,6; 2,2; 4,4; 6,6; */
	finligne(i, vec[k], p1, j);
	if (p1 != obs) {
      if ( ! ( (j/24+i/24)%2 || (j%24 + i%24)%2 )    /* will midpoint (i,j) be a square? */ 
           && hopimcheck(i,j,(j+i)/2,vec[k]))	 /* V3.12  TM */
      {
		empile(i, (j+i)/2, (j+i)/2);
      }
      finligne(j, vec[k], p, j1);
      if (	p != obs
            && abs(j1 - j) == abs(j - i)
            && rightcolor(p1, camp)		/* V3.14  TLi */
            && hopimcheck(i,j,j1,vec[k]))	 /* V3.12  TM */
      {
		empile(i, j, j);
      }
	}
  }
  for (k= 17; k <= 40; k++) {     /* 2,4; 2,6; 4,6; */ /* V2.90  NG */
	if (   abs(e[i + (vec[k]<<1)]) >= roib
           && (e[j1= i + vec[k]] == vide
               || rightcolor(e[j1], camp))
           && hopimcheck(i,j1,i + (vec[k]<<1),vec[k])) /* V3.12  TM */
      empile(i, j1, j1);
  }
}

void gcat(square i, couleur camp) {	     /* V2.60  NG, V3.14  TLi */
  /* generate moves of a CAT */
  numvec  k;
  square  j;

  for (k= 9; k <= 16; k++) {
	j= i + vec[k];
	if (rightcolor(e[j], camp)) {			/* V3.14  TLi */
      empile(i, j, j);
      continue;
	}
	else {
      while (e[j] == vide) {
		empile(i, j, j);
		j+= mixhopdata[3][k];
      }
      if (rightcolor(e[j], camp)) {		/* V3.14  TLi */
		empile(i, j, j);
      }
	}
  }
}

/*****	V3.12  TLi  begin *****/

void gmaooa(
  square	id,
  square	pass,
  square	ia1,
  square	ia2,
  couleur	camp)
{
  if (e[pass] == vide) {
	if (e[ia1] == vide || rightcolor(e[ia1], camp)) {
      if (maooaimcheck(id, ia1, pass))   /* V3.81 SE */
        empile(id, ia1, ia1);
	}
	if (e[ia2] == vide || rightcolor(e[ia2], camp)) {
      if (maooaimcheck(id, ia2, pass))   /* V3.81 SE */
        empile(id, ia2, ia2);
	}
  }
}

void gmao(square i, couleur camp) {
  gmaooa(i, i + 24, i + 47, i + 49, camp);
  gmaooa(i, i - 24, i - 47, i - 49, camp);
  gmaooa(i, i +  1, i + 26, i - 22, camp);
  gmaooa(i, i -  1, i - 26, i + 22, camp);
}

void gmoa(square i, couleur camp) {
  gmaooa(i, i + 23, i + 47, i + 22, camp);
  gmaooa(i, i - 23, i - 47, i - 22, camp);
  gmaooa(i, i + 25, i + 26, i + 49, camp);
  gmaooa(i, i - 25, i - 26, i - 49, camp);
}

/***** V3.12  TLi  end *****/

void gdoubleg(square i, couleur camp) {			/* V3.44  SE */
  numvec k,k1;
  piece p;
  square j,j1;

  for (k=8; k>0; k--) {
	finligne(i, vec[k], p, j);
	if (p!=obs) {
      j+=vec[k];
      if (e[j]==vide) {
		for (k1=8; k1>0; k1--) {
          finligne(j, vec[k1], p, j1);
          if (p!=obs) {
			j1+=vec[k1];
			if ((e[j1]==vide) || (rightcolor(e[j1],camp))) {
              empile(i, j1, j1);
			}
          }
		}
      }
	}
  }
}

typedef enum
{
  UP,
  DOWN
} UPDOWN;

void filter(square i, numecoup prevnbcou, UPDOWN u)
{
  numecoup s = prevnbcou+1;
  while (s<=nbcou)
    if ((u==DOWN && ca[s]-i>-8)
        || (u==UP && ca[s]-i<8))
    {
      memmove(cd+s, cd+s+1, (nbcou-s) * sizeof cd[s]);
      memmove(ca+s, ca+s+1, (nbcou-s) * sizeof ca[s]);
      memmove(cp+s, cp+s+1, (nbcou-s) * sizeof cp[s]);
      --nbcou;
    }
    else
      ++s;
}

void genhunt(square i, piece p, PieNam pabs)
{
  /*   PieNam const pabs = abs(p);	*/	/* V4.01  NG */
  assert(pabs>=Hunter0);
  assert(pabs<Hunter0+maxnrhuntertypes);
  
  {
    unsigned int const typeofhunter = pabs-Hunter0;
    HunterType const * const huntertype = huntertypes+typeofhunter;

    if (p>0) {
      numecoup savenbcou = nbcou;
      gen_wh_piece(i,huntertype->home);
      filter(i,savenbcou,DOWN);

      savenbcou = nbcou;
      gen_wh_piece(i,huntertype->away);
      filter(i,savenbcou,UP);
    }
    else {
      numecoup savenbcou = nbcou;
      gen_bl_piece(i,-huntertype->away);
      filter(i,savenbcou,DOWN);

      savenbcou = nbcou;
      gen_bl_piece(i,-huntertype->home);
      filter(i,savenbcou,UP);
    }
  }
}

void gfeerrest(square i, piece p, couleur camp) {	/* V3.14  TLi */
  numvec k;
  square *bnp;					/* V2.60  NG */
  PieNam const pabs = abs(p);				/* V4.01  NG */

  switch  (abs(p)) {
  case maob:
	gmao(i, camp);					/* V3.12  TLi */
	return;

  case paob:
	gchin(i, 1, 4, camp);				/* V3.14  TLi */
	return;

  case leob:
	gchin(i, 1, 8, camp);				/* V3.14  TLi */
	return;

  case vaob:
	gchin(i, 5, 8, camp);				/* V3.14  TLi */
    return;
  case naob:
	gchin(i, 9, 16, camp);				/* V3.62  TLi */
	return;

  case roseb:
	grose(i, _rrider, camp);	     /* V3.14  TLi, V3.63  NG */
	return;

  case nequib:
	gnequi(i, camp);				/* V3.14  TLi */
	return;

  case locb:
	glocust(i, 1, 8, camp);				/* V3.53  TLi */
	return;

  case nightlocustb:				/* V3.53  TLi */
	glocust(i, 9, 16, camp);
	return;

  case bishoplocustb:				/* V3.53  TLi */
	glocust(i, 5, 8, camp);
	return;

  case rooklocustb:					/* V3.53  TLi */
	glocust(i, 1, 4, camp);
	return;

  case kangoub:
	gkang(i, camp);			     /* V1.5c  NG, V3.14  TLi */
	return;

  case csb:
	for (k= 9; k <= 16; k++)			/* V2.1c  NG */
      gcs(i, vec[k], vec[25 - k], camp);		/* V3.14  TLi */
	return;

  case hamstb:
	ghamst(i);					/* V3.12  TLi */
	return;

  case ubib:
	for (bnp= boardnum; *bnp; bnp++)		/* V1.7c  NG */
      e_ubi[*bnp]= e[*bnp];
	gubi(i, i, camp);				/* V3.14  TLi */
	return;

  case mooseb:
	gmhop(i, 8, 1, 0, camp);  /* V2.1c  NG, V3.14  TLi, V3.62  NG */
	return;

  case eagleb:
	gmhop(i, 8, 1, 1, camp);  /* V2.1c  NG, V3.14  TLi, V3.62  NG */
	return;

  case sparrb:
	gmhop(i, 8, 1, 2, camp);  /* V2.1c  NG, V3.14  TLi, V3.62  NG */
	return;

  case margueriteb:					/* TLi V3.64  */
	gmhop(i, 8, 1, 0, camp);
	gmhop(i, 8, 1, 1, camp);
	gmhop(i, 8, 1, 2, camp);
	gerhop(i, 1, 8, camp);
	ghamst(i);
	return;

  case archb:					/* V2.1c  NG */
	if (NoEdge(i)) {				/* V3.22  TLi */
      for (k= 5; k <= 8; k++)
		grfou(i, i, vec[k], 1, camp, empile);	/* V3.14  TLi */
	}
	else {
      for (k= 5; k <= 8; k++)
		grfou(i, i, vec[k], 1, camp, testempile); /* V3.14TLi */
	}
	return;

  case reffoub:
	for (k= 5; k <= 8; k++)				/* V2.1c  NG */
      grfou(i, i, vec[k], 4, camp, testempile);	/* V3.14  TLi */
	return;

  case cardb:
	for (k= 5; k <= 8; k++)		/* V2.1c  NG */
      gcard(i, i, vec[k], 1, camp, empile);   /* V3.14  TLi */
	return;

  case dcsb:
	for (k= 9; k <= 14; k++)			/* V2.1c  NG */
      gcs(i, vec[k], vec[23 - k], camp);		/* V3.14  TLi */
	for (k= 15; k <= 16; k++)			/* V2.1c  NG */
      gcs(i, vec[k], vec[27 - k], camp);		/* V3.14  TLi */
	return;

  case refcb:
	grefc(i, i, 2, camp);		     /* V2.1c  NG, V3.14  TLi */
	return;

  case equib:
	gequi(i, camp);			     /* V2.60  NG, V3.14  TLi */
	return;

  case catb:
	gcat(i, camp);					/* V3.14  NG */
	return;

  case sireneb:
	gmarin(i, 1, 8, camp);				/* V3.14  TLi */
	return;

  case tritonb:
	gmarin(i, 1, 4, camp);				/* V3.14  TLi */
	return;

  case nereidb:
	gmarin(i, 5, 8, camp);				/* V3.14  TLi */
	return;

  case orphanb:
	gorph(i, camp);			      /* V3.0  TLi, V3.14  NG */
	return;

  case friendb:
	gfriend(i, camp);				/* V3.65  TLi */
	return;

  case edgehb:
	gedgeh(i, camp);				/* V3.14 TLi */
	return;

  case moab:
	gmoa(i, camp);					/* V3.12  TLi */
	return;

  case moaridb:
	gemoarider(i, camp);
	return;

  case maoridb:
	gemaorider(i, camp);
	return;

  case bscoutb:
	for (k= 5; k <= 8; k++)				/* V3.05  NG */
      gcs(i, vec[k], vec[13 - k], camp);		/* V3.14  TLi */
	return;

  case gscoutb:
	for (k= 4; k > 0; k--)				/* V3.05  NG */
      gcs(i, vec[k], vec[5 - k], camp);		/* V3.14  TLi */
	return;

  case skyllab:
	geskylla(i, camp);				/* V3.12  TLi */
	return;

  case charybdisb:
	gecharybdis(i, camp);				/* V3.12  TLi */
	return;

    /*****  V3.22  TLi	*****/
  case andergb:					/* V3.65  TLi */
  case sb:
	gerhop(i, 1, 8, camp);
	return;

  case lionb:
	gelrhop(i, 1, 8, camp);
	return;

  case nsautb:
	gerhop(i, 9, 16, camp);
	return;

  case camhopb:
	gerhop(i, 33, 40, camp);
	return;

  case zebhopb:
	gerhop(i, 25, 32, camp);
	return;

  case gnuhopb:
	gerhop(i, 33, 40, camp);
	gerhop(i, 9, 16, camp);
	return;

  case tlionb:
	gelrhop(i, 1, 4, camp);
	return;

  case flionb:
	gelrhop(i, 5, 8, camp);
	return;

  case rookhopb:
	gerhop(i, 1, 4, camp);
	return;

  case bishophopb:
	gerhop(i, 5, 8, camp);
	return;
  case contragrasb:
	gecrhop(i, 1, 8, camp);
	return;						/* V3.29  NG */

  case roselionb:
	for (k= 9; k <= 16; k++) {
      groselh(i, k, 0, _rlion, camp);
      groselh(i, k, 8, _rlion, camp);
	}
	return;

  case rosehopperb:
	for (k= 9; k <= 16; k++) {
      groselh(i, k, 0, _rhopper, camp);
      groselh(i, k, 8, _rhopper, camp);
	}
	return;

  case g2b:
	gerhop2(i, 1, 8, camp);				/* V3.34  TLi */
	return;

  case g3b:
	gerhop3(i, 1, 8, camp);				/* V3.35  TLi */
	return;
  case khb:
	geshop(i, 1, 8, camp);				/* V3.44  SE */
	return;

  case doublegb:
	gdoubleg(i, camp);				/* V3.44  SE */
	return;

  case orixb:
	gorix(i, camp);					 /* V3.44  NG */
	return;

  case gralb:
	if (camp==blanc)				/* V3.46  NG */
      gebleap(i, 65, 68);    /* alfilb */
	else
      genleap(i, 65, 68);    /* alfiln */
	gerhop(i, 1, 4, camp);	   /* rookhopper */
	return;

  case rookmooseb:
	gmhop(i, 4, 1, 0, camp);			/* V3.62  NG */
	return;

  case rookeagleb:
	gmhop(i, 4, 1, 1, camp);			/* V3.62  NG */
	return;

  case rooksparrb:
	gmhop(i, 4, 1, 2, camp);			/* V3.62  NG */
	return;

  case bishopmooseb:
	gmhop(i, 8, 5, 0, camp);			/* V3.62  NG */
	return;

  case bishopeagleb:
	gmhop(i, 8, 5, 1, camp);			/* V3.62  NG */
	return;

  case bishopsparrb:
	gmhop(i, 8, 5, 2, camp);			/* V3.62  NG */
	return;

  case raob:
	for (k= 9; k <= 16; k++) {			/* V3.63  NG */
      groselh(i, k, 0, _rrao, camp);
      groselh(i, k, 8, _rrao, camp);
	}
	grose(i, _rrao, camp);
	return;

  case scorpionb:
	if (camp==blanc)				/* V3.63  NG */
      gebleap(i, 1, 8);	    /* ekingb */
	else
      genleap(i, 1, 8);	    /* ekingn */
	gerhop(i, 1, 8, camp);	    /* grashopper */
	return;

  case nrlionb:
	gelrhop(i, 9, 16, camp);			/* V3.64  TLi */
	return;

  case mrlionb:
	gemaoriderlion(i, camp);			/* V3.64  TLi */
	return;

  case molionb:
	gemoariderlion(i, camp);			/* V3.64  TLi */
	return;

  case dolphinb:                                   /* V3.74  TLi */
    gkang(i, camp);
    gerhop(i, 1, 8, camp);
    return;

  case rabbitb:
	grabbit(i, camp);				/* V3.76  NG */
	return;

  case bobb:
	gbob(i, camp);					/* V3.76  NG */
	return;

  case equiengb:				/* V3.78  SE */
	gequiapp(i, camp);
	return;

  case equifrab:				/* V3.78  SE */
    gnequiapp(i, camp);
	return;

  case querqub:   /* V3.78 SE */ 
	switch (i%24-8) {
    case 0:
    case 7: 	if (camp == blanc)
        gebrid(i, 1, 4);
      else
        genrid(i, 1, 4);
      break;
    case 2:
    case 5: 	if (camp == blanc)
        gebrid(i, 5, 8);
      else
        genrid(i, 5, 8);
      break;
    case 3:	if (camp == blanc)
        gebrid(i, 1, 8);
      else
        genrid(i, 1, 8);
      break;
    case 1:
    case 6:	if (camp == blanc)
        gebleap(i, 9, 16);
      else
        genleap(i, 9, 16);
      break;
    case 4:	if (camp == blanc)
        gebleap(i, 1, 8);
      else
        genleap(i, 1, 8);
      break;
    }
  break;

  case bouncerb : 
    genbouncer(i, 1, 8, camp);
    break;

  case rookbouncerb : 
    genbouncer(i, 1, 4, camp);
    break;

  case bishopbouncerb : 
    genbouncer(i, 5, 8, camp);
    break;

  default:
	/* Since pieces like DUMMY fall through 'default', we have */
	/* to check exactly if there is something to generate ...  */
	if ((pabs>=Hunter0) && (pabs<Hunter0+maxnrhuntertypes))	/* V4.01  NG */
      genhunt(i,p,pabs);
  }
} /* gfeerrest */

/* Two auxiliary functions for generating super pawn moves */
void gen_sp_nocaptures(square i, numvec dir) {
  /* generates non capturing moves of a super pawn in direction dir */
  square  j;

  /* it can move from first rank */
  for (j= i + dir; e[j] == vide; j+= dir)
	empile(i, j, j);
}

void gen_sp_captures(square i, numvec dir, couleur camp) {
  /* generates capturing moves of a super pawn of colour camp in
     direction dir.  */

  square  j;
  piece   p;

  /* it can move from first rank */
  finligne(i, dir, p, j);
  if (rightcolor(p, camp))
	empile(i, j, j);
}

void gfeerblanc(square i, piece p) {
  testdebut= nbcou;					/* V3.00  NG */
  switch(p) {
  case nb:
	gebrid(i, 9, 16);
	return;

  case zb:
	gebleap(i, 25, 32);
	return;

  case chb:
	gebleap(i, 33, 40);
	return;

  case gib:
	gebleap(i, 41, 48);
	return;

  case rccinqb:
	gebleap(i, 69, 80);
	return;

  case bub:
	gebleap(i, 49, 60);
	return;

  case vizirb:
	gebleap(i, 1, 4);
	return;

  case alfilb:
	gebleap(i, 65, 68);
	return;

  case fersb:
	gebleap(i, 5, 8);
	return;

  case dabb:
	gebleap(i, 61, 64);
	return;

  case pbb:
	genpbb(i);
	return;

  case amazb:
	gebrid(i, 1, 8);				/* V1.4c  NG */
	gebleap(i, 9, 16);
	return;

  case impb:
	gebrid(i, 1, 4);				/* V1.4c  NG */
	gebleap(i, 9, 16);
	return;

  case princb:
	gebrid(i, 5, 8);				/* V1.4c  NG */
	gebleap(i, 9, 16);
	return;

  case gnoub:
	gebleap(i, 33, 40);				/* V1.4c  NG */
	gebleap(i, 9, 16);
	return;

  case antilb:
	gebleap(i, 49, 56);				/* V1.5c  NG */
	return;

  case ecurb:
	gebleap(i, 61, 68);				/* V1.5c  NG */
	gebleap(i, 9, 16);
	return;

  case waranb:
	gebrid(i, 9, 16);				/* V1.5c  NG */
	gebrid(i, 1, 4);
	return;

  case dragonb:
	genpb(i);					/* V1.5c  NG */
	gebleap(i, 9, 16);
	return;

  case camridb:
	gebrid(i, 33, 40);				/* V2.1c  NG */
	return;

  case zebridb:
	gebrid(i, 25, 32);				/* V2.1c  NG */
	return;

  case gnuridb:
	gebrid(i, 33, 40);				/* V2.1c  NG */
	gebrid(i, 9, 16);
	return;

  case bspawnb:
	gen_sp_nocaptures(i, 23);			/* V3.22  TLi */
	gen_sp_nocaptures(i, 25);
	gen_sp_captures(i, 24, blanc);
	return;

  case spawnb:
	gen_sp_nocaptures(i, 24);			/* V3.22  TLi */
	gen_sp_captures(i, 23, blanc);
	gen_sp_captures(i, 25, blanc);
	return;

  case rhuntb:
	gebrid(i, 2, 2);				/* V3.03  TLi */
	gebrid(i, 7, 8);
	return;

  case bhuntb:
	gebrid(i, 4, 4);				/* V3.03  TLi */
	gebrid(i, 5, 6);
	return;

  case ekingb:
	gebleap(i, 1, 8);				/* V3.05  NG */
	return;

  case okapib:
	gebleap(i, 17, 32);				/* V3.32  TLi */
	return;

  case leap37b:
	gebleap(i, 81, 88);				/* V3.34  TLi */
	return;

  case leap16b:
	gebleap(i, 89, 96);				/* V3.38  NG */
	return;

  case leap24b:
	gebleap(i, 97, 104);				/* V3.42  TLi */
	return;

  case leap35b:
	gebleap(i, 105, 112);				/* V3.42  TLi */
	return;

  case leap15b:
	gebleap(i, 113, 120);				/* V3.46  NG */
	return;

  case leap25b:
	gebleap(i, 121, 128);				/* V3.46  NG */
	return;

  case vizridb:
	gebrid(
      i,	1, 4);				      /* V3.54	NG */
	return;

  case fersridb:
	gebrid(i, 5, 8);				/* V3.54  NG */
	return;

  case bisonb:
	gebleap(i, 25, 40);				/* V3.60  TLi */
	return;

  case elephantb:
	gebrid(i, 1, 16);				/* V3.62  TLi */
	return;

  case leap36b:
	gebleap(i, 129, 136);				/* V3.64  TLi */
	return;

  default:
	gfeerrest(i, p, blanc);				/* V3.14  TLi */
  }
}

void gfeernoir(square i, piece p) {
  testdebut= nbcou;					/* V3.00  NG */
  switch(p) {
  case nn:
	genrid(i, 9, 16);
	return;

  case zn:
	genleap(i, 25, 32);
	return;

  case chn:
	genleap(i, 33, 40);
	return;

  case gin:
	genleap(i, 41, 48);
	return;

  case rccinqn:
	genleap(i, 69, 80);
	return;

  case bun:
	genleap(i, 49, 60);
	return;

  case vizirn:
	genleap(i, 1, 4);
	return;

  case alfiln:
	genleap(i, 65, 68);
	return;

  case fersn:
	genleap(i, 5, 8);
	return;

  case dabn:
	genleap(i, 61, 64);
	return;

  case pbn:
	genpbn(i);
	return;

  case amazn:
	genrid(i, 1, 8);				/* V1.4c  NG */
	genleap(i, 9, 16);
	return;

  case impn:
	genrid(i, 1, 4);				/* V1.4c  NG */
	genleap(i, 9, 16);
	return;

  case princn:
	genrid(i, 5, 8);				/* V1.4c  NG */
	genleap(i, 9, 16);
	return;

  case gnoun:
	genleap(i, 33, 40);				/* V1.4c  NG */
	genleap(i, 9, 16);
	return;

  case antiln:
	genleap(i, 49, 56);				/* V1.5c  NG */
	return;

  case ecurn:
	genleap(i, 61, 68);				/* V1.5c  NG */
	genleap(i, 9, 16);
	return;

  case warann:
	genrid(i, 9, 16);				/* V1.5c  NG */
	genrid(i, 1, 4);				/* V1.5c  NG */
	return;

  case dragonn:
	genpn(i);					/* V1.5c  NG */
	genleap(i, 9, 16);
	return;

  case camridn:
	genrid(i, 33, 40);				/* V2.1c  NG */
	return;

  case zebridn:
	genrid(i, 25, 32);				/* V2.1c  NG */
	return;

  case gnuridn:
	genrid(i, 33, 40);				/* V2.1c  NG */
	genrid(i, 9, 16);
	return;

  case bspawnn:
	gen_sp_nocaptures(i, -23);			/* V3.22  TLi */
	gen_sp_nocaptures(i, -25);
	gen_sp_captures(i, -24, noir);
	return;

  case spawnn:
	gen_sp_nocaptures(i, -24);			/* V3.22  TLi */
	gen_sp_captures(i, -23, noir);
	gen_sp_captures(i, -25, noir);
	return;

  case rhuntn:
	genrid(i, 2, 2);		     /* V3.03  TLi, V3.04  NG */
	genrid(i, 7, 8);
	return;

  case bhuntn:
	genrid(i, 4, 4);		     /* V3.03  TLi, V3.04  NG */
	genrid(i, 5, 6);
	return;

  case ekingn:
	genleap(i, 1, 8);				/* V3.05  NG */
	return;

  case okapin:
	genleap(i, 17, 32);				/* V3.32  TLi */
	return;

  case leap37n:
	genleap(i, 81, 88);				/* V3.34  TLi */
	return;

  case leap16n:
	genleap(i, 89, 96);				/* V3.34  TLi */
	return;

  case leap24n:
	genleap(i, 97, 104);				/* V3.42  TLi */
	return;

  case leap35n:
	genleap(i, 105, 112);				/* V3.42  TLi */
	return;

  case leap15n:
	genleap(i, 113, 120);				/* V3.46  NG */
	return;

  case leap25n:
	genleap(i, 121, 128);				/* V3.46  NG */
	return;

  case vizridn:
	genrid(i, 1, 4);				/* V3.54  NG */
	return;

  case fersridn:
	genrid(i, 5, 8);				/* V3.54  NG */
	return;

  case bisonn:
	genleap(i, 25, 40);				/* V3.60  TLi */
	return;

  case elephantn:
	genrid(i, 1, 16);				/* V3.62  TLi */
	return;

  case leap36n:
	genleap(i, 129, 136);				/* V3.64  TLi */
	return;

  default:
	gfeerrest(i, p, noir);				/* V3.14  TLi */
  }
} /* end of gfeernoir */

void initkiller(void) {
  cdkil = kpilcd[nbply];
  cakil = kpilca[nbply];
  flagkil = false;
}

int compopt(const void *a, const void *b) {
  return  (((otelement *)a)->nbr - ((otelement *)b)->nbr);
}

void finkiller(void) {
  if (optimize) {		    /* V3.02  TLi */
	qsort(ot, otc, sizeof(otelement), compopt);
	nbcou= repere[nbply];
	while (otc) {
      nbcou++;
      otc--;
      cd[nbcou]= ot[otc].d;
      ca[nbcou]= ot[otc].a;
      cp[nbcou]= ot[otc].p;
	}
  }
  else if (flagkil) {
	nbcou++;
	cd[nbcou] = cdkil;
	ca[nbcou] = cakil;
	cp[nbcou] = cpkil;
  }
}

void genrb_cast(void) {					/* V3.55  TLi */
  /* It works only for castling_supported == TRUE
     have a look at funtion verifieposition() in py6.c
  */
  if (dont_generate_castling)
    return;
  if (TSTFLAGMASK(castling_flag[nbply],wh_castlings) > ke1_cancastle
      && e[square_e1]==roib 
      /* then the king on e1 and at least one rook can castle !!
         V3.55  NG */
      && !echecc(blanc))
  {
	if (TSTFLAGMASK(castling_flag[nbply],whk_castling) == whk_castling
        && e[square_h1]==tb	      /* 0-0 */
	    && e[square_f1] == vide && e[square_g1] == vide)
	{
      if (complex_castling_through_flag)  /* V3.80  SE */
      {
        numecoup sic_nbcou= nbcou;
        empile (square_e1, square_f1, square_f1);
        if (nbcou > sic_nbcou)
        {
          boolean ok= (jouecoup() && !echecc(blanc));
          repcoup();
          if (ok)
            empile(square_e1, square_g1, maxsquare+2);
        }
      }
      else
      {
	    e[square_e1]= vide;
	    e[square_f1]= roib;
	    rb= square_f1;
	    if (!echecc(blanc)) {
          e[square_e1]= roib;
          e[square_f1]= vide;
          rb= square_e1;
          empile(square_e1, square_g1, maxsquare+2);		/* V3.55  TLi */
	    }
	    else {
          e[square_e1]= roib;
          e[square_f1]= vide;
          rb= square_e1;
	    }
      }
	}
	if (TSTFLAGMASK(castling_flag[nbply],whq_castling) == whq_castling
        && e[square_a1]==tb 	 /* 0-0-0 */
        && e[square_d1] == vide
        && e[square_c1] == vide
        && e[square_b1] == vide)
	{
      if (complex_castling_through_flag)  /* V3.80  SE */
      {
        numecoup sic_nbcou= nbcou;
        empile (square_e1, square_d1, square_d1);
        if (nbcou > sic_nbcou)
        {
          boolean ok= (jouecoup() && !echecc(blanc));
          repcoup();
          if (ok)
            empile(square_e1, square_c1, maxsquare+3);
        }
      }
      else
      {
	    e[square_e1]= vide;
	    e[square_d1]= roib;
	    rb= square_d1;
	    if (!echecc(blanc)) {
          e[square_e1]= roib;
          e[square_d1]= vide;
          rb= square_e1;
          empile(square_e1, square_c1, maxsquare+3);		/* V3.55  TLi */
	    }
	    else {
          e[square_e1]= roib;
          e[square_d1]= vide;
          rb= square_e1;
	    }
      }
	}
  }
} /* genrb_cast */

void genrb(square i) {
  square	j;
  numvec	k;
  boolean	flag = false;		/* K im Schach ? */
  numecoup	anf, l1, l2;

  VARIABLE_INIT(anf);

  if (CondFlag[whrefl_king] && !calctransmute) {
	/* K im Schach zieht auch */
	piece	*ptrans;

	anf= nbcou;
	calctransmute= true;
	for (ptrans= transmpieces; *ptrans; ptrans++) {
      if (nbpiece[-*ptrans]
	      && (*checkfunctions[*ptrans])(i, -*ptrans, eval_white))
      {
		flag = true;
        current_trans_gen=*ptrans;
		gen_wh_piece(i, *ptrans);
        current_trans_gen=vide;
      }
	}
	calctransmute= false;

	if (flag && nbpiece[orphann]) {
      piece king= e[rb];
      e[rb]= dummyb;
      if (!echecc(blanc)) {
		/* white king checked only by an orphan
		** empowered by the king */
		flag= false;
      }
      e[rb]= king;
	}

	/* K im Schach zieht nur */
	if (CondFlag[whtrans_king] && flag)
      return;
  }

  if (CondFlag[sting])				/* V3.63  NG */
	gerhop(i, 1, 8, blanc);
  for (k= 8; k > 0; k--) {				/* V2.4c  NG */
	j = i + vec[k];
	if (e[j] <= vide)
      empile(i, j, j);
  }
  if (flag) {
	/* testempile nicht nutzbar */
	/* VERIFY: has anf always a propper value??
	 */
	for (l1= anf + 1; l1 <= nbcou; l1++) {
      if (ca[l1] != initsquare) {
		for (l2= l1 + 1; l2 <= nbcou; l2++) {
          if (ca[l1] == ca[l2]) {
			ca[l2]= initsquare;
          }
		}
      }
	}
  }

  /* Now we test castling */				/* V3.35  NG */
  if (castling_supported)
	genrb_cast();
}

void gen_wh_ply(void) {				       /* V3.12  TLi */
  square i, j, z;
  piece p;

  /* Don't try to "optimize" by hand. The double-loop is tested as
     the fastest way to compute (due to compiler-optimizations !)
     V3.14  NG
  */
  z= bas;
  for (i= 8; i > 0; i--, z+= 16) {			/* V2.90  NG */
	for (j= 8; j > 0; j--, z++) {			/* V2.90  NG */
      if ((p = e[z]) != vide) {
		if (TSTFLAG(spec[z], Neutral))
          p = -p;				/* V1.4c  NG */
		if (p > obs)				/* V2.70c TLi */
          gen_wh_piece(z, p);
      }
	}
  }
} /* gen_wh_ply */

void gen_wh_piece_aux(square z, piece p) {		/* V3.46  SE/TLi */
  switch(p) {
  case roib:
	genrb(z);
	break;

  case pb:
	genpb(z);
	break;

  case cb:
	gebleap(z, 9, 16);
	break;

  case tb:
	gebrid(z, 1, 4);				/* V2.60  NG */
	break;

  case db:
	gebrid(z, 1, 8);				/* V2.60  NG */
	break;

  case fb:
	gebrid(z, 5, 8);				/* V2.60  NG */
	break;

  default:
	gfeerblanc(z, p);
	break;
  }
}

static void orig_gen_wh_piece(square z, piece p) { /* V3.71 TM */
  piece pp;


  if (flag_madrasi) {					/* V3.60  TLi */
	if (!libre(z, true)) {				/* V3.44  TLi */
      return;
	}
  }

  if (TSTFLAG(PieSpExFlags,Paralyse)) {	       /* V2.90c  TLi */
	if (paralysiert(z)) {
      return;
	}
  }

  if (anymars||anyantimars) {				     /* V3.46  SE/TLi */
	square mren;
	Flags psp;

	if (is_phantomchess) {				/* V3.47  NG */
      numecoup	    anf1, anf2, l1, l2;
      anf1= nbcou;
      /* generate standard moves first */
      flagactive= false;
      flagpassive= false;
      flagcapture= false;

      gen_wh_piece_aux(z, p);

      /* Kings normally don't move from their rebirth-square */
      if (p == e[rb] && !rex_phan) {
		return;
      }
      /* generate moves from rebirth square */
      flagactive= true;
      mren=(*marsrenai)(p,psp=spec[z],z,initsquare,initsquare,noir);
      /* if rebirth square is where the piece stands,
         we've already generated all the relevant moves.
      */
      if (mren == z) {
		return;
      }
      if (e[mren] == vide) {
		anf2= nbcou;
		pp=e[z];		 /* Mars/Neutral bug V3.50 SE */
		e[z]= vide;
		spec[z]= EmptySpec;
		spec[mren]= psp;
		e[mren]= p;
		marsid= z;

		gen_wh_piece_aux(mren, p);

		e[mren]= vide;
		spec[z]= psp;
		e[z]= pp;
		flagactive= false;
		/* Unfortunately we have to check for
		   duplicate generated moves now.
		   there's only ONE duplicate per arrival field
		   possible !
        */
		for (l1= anf1 + 1; l1 <= anf2; l1++) {
          for (l2= anf2 + 1; l2 <= nbcou; l2++) {
			if (ca[l1] == ca[l2]) {
              ca[l2]= initsquare;
              break;  /* remember: ONE duplicate ! */
			}
          }
		}
      }
	}
	else {
      /* generate noncapturing moves first */
      flagpassive= true;
      flagcapture= false;

      gen_wh_piece_aux(z, p);

      /* generate capturing moves now */
      flagpassive= false;
      flagcapture= true;
      more_ren=0;
      do {	  /* V3.50 SE Echecs Plus */
		mren=(*marsrenai)(p,psp=spec[z],z,initsquare,initsquare,noir);
		if ((mren == z) || (e[mren] == vide)) {
          pp= e[z];		/* Mars/Neutral bug V3.50 SE */
          e[z]= vide;
          spec[z]= EmptySpec;
          spec[mren]= psp;
          e[mren]= p;
          marsid= z;

          gen_wh_piece_aux(mren, p);

          e[mren]= vide;
          spec[z]= psp;
          e[z]= pp;
		}
      } while (more_ren);
      flagcapture= false;
	}
  }
  else {
	gen_wh_piece_aux(z, p);
  }

  if (CondFlag[messigny] && !(rb == z && rex_mess_ex)) {
    /* V3.55  TLi */
	square *bnp;
	for (bnp= boardnum; *bnp; bnp++) {
      if (e[*bnp] == - p) {
		empile(z,*bnp,maxsquare+1);
      }
	}
  }
} /* orig_gen_wh_piece */

void singleboxtype3_gen_wh_piece(square z, piece p) {	 /* V3.71 TM */
  numecoup save_nbcou = nbcou;
  unsigned int latent_prom = 0;
  square sq;
  for (sq = next_latent_pawn(initsquare,blanc);
       sq!=initsquare;
       sq = next_latent_pawn(sq,blanc))
  {
	piece pprom;
	for (pprom = next_singlebox_prom(vide,blanc);
         pprom!=vide;
         pprom = next_singlebox_prom(pprom,blanc))
	{
      numecoup save_nbcou = nbcou;
      ++latent_prom;
      e[sq] = pprom;
      orig_gen_wh_piece(z, sq==z ? pprom : p);
      e[sq] = pb;
      for (++save_nbcou; save_nbcou<=nbcou; ++save_nbcou) {
		sb3[save_nbcou].where = sq;
		sb3[save_nbcou].what = pprom;
      }
	}
  }

  if (latent_prom==0) {
	orig_gen_wh_piece(z,p);

	for (++save_nbcou; save_nbcou<=nbcou; ++save_nbcou) {
      sb3[save_nbcou].where = initsquare;
      sb3[save_nbcou].what = vide;
	}
  }
}

void (*gen_wh_piece)(square z, piece p)
  = &orig_gen_wh_piece; /* V3.71 TM */


/************	  V3.0	TLi	Anfang	   ****************************/

/* V3.33  TLi */
#define ColourSpec Flags
#define OppsiteColour(s)  (s == White ? Black : White)

boolean IsABattery(
  square	KingSquare,
  square	FrontSquare,
  numvec	Direction,
  ColourSpec	ColourMovingPiece,
  piece	BackPiece1,
  piece	BackPiece2)
{
  square sq;
  piece p;

  /* is the line between king and front piece empty? */
  EndOfLine(FrontSquare, Direction, sq);
  if (sq == KingSquare) {
	/* the line is empty, but is there really an appropriate back
	** battery piece? */
	EndOfLine(FrontSquare, -Direction, sq);
	p= e[sq];
	if (p < vide)
      p= -p;
	if (   (p == BackPiece1 || p == BackPiece2)
           && TSTFLAG(spec[sq], ColourMovingPiece))
	{
      /* So, it is a battery. */
      return true;
	}
  }
  return false;
} /* IsABattery */

void GenMatingPawn(
  square	z,
  square	king,
  ColourSpec	ColourMovingPiece)
{
  boolean Battery = false;
  square j;
  numvec k;

  if ((k= CheckDirBishop[king-z]) != 0) {
	Battery=
	  IsABattery(king, z, k, ColourMovingPiece, Bishop, Queen);
  }
  else if ((k= CheckDirRook[king-z]) != 0) {
	Battery= IsABattery(king, z, k, ColourMovingPiece, Rook, Queen);
  }
  /* if the pawn is not the front piece of a battery reset k,
     otherwise normalise it to be positiv. This is necessary to
     avoid moves along the battery line subsequently.
  */
  if (Battery) {
	if (k < 0)
      k= -k;
  }
  else
	k= 0;

  if (ColourMovingPiece == White) {
	if (z < bas + 24) {
      /* Pawn on first rank */
      return;
	}
	else {
      /* not first rank */
      /* ep captures */
      if ((j = ep[nbply-1]) != initsquare
          && trait[nbply-1] != trait[nbply])
      {
		if (z+25 == j || z+23 == j)
          empile(z, j, ca[repere[nbply]]);
      }

      /* single step */
      if (k != 24) {
		/* suppress moves along a battery line */
		j= z+24;
		if (e[j] == vide) {
          if (   Battery
                 || j+23 == king
                 || j+25 == king
                 || (PromSq(blanc, j)
                     && (CheckDirQueen[king-j]
                         || CheckDirKnight[king-j])))
          {
			empile(z, j, j);
          }

          /* double step */
          if (z < bas+32) {
			j+= 24;
			if (  e[j] == vide
                  && (Battery
                      || j+23 == king
                      || j+25 == king))
			{
              empile(z, j, j);
			}
          }
		}
      }

      /* capture 23 */
      j= z+23;
      if (e[j] != vide && TSTFLAG(spec[j], Black)) {
		if ( Battery
             || j+23 == king
             || j+25 == king
             || (PromSq(blanc, j)
                 && (CheckDirQueen[king-j]
                     || CheckDirKnight[king-j])))
		{
          empile(z, j, j);
		}
      }
      /* capture 25 */
      j= z+25;
      if (e[j] != vide && TSTFLAG(spec[j], Black)) {
		if ( Battery
             || j+23 == king
             || j+25 == king
             || (PromSq(blanc, j)
                 && (CheckDirQueen[king-j]
                     || CheckDirKnight[king-j])))
		{
          empile(z, j, j);
		}
      }
	}
  }
  else {
	if (z > haut - 24) {
      /* Pawn on last rank */
      return;
	}
	/* not last rank */
	/* ep captures */
	if ((j = ep[nbply-1]) != initsquare
	    && trait[nbply-1] != trait[nbply])
	{
      if (z-25 == j || z-23 == j)
		empile(z, j, ca[repere[nbply]]);

	}

	/* single step */
	if (k != 24) {	/* suppress moves along a battery line */
      j= z-24;
      if (e[j] == vide) {
		if (Battery
            || j-23 == king
            || j-25 == king
            || (PromSq(noir, j)
                && (CheckDirQueen[king-j]
                    || CheckDirKnight[king-j])))
		{
          empile(z, j, j);
		}
		/* double step */
		if (z > haut-32) {
          j-= 24;
          if (e[j] == vide
		      && (Battery || j-23 == king || j-25 == king))
          {
			empile(z, j, j);
          }
		}
      }
	}

	/* capture 23 */
	j= z-23;
	if (e[j] != vide && TSTFLAG(spec[j], White)) {
      if (Battery
	      || j-23 == king
	      || j-25 == king
	      || (PromSq(noir, j)
              && (CheckDirQueen[king-j]
                  || CheckDirKnight[king-j])))
      {
		empile(z, j, j);
      }
	}

	/* capture 25 */
	j= z-25;
	if (e[j] != vide && TSTFLAG(spec[j], White)) {
      if (Battery
          || j-23 == king
          || j-25 == king
          || (PromSq(noir, j)
              && (CheckDirQueen[king-j]
                  || CheckDirKnight[king-j])))
      {
		empile(z, j, j);
      }
	}
  }
} /* GenMatingPawn */

void GenMatingKing(
  square	z,
  square	king,
  ColourSpec	ColourMovingPiece)
{
  square	sq;
  numvec	k, k2;
  boolean	Generate = false;
  ColourSpec	ColourCapturedPiece = OppsiteColour(ColourMovingPiece);

  if (rb == rn) {
	/* neutral kings */
	for (k2= 1; k2 <= 8; k2++) {
      sq= z + vec[k2];
      /* they must capture to mate the opponent */
      if (e[sq] != vide && TSTFLAG(spec[sq], ColourCapturedPiece))
		empile(z, sq, sq);
	}
  }
  else {
	/* check if the king is the front piece of a battery
	   that can fire */
	if ((k = CheckDirBishop[king-z]) != 0) {
      Generate=
        IsABattery(king, z, k, ColourMovingPiece, Bishop, Queen);
	}
	else if ((k = CheckDirRook[king-z]) != 0) {
      Generate=
        IsABattery(king, z, k, ColourMovingPiece, Rook, Queen);
	}

	if (Generate) {
      for (k2= 1; k2 <= 8; k2++) {
		/* prevent the king from moving along the battery line*/
		if (k2 == k || k2 == -k)
          continue;
		sq= z + vec[k2];
		if ( (e[sq] == vide
		      || TSTFLAG(spec[sq], ColourCapturedPiece))
             && move_diff_code[abs(king-sq)] > 2)
		{
          empile(z, sq, sq);
		}
      }
	}
	if (  (CondFlag[ColourCapturedPiece == White
                    ? whiteedge : blackedge])
          || DoubleMate)
	{
      for (k2= 1; k2 <= 8; k2++) {
		sq= z + vec[k2];
		if (  (e[sq] == vide
		       || TSTFLAG(spec[sq], ColourCapturedPiece))
              && move_diff_code[abs(king-sq)] < 3)
		{
          empile(z, sq, sq);
		}
      }
	}
  }

  /* castling */
  if (castling_supported) {				/* V3.53  NG */
	if (ColourMovingPiece == White) {
      /* white to play */
      genrb_cast();    /* V3.55  TLi */
	}
	else {
      /* black to play */
      genrn_cast();    /* V3.55  TLi */
	}
  } /* castling_supported ... */
}

void GenMatingKnight(
  square	z,
  square	king,
  ColourSpec	ColourMovingPiece)
{
  square	sq;
  numvec	k;
  boolean	Generate = false;
  ColourSpec	ColourCapturedPiece = OppsiteColour(ColourMovingPiece);

  /* check if the knight is the front piece of a battery that can
     fire
  */
  if ((k = CheckDirBishop[king-z]) != 0) {
	Generate=
	  IsABattery(king, z, k, ColourMovingPiece, Bishop, Queen);
  }
  else if ((k = CheckDirRook[king-z]) != 0) {
	Generate=
	  IsABattery(king, z, k, ColourMovingPiece, Rook, Queen);
  }

  if (Generate
      || (SquareCol(z) == SquareCol(king)
          && move_diff_code[k= abs(king-z)] < 21
          && move_diff_code[k] != 8))
  {
	for (k= 9; k <= 16; k++) {
      sq= z + vec[k];
      if (e[sq] == vide
          || TSTFLAG(spec[sq], ColourCapturedPiece))
      {
		if (Generate || CheckDirKnight[sq-king] != 0)
          empile(z, sq, sq);
      }
	}
  }
}

void GenMatingRook(
  square	z,
  square	king,
  ColourSpec	ColourMovingPiece)
{
  square	sq, sq2;
  numvec	k, k2;
  ColourSpec	ColourCapturedPiece = OppsiteColour(ColourMovingPiece);

  /* check if the rook is the front piece of a battery that can fire
   */
  k= CheckDirBishop[king-z];
  if (k != 0
      && IsABattery(king, z, k, ColourMovingPiece, Bishop, Queen))
  {
	for (k= 1; k <= 4; k++) {
      sq= z + vec[k];
      while (e[sq] == vide) {
		empile(z, sq, sq);
		sq+= vec[k];
      }
      if (TSTFLAG(spec[sq], ColourCapturedPiece))
		empile(z, sq, sq);
	}
  }
  else {
	short OriginalDistance = move_diff_code[abs(z-king)];

	if ((k2 = CheckDirRook[king-z])) {
      /* the rook is already on a line with the king */
      EndOfLine(z, k2, sq);
      /* We are at the end of the line */
      if (TSTFLAG(spec[sq], ColourCapturedPiece)) {
		EndOfLine(sq, k2, sq2);
		if (sq2 == king)
          empile(z, sq, sq);
      }

      /* it makes no sense to move away -- except for gridchess */
      if (CondFlag[gridchess] && OriginalDistance < 26) {
		sq= z-k2;
		while (e[sq] == vide) {
          empile(z, sq, sq);
          sq-= k2;
		}
		if (TSTFLAG(spec[sq], ColourCapturedPiece))
          empile(z, sq, sq);
      }
	}
	else {
      for (k= 1; k <= 4; k++) {
		sq= z + vec[k];
		if (e[sq] == obs)			/* V3.45  NG */
          continue;
		if (move_diff_code[abs(sq-king)] < OriginalDistance) {
          /* The rook must move closer to the king! */
          while (!(k2= CheckDirRook[king-sq])
                 && e[sq] == vide)
          {
			sq+= vec[k];
          }
          /* We are at the end of the line or in checking
             distance
          */
          if (k2 == 0)
			continue;
          if (e[sq] == vide
		      || TSTFLAG(spec[sq], ColourCapturedPiece))
          {
			EndOfLine(sq, k2, sq2);
			if (sq2 == king)
              empile(z, sq, sq);
          }
		}
      }
	}
  }
}

void GenMatingQueen(
  square	z,
  square	king,
  ColourSpec	ColourMovingPiece)
{
  square sq, sq2;
  numvec  k, k2;
  ColourSpec ColourCapturedPiece = OppsiteColour(ColourMovingPiece);

  for (k= 1; k <= 8; k++) {
	sq= z + vec[k];
	while (e[sq] == vide) {
      k2= CheckDirQueen[king-sq];
      if (k2) {
		EndOfLine(sq, k2, sq2);
		if (sq2 == king)
          empile(z, sq, sq);
      }
      sq+= vec[k];
	}
	if (TSTFLAG(spec[sq], ColourCapturedPiece)) {
      k2= CheckDirQueen[king-sq];
      if (k2) {
		EndOfLine(sq, k2, sq2);
		if (sq2 == king)
          empile(z, sq, sq);
      }
	}
  }
}

void GenMatingBishop(
  square	z,
  square	king,
  ColourSpec	ColourMovingPiece)
{
  square	sq, sq2;
  numvec	k, k2;
  ColourSpec	ColourCapturedPiece = OppsiteColour(ColourMovingPiece);

  /* check if the bishop is the front piece of a battery that can
     fire
  */
  if (((k = CheckDirRook[king-z]) != 0)
      && IsABattery(king, z, k, ColourMovingPiece, Rook, Queen))
  {
	for (k= 5; k <= 8; k++) {
      sq= z + vec[k];
      while (e[sq] == vide) {
		empile(z, sq, sq);
		sq+= vec[k];
      }
      if (TSTFLAG(spec[sq], ColourCapturedPiece))
		empile(z, sq, sq);
	}
  }
  else if ((SquareCol(z) == SquareCol(king))) {
	short OriginalDistance = move_diff_code[abs(z-king)];

	if ((k2 = CheckDirBishop[king-z])) {
      /* the bishop is already on a line with the king */
      EndOfLine(z, k2, sq);
      /* We are at the end of the line */
      if (TSTFLAG(spec[sq], ColourCapturedPiece)) {
		EndOfLine(sq, k2, sq2);
		if (sq2 == king)
          empile(z, sq, sq);
      }
      /* it makes no sense to move away -- except for gridchess */
      if (CondFlag[gridchess] && OriginalDistance < 26) {
		sq= z-k2;
		while (e[sq] == vide) {
          empile(z, sq, sq);
          sq-= k2;
		}
		if (TSTFLAG(spec[sq], ColourCapturedPiece))
          empile(z, sq, sq);
      }
	}
	else {
      for (k= 5; k <= 8; k++) {
		sq= z + vec[k];
		if (e[sq] == obs)	     /* V3.45  NG */
          continue;
		if (move_diff_code[abs(sq-king)] < OriginalDistance) {
          /* The bishop must move closer to the king! */
          while (!(k2= CheckDirBishop[king-sq])
                 && e[sq] == vide)
          {
			sq+= vec[k];
          }
          /* We are at the end of the line or in checking
             distance */
          if (k2 == 0)
			continue;
          if ( e[sq] == vide
               || TSTFLAG(spec[sq], ColourCapturedPiece))
          {
			EndOfLine(sq, k2, sq2);
			if (sq2 == king)
              empile(z, sq, sq);
          }
		}
      }
	}
  }
} /* GenMatingBishop */

void GenMatingMove(couleur camp) {			/* V3.33  TLi */
  if (totalortho) {
	ColourSpec ColourMovingPiece = camp == blanc ? White : Black;
	square	   i, j, z, OpponentsKing = camp == blanc ? rn : rb;
	piece	   p;

	nextply();
	trait[nbply]= camp;
	/* flagminmax= false;	    V3.44  TLi */
	/* flag_minmax[nbply]= false;	   V3.44  TLi */
	initkiller();

	FlagGenMatingMove= TSTFLAG(PieSpExFlags, Neutral);

	/* Don't try to "optimize" by hand. The double-loop is tested
	   as the fastest way to compute (due to
	   compiler-optimizations!)			    V3.14  NG */

	z= bas;
	for (i= 8; i > 0; i--, z+= 16) {		/* V2.90  NG */
      for (j= 8; j > 0; j--, z++) {		    /* V2.90  NG */
		p= e[z];
		if (p != vide && TSTFLAG(spec[z], ColourMovingPiece)) {
          switch(abs(p)) {
          case King:
			GenMatingKing(z,
                          OpponentsKing, ColourMovingPiece);
			break;
          case Pawn:
			GenMatingPawn(z,
                          OpponentsKing, ColourMovingPiece);
			break;
          case Knight:
			GenMatingKnight(z,
                            OpponentsKing, ColourMovingPiece);
			break;
          case Rook:
			GenMatingRook(z,
                          OpponentsKing, ColourMovingPiece);
			break;
          case Queen:
			GenMatingQueen(z,
                           OpponentsKing, ColourMovingPiece);
			break;
          case Bishop:
			GenMatingBishop(z,
                            OpponentsKing, ColourMovingPiece);
			break;
          }
		}
      }
	}
	finkiller();
  }
  else {
	if (FlagMoveOrientatedStip) {
      if (stipulation == stip_ep) {
		if (  ep[nbply] == initsquare
              && ep2[nbply] == initsquare)
		{
          nextply();
          return;
		}
      }
      else if (stipulation == stip_castling) {
		if (camp == blanc
		    ? TSTFLAGMASK(castling_flag[nbply],wh_castlings)<=ke1_cancastle
		    : TSTFLAGMASK(castling_flag[nbply],bl_castlings)<=ke8_cancastle)
		{
          nextply();
          return;
		}
      }
      /* FlagGenMatingMove= True; */
      FlagGenMatingMove= ! (camp == blanc
                            ? flagwhitemummer
                            : flagblackmummer);
	}
	genmove(camp);
  }
  FlagGenMatingMove= False;
} /* GenMatingMove */

void gorph(square i, couleur camp) {			/* V3.14  NG */
  piece	*porph;
  numecoup	anf, l1, l2;

  anf= nbcou;
  for (porph= orphanpieces; *porph; porph++) {
	if (nbpiece[*porph] || nbpiece[-*porph]) {
      if (camp == blanc) {
		if (ooorphancheck(i, -*porph, orphann, eval_white))
          gen_wh_piece(i, *porph);
      }
      else { /* camp == noir */
		if (ooorphancheck(i, *porph, orphanb, eval_black))
          gen_bl_piece(i, -*porph);
      }
	}
  }
  for (l1= anf + 1; l1 <= nbcou; l1++) {
	if (ca[l1] != initsquare) {
      for (l2= l1 + 1; l2 <= nbcou; l2++) {
		if (ca[l1] == ca[l2]) {
          ca[l2]= initsquare;
		}
      }
	}
  }
}

void gfriend(square i, couleur camp) {		  /* V3.65  TLi */
  piece	*pfr;
  numecoup	anf, l1, l2;

  anf= nbcou;
  for (pfr= orphanpieces; *pfr; pfr++) {
	if (nbpiece[*pfr]) {
      if (camp == blanc) {
		if (fffriendcheck(i, *pfr, friendb, eval_white)) {
          gen_wh_piece(i, *pfr);
		}
      }
      else  { /* camp == noir */
		if (fffriendcheck(i, -*pfr, friendn, eval_black)) {
          gen_bl_piece(i, -*pfr);
		}
      }
	}
  }
  for (l1= anf + 1; l1 <= nbcou; l1++) {
	if (ca[l1] != initsquare) {
      for (l2= l1 + 1; l2 <= nbcou; l2++) {
		if (ca[l1] == ca[l2]) {
          ca[l2]= initsquare;
		}
      }
	}
  }
}


void gedgeh(square i, couleur camp) {			/* V3.14  TLi */
  square j;
  numvec k;

  for (k= 8; k > 0; k--) {	    /* V3.00  NG */
	j= i;
	while (e[j+= vec[k]] == vide) {
      if (NoEdge(j) != NoEdge(i))
		empile(i, j, j);
	}
	if (rightcolor(e[j], camp)) {
      if (NoEdge(j) != NoEdge(i))
		empile(i, j, j);
	}
  }
}

/**************     V3.0  TLi	  Ende	   ****************************/

/**************     V3.12  TLi	 begin	   ****************************/
void geskylchar(square id, square ia, square ipp, couleur camp) {
  if (e[ia] == vide) {
	if (e[ipp] == vide)
      empile(id, ia, ia);
	else if (rightcolor(e[ipp], camp))
      empile(id, ia, ipp);
  }
}

void geskylla(square i, couleur camp) {
  geskylchar(i, i + 26, i +  1, camp);
  geskylchar(i, i + 49, i + 24, camp);
  geskylchar(i, i + 47, i + 24, camp);
  geskylchar(i, i + 22, i -  1, camp);
  geskylchar(i, i - 26, i -  1, camp);
  geskylchar(i, i - 49, i - 24, camp);
  geskylchar(i, i - 47, i - 24, camp);
  geskylchar(i, i - 22, i +  1, camp);
}

void gecharybdis(square i, couleur camp) {
  geskylchar(i, i + 26, i + 25, camp);
  geskylchar(i, i + 49, i + 25, camp);
  geskylchar(i, i + 47, i + 23, camp);
  geskylchar(i, i + 22, i + 23, camp);
  geskylchar(i, i - 26, i - 25, camp);
  geskylchar(i, i - 49, i - 25, camp);
  geskylchar(i, i - 47, i - 23, camp);
  geskylchar(i, i - 22, i - 23, camp);
} /* gecharybdis */

/**************     V3.12  TLi	  end	   ****************************/

/***********************************************************************
 **								       **
 **		  generating functions for pawns		       **
 **								       **
 ***********************************************************************/

/* Two auxiliary function for generating pawn moves */
void	gen_p_captures(square i, square j, couleur camp) {
  /* generates move of a pawn of colour camp on i capturing a
     piece on j
  */

  if (rightcolor(e[j], camp)) {
	/* normal capture */
	empile(i, j, j);
  }
  else {
	/* ep capture */
	if ( (abs(e[i]) != Orphan)	 /* orphans cannot capture ep */
         && ((j == ep[nbply - 1]) || (j == ep2[nbply - 1]))
         /* a pawn has just done a critical move */
         && (trait[nbply - 1] != camp))	 /* the opp. moved before */
	{
      if (nbply == 2) {	 /* ep.-key  standard pawn  V3.37  NG */
		if (camp == blanc)
          ca[repere[2]]= j - 24;
		else
          ca[repere[2]]= j + 24;
      }
      if (rightcolor(e[ca[repere[nbply]]], camp)) {
		/* the pawn has the right color */
		empile(i, j, ca[repere[nbply]]);
      }
	}
  }
} /* end gen_p_captures */

void gen_p_nocaptures(square i, numvec dir, short steps)
{
  /* generates moves of a pawn in direction dir where steps single
     steps are possible.
  */
  square  j= i;

  while (steps-- && (e[j+= dir] == vide) && empile(i, j, j))
	;
}

/****************************  white pawn  ****************************/
void genpb(square i) {
  if (i < bas + 24) {
	/* pawn on first rank */
	if (CondFlag[parrain]
		|| CondFlag[einstein]
		|| abs(e[i]) == orphanb)
	{
      gen_p_captures(i, i + 23, blanc);
      gen_p_captures(i, i + 25, blanc);
      /* triple or single step? */
      gen_p_nocaptures(i, 24, CondFlag[einstein] ? 3 : 1);
	}
	else {
      return;
	}
  }
  else {
	/* not first rank */
	if ( sbtype1 /* V3.71 TM */
         && PromSq(blanc,i+24)
         && next_singlebox_prom(vide,blanc)==vide)
	{
      return;
	}
	gen_p_captures(i, i + 23, blanc);
	gen_p_captures(i, i + 25, blanc);
	/* double or single step? */
	gen_p_nocaptures(i, 24, (i < bas + 32) ? 2 : 1);
  }
} /* end of genpb */

/****************************  black pawn  ****************************/
void genpn(square i) {
  if (i > haut - 24) {
	/* pawn on last rank */
	if ( CondFlag[parrain]
         || CondFlag[einstein]
         || abs(e[i]) == orphanb)
	{
      gen_p_captures(i, i - 23, noir);
      gen_p_captures(i, i - 25, noir);
      /* triple or single step? */
      gen_p_nocaptures(i, -24, CondFlag[einstein] ? 3 : 1);
	}
	else {
      return;
	}
  }
  else {
	/* not last rank */
	if (   sbtype1 /* V3.71 TM */
           && PromSq(noir,i-24)
           && next_singlebox_prom(vide,noir)==vide)
	{
      return;
	}
	gen_p_captures(i, i - 23, noir);
	gen_p_captures(i, i - 25, noir);
	/* double or single step? */
	gen_p_nocaptures(i, -24, (i > haut - 32) ? 2 : 1);
  }
}

/************************  white berolina pawn	***********************/
void genpbb(square i) {
  if (i < bas + 24) {
	/* pawn on first rank */
	if ( CondFlag[parrain]
         || CondFlag[einstein]
         || abs(e[i]) == orphanb)
	{
      gen_p_captures(i, i + 24, blanc);
      /* triple or single step? */
      gen_p_nocaptures(i, 23, CondFlag[einstein] ? 3 : 1);
      gen_p_nocaptures(i, 25, CondFlag[einstein] ? 3 : 1);
	}
	else {
      return;
	}
  }
  else {
	/* not first rank */
	gen_p_captures(i, i + 24, blanc);
	/* double or single step? */
	gen_p_nocaptures(i, 23, (i < bas + 32) ? 2 : 1);
	gen_p_nocaptures(i, 25, (i < bas + 32) ? 2 : 1);
  }
}

/************************  black berolina pawn	***********************/
void genpbn(square i) {
  if (i > haut - 24) {
	/* pawn on last rank */
	if ( CondFlag[parrain]
         || CondFlag[einstein]
         || abs(e[i]) == orphanb)
	{
      gen_p_captures(i, i - 24, noir);
      /* triple or single step? */
      gen_p_nocaptures(i, -23, CondFlag[einstein] ? 3 : 1);
      gen_p_nocaptures(i, -25, CondFlag[einstein] ? 3 : 1);
	}
	else {
      return;
	}
  }
  else {
	/* not last rank */
	gen_p_captures(i, i - 24, noir);
	/* double or single step? */
	gen_p_nocaptures(i, -23, (i > haut - 32) ? 2 : 1);
	gen_p_nocaptures(i, -25, (i > haut - 32) ? 2 : 1);
  }
}


void gorix(square i, couleur camp) {			/* V3.44  NG */
  /* Orix */
  numvec  k;
  piece   p1;
  square  j, j1;

  for (k= 8; k > 0; k--) {	    /* 0,2; 0,4; 0,6; 2,2; 4,4; 6,6; */
	finligne(i, vec[k], p1, j);
	if (p1 != obs) {
      finligne(j, vec[k], p1, j1);
      if (   abs(j1 - j) > abs(j - i)
             && hopimcheck(i,j+j-i,j,vec[k]))
      {
        empile(i, j + j - i, j + j - i);
      }
      else if (	abs(j1 - j) == abs(j - i)
                && rightcolor(p1, camp)
                && hopimcheck(i,j1,j,vec[k]))
      {
        empile(i, j1, j1);
      }
	}
  }
}

