/********************* MODIFICATIONS to py5.c **************************
 **
 ** Date       Who  What
 **
 ** 2003/11/24 TLi  castling + neutrals bug fixed
 **
 ** 2004/02/05 SE   New conditions : MAFF, OWU (invented M.Brada)
 **                 Mate with A Free Field, (Mate with) One White Unit (in K field)
 **
 ** 2004/02/06 SE   New conditions : Oscillating Kings (invented A.Bell)
 **				   Ks swapped after each W &/or Bl move; TypeB can't self-check before swap
 **
 ** 2004/03/05 SE   New condition : Antimars (and variants) (invented S.Emmerson)
 **                 Pieces reborn to move, capture normally
 **
 ** 2004/03/19 SE   New condition: Supertransmutingkings (?inventor)
 **
 ** 2004/03/20 SE   New condition: AntiSuperCirce
 **
 ** 2004/04/22 SE   Castling with Imitators
 **
 ** 2004/04/23 SE   Oscillating Ks TypeC, also allowed A/B/C different for white/black
 **
 ** 2004/05/01 ThM  Bugfix: Multiple imitator promotions
 **
 ** 2004/05/01 SE   Bugfixes: OscKs + Castling, Antisupercirce Cheylan, Republican
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
#include <stdlib.h>		/* H.D. 10.02.93 prototype fuer exit */

#ifdef DOS			/* V2.90  ElB, TLi, NG */
#	ifdef GCC
#		include <pc.h>
#	else
#		include <bios.h>
#	endif /* GCC */
#endif /* DOS */

#ifdef OS2			/* V3.11  NG */
#	ifdef GCC
#	endif /* GCC */
#endif /* OS2 */

#include "py.h"
#include "pyproc.h"
#include "pyhash.h"
#include "pydata.h"
#include "pymsg.h"

piece	linechampiece(piece p, square sq) {	/* V3.64  TLi */
  piece pja= p;
  if (CondFlag[leofamily]) {
    switch (abs(p)) {
    case leob: case maob: case vaob: case paob:
      switch(sq%24) {
      case 8:  case 15:   pja= paob;	break;
      case 9:  case 14:   pja= maob;	break;
      case 10: case 13:   pja= vaob;	break;
      case 11:	    pja= leob;	break;
      }
    }
    return (pja != p && p < vide) ? - pja : pja;
  } else
	if (CondFlag[cavaliermajeur]) {		    /* V3.01  NG */
      switch (abs(p)) {
      case db: case nb: case fb: case tb:
        switch(sq%24) {
        case 8:  case 15:   pja= tb;  break;
        case 9:  case 14:   pja= nb;  break;
        case 10: case 13:   pja= fb;  break;
        case 11:	    pja= db;  break;
        }
      }
      return (pja != p && p < vide) ? - pja : pja;
	} else {
      switch (abs(p)) {
      case db: case cb: case fb: case tb:
        switch(sq%24) {
        case 8:  case 15:   pja= tb;  break;
        case 9:  case 14:   pja= cb;  break;
        case 10: case 13:   pja= fb;  break;
        case 11:	    pja= db;  break;
        }
      }
      return (pja != p && p < vide) ? - pja : pja;
	}
} /* linechampiece */


piece	champiece(piece p)
{
  /* function realisiert Figurenwechsel bei Chamaeleoncirce */
  if (CondFlag[leofamily]) {		    /* V3.01  NG */
    switch (p) {
    case leob:	return maob;
    case leon:	return maon;
    case maob:	return vaob;
    case maon:	return vaon;
    case vaob:	return paob;
    case vaon:	return paon;
    case paob:	return leob;
    case paon:	return leon;
    }
  } else
	if (CondFlag[cavaliermajeur]) {		    /* V3.01  NG */
      switch (p) {
      case db:	return nb;
      case dn:	return nn;
      case nb:	return fb;
      case nn:	return fn;
      case fb:	return tb;
      case fn:	return tn;
      case tb:	return db;
      case tn:	return dn;
      }
	} else {
      switch (p) {
      case db:	return cb;
      case dn:	return cn;
      case cb:	return fb;
      case cn:	return fn;
      case fb:	return tb;
      case fn:	return tn;
      case tb:	return db;
      case tn:	return dn;
      }
	}
  return p;
}

piece	norskpiece(piece p)
{
  /* function realisiert Figurenwechsel bei NorskSjakk */
  if (CondFlag[leofamily]) {			/* V3.01  NG */
	switch (p) {
    case leob:	return maob;
    case leon:	return maon;
    case maob:	return leob;
    case maon:	return leon;
    case vaob:	return paob;
    case vaon:	return paon;
    case paob:	return vaob;
    case paon:	return vaon;
	}
  } else
    if (CondFlag[cavaliermajeur]) {		/* V3.01  NG */
      switch (p) {
      case db:	return nb;
      case dn:	return nn;
      case nb:	return db;
      case nn:	return dn;
      case fb:	return tb;
      case fn:	return tn;
      case tb:	return fb;
      case tn:	return fn;
      }
    } else {
      switch (p) {
      case db:	return cb;
      case dn:	return cn;
      case cb:	return db;
      case cn:	return dn;
      case fb:	return tb;
      case fn:	return tn;
      case tb:	return fb;
      case tn:	return fn;
      }
    }
  return p;
} /* norskpiece */

piece	dec_einstein(piece p)
{
  /* function realisiert Figurenwechsel bei Einsteinschach abwaerts */
  switch (p) {
  case db:	return tb;
  case dn:	return tn;
  case tb:	return fb;
  case tn:	return fn;
  case fb:	return cb;
  case fn:	return cn;
  case cb:	return pb;
  case cn:	return pn;
  }
  return p;
} /* end of dec_einstein */

piece	inc_einstein(piece p)
{
  /* function realisiert Figurenwechsel bei Einsteinschach aufwaerts */
  switch (p) {
  case pb:	return cb;
  case pn:	return cn;
  case cb:	return fb;
  case cn:	return fn;
  case fb:	return tb;
  case fn:	return tn;
  case tb:	return db;
  case tn:	return dn;
  }
  return p;
} /* end of inc_einstein */

/**********  V3.1  TLi	begin  *********/
/* all renai-functions have the following parameters:
   p:	piece captured
   pspec:	specifications of the piece captured
   j:	square where the capture takes place
   i:	square whre the capturing piece just came from
   camp:	colour of the moving=capturing side
*/
#ifdef DOS
#	pragma warn -par
#endif

square renplus(piece p, Flags pspec, square j, square i, square ia, couleur camp) {
  /* V3.50 SE Echecs plus */

  if (j==square_d4 || j==square_e4 || j==square_d5 || j==square_e5) {
	square k;
	switch (more_ren) {
    case 0 : k= square_d4; break;
    case 1 : k= square_e4; break;
    case 2 : k= square_d5; break;
    case 3 : k= square_e5; break;
    default: k= -1;	/* avoid compiler warning, ElB. */
	}
	more_ren++;
	if (more_ren == 4)
      more_ren= 0;

	return k;
  }
  return j;
}

square renrank(piece p, Flags pspec, square j, square i, square ia, couleur camp) {
  /* V3.45  TLi */
  square sq= (j / 24) & 1 ? rennormal (p, pspec, j, i, ia, camp) :
    renspiegel(p, pspec, j, i, ia, camp);
  return 24 * (j / 24) + sq % 24;
} /* renrank */

square renfile(piece p, Flags pspec, square j, square i, square ia, couleur camp)
{
  smallint	col= j % 24;

  if (camp == noir) {	/* white piece captured */
    if (is_pawn(p))
      return	(col + 216);
    else
      return	(col + 192);
  } else {		/* black piece captured */
    if (is_pawn(p))
      return	(col + 336);
    else
      return	(col + 360);
  }
} /* renfile */

square renspiegelfile(piece p, Flags pspec, square j, square i, square ia, couleur camp)
{
  return	renfile(p, pspec, j, i, ia, advers(camp));
} /* renspiegelfile */

square renpwc(piece p, Flags pspec, square j, square i, square ia, couleur camp)
{
  return	i;
} /* renpwc */

square renequipollents(piece p, Flags pspec, square j, square i, square ia, couleur camp)
{
  /* we have to solve the enpassant capture / locust capture problem in the future.  V3.80  NG */
#ifdef WINCHLOE
  return (j + ia - i);
#endif
  return (j + j - i);
} /* renequipollents */

square renequipollents_anti(piece p, Flags pspec, square j, square i, square ia, couleur camp)
{
  /* we have to solve the enpassant capture / locust capture problem in the future.  V3.80  NG */
#ifdef WINCHLOE
  return (ia + ia - i);
#endif
  return (j + j - i);
} /* renequipollents_anti */

square rensymmetrie(piece p, Flags pspec, square j, square i, square ia, couleur camp)
{
  return (575 - j);
} /* rensymmetrie */

square renantipoden(piece p, Flags pspec, square j, square i, square ia, couleur camp)
{
  i= j;
  if (j / 24 < 12)
    i+= 4 * 24;
  else
    i-= 4 * 24;
  if (j % 24 < 12)
    i+= 4;
  else
    i-= 4;
  return i;
} /* renantipoden */

square rendiagramm(piece p, Flags pspec, square j, square i, square ia, couleur camp)
{
  return DiaRen(pspec);
} /* rendiagramm */

square rennormal(piece p, Flags pspec, square j, square i, square ia, couleur camp)
{
  square		Result;
  smallint	col, ran;
  couleur		cou;

  col = j % 24;
  ran = j / 24;

  p= abs(p);				/* V3.1  TLi */

  if (CondFlag[circemalefiquevertical]) { /* V3.42  NG */
    col= 23 - col;
    if (p == db)
      p= roib;
    else if (p == roib)
      p= db;
  }

  if ((ran&1) != (col&1))			/* V2.60  NG */
    cou = blanc;
  else
    cou = noir;

  if (CondFlag[cavaliermajeur])		/* V2.60  NG */
    if (p == nb)
      p = cb;

  /* Below is the reason for the	define problems. What a "hack" !  V3.60  NG */
  if (CondFlag[leofamily] &&
      (p > Bishop) && (Vao >= p))
    p-= 4;

  if (camp == noir) {		/* captured white piece */
    if (is_pawn(p))
      Result= col + 216;
    else {
      if (!flagdiastip && TSTFLAG(pspec, FrischAuf)) {
        Result=  col + (CondFlag[glasgow] ? 360-24 : 360);
      }
      else
        switch(p) {
        case roib:
          Result= square_e1;
          break;
        case cb:
          Result= cou == blanc ? square_b1 : square_g1;
          break;
        case tb:
          Result= cou == blanc ? square_h1 : square_a1;
          break;
        case db:
          Result= square_d1;
          break;
        case fb:
          Result= cou == blanc ? square_f1 : square_c1;
          break;
        default:	/* fairy piece */
          Result= col + (CondFlag[glasgow] ? 360-24 : 360);	/* V3.39  TLi */
        }
    }
  } else {			/* captured black piece */
    if (is_pawn(p))
      Result= col + 336;
    else {
      if (!flagdiastip && TSTFLAG(pspec, FrischAuf)) {
        Result= col + (CondFlag[glasgow] ? 192+24 : 192);
      }
      else
        switch(p) {
        case fb:
          Result= cou == blanc ? square_c8 : square_f8;
          break;
        case db:
          Result= square_d8;
          break;
        case tb:
          Result= cou == blanc ? square_a8 : square_h8;
          break;
        case cb:
          Result= cou == blanc ? square_g8 : square_b8;
          break;
        case roib:
          Result= square_e8;
          break;
        default:	/* fairy piece */
          Result= col + (CondFlag[glasgow] ? 192+24 : 192);	/* V3.39  TLi */
        }
    }
  }

  return(Result);
} /* rennormal */

square rendiametral(piece p, Flags pspec, square j, square i, square ia, couleur camp) {
  /* V3.44  TLi */
  return 575-rennormal(p, pspec, j, i, ia, camp);
} /* rendiametral */

square renspiegel(piece p, Flags pspec, square j, square i, square ia, couleur camp)
{
  return	rennormal(p, pspec, j, i, ia, advers(camp));
} /* renspiegel */

square rensuper(piece p, Flags pspec, square j, square i, square ia, couleur camp)
{
  return	super[nbply];
} /* rensuper */
/**********  V3.1  TLi	end  *********/
#ifdef DOS
#	pragma warn +par
#endif

boolean is_pawn(piece p)	/* V3.22  TLi */
{
  switch (abs(p)) {
  case	Pawn:
  case	BerolinaPawn:
  case	SuperBerolinaPawn:
  case	SuperPawn:		return	True;
  default:			return	False;
  }
} /* end of is_pawn */

void genrn_cast(void) {    /* V3.55  TLi */
  /* It works only for castling_supported == TRUE */
  /* have a look at funtion verifieposition() in py6.c */
  if (TSTFLAGMASK(castling_flag[nbply],bl_castlings) > ke8_cancastle
      && e[square_e8]==roin
      /* then the king on e8 and at least one rook can castle !! */	/* V3.55  NG */
      && !echecc(noir)) {
    if (TSTFLAGMASK(castling_flag[nbply],blk_castling) == blk_castling
        && e[square_h8]==tn	/* 0-0 */
        && e[square_f8] == vide && e[square_g8] == vide) {
      if (complex_castling_through_flag)  /* V3.80  SE */
      {
        numecoup sic_nbcou= nbcou;
        empile (square_e8, square_f8, square_f8);
        if (nbcou > sic_nbcou) {
          boolean ok= (jouecoup() && !echecc(noir));
          repcoup();
          if (ok)
            empile(square_e8, square_g8, maxsquare+2);
        }
      } else {
        e[square_e8]= vide;
        e[square_f8]= roin;
        rn= square_f8;
        if (!echecc(noir)) {
          e[square_e8]= roin;
          e[square_f8]= vide;
          rn= square_e8;
          /* empile(square_e8, square_g8, square_g8);      V3.55  TLi */
          empile(square_e8, square_g8, maxsquare+2);	     /* V3.55  TLi */
        } else {
          e[square_e8]= roin;
          e[square_f8]= vide;
          rn= square_e8;
        }
      }
    }
    if (TSTFLAGMASK(castling_flag[nbply],blq_castling) == blq_castling
        && e[square_a8]==tn /* 0-0-0 */
        && e[square_d8] == vide
        && e[square_c8] == vide
        && e[square_b8] == vide) {
      if (complex_castling_through_flag)  /* V3.80  SE */
      {
        numecoup sic_nbcou= nbcou;
        empile (square_e8, square_d8, square_d8);
        if (nbcou > sic_nbcou) {
          boolean ok= (jouecoup() && !echecc(noir));
          repcoup();
          if (ok)
            empile(square_e8, square_c8, maxsquare+3);
        }
      } else {
        e[square_e8]= vide;
        e[square_d8]= roin;
        rn= square_d8;
        if (!echecc(noir)) {
          e[square_e8]= roin;
          e[square_d8]= vide;
          rn= square_e8;
          /* empile(square_e8, square_c8, square_c8);      V3.55  TLi */
          empile(square_e8, square_c8, maxsquare+3);	     /* V3.55  TLi */
        } else {
          e[square_e8]= roin;
          e[square_d8]= vide;
          rn= square_e8;
        }
      }
    }
  }
} /* genrn_cast */

void genrn(square i) {
  square	j;
  numvec	k;
  boolean	flag = false;		/* K im Schach */
  numecoup	anf, l1, l2;

  VARIABLE_INIT(anf);

  if (CondFlag[blrefl_king] && !calctransmute) {
	/* K im Schach zieht auch */
	piece	*ptrans;

	anf= nbcou;
	calctransmute= true;
	for (ptrans= transmpieces; *ptrans; ptrans++) {
      if (nbpiece[*ptrans]
          && (*checkfunctions[*ptrans])(i, *ptrans, eval_black))
      {
		flag = true;
        current_trans_gen=-*ptrans;
		gen_bl_piece(i, -*ptrans);
        current_trans_gen=vide;
      }
	}
	calctransmute= false;

	if (flag && nbpiece[orphanb]) {
      piece king= e[rn];
      e[rn]= dummyn;
      if (!echecc(noir)) {
		/* black king checked only by an orphan
		** empowered by the king */
		flag= false;
      }
      e[rn]= king;
	}

	/* K im Schach zieht nur */
	if (CondFlag[bltrans_king] && flag)
      return;
  }

  if (CondFlag[sting])				/* V3.63  NG */
	gerhop(i, 1, 8, noir);

  for (k= 8; k > 0; k--) {				/* V2.4c  NG */
	j= i + vec[k];
	if ((e[j] == vide) || (e[j] >= roib))
      empile(i, j, j);
  }

  if (flag) {		      /* testempile nicht nutzbar */
	/* VERIFY: has anf always a proper value?
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
	genrn_cast();					/* V3.55  TLi */

} /* genrn */

void gen_bl_ply(void) {
  square i, j, z;
  piece p;

  /* Don't try to "optimize" by hand. The double-loop is tested as the  */
  /* fastest way to compute (due to compiler-optimizations !) V3.14  NG */
  z= haut;					/* V2.90  NG */
  for (i= 8; i > 0; i--, z-= 16)			/* V2.90  NG */
	for (j= 8; j > 0; j--, z--) {			/* V2.90  NG */
      if ((p = e[z]) != vide) {
        if (TSTFLAG(spec[z], Neutral))
          p = -p;				/* V1.4c  NG */
        if (p < vide)				/* V2.70  TLi */
          gen_bl_piece(z, p);
      }
	}
} /* gen_bl_ply */

void gen_bl_piece_aux(square z, piece p) {	/* V3.46  SE/TLi */
  switch(p) {
  case roin:	genrn(z);
    break;
  case pn:	genpn(z);
    break;
  case cn:	genleap(z, 9, 16);
    break;
  case tn:	genrid(z, 1, 4);	/* V2.60  NG */
    break;
  case dn:	genrid(z, 1, 8);	/* V2.60  NG */
    break;
  case fn:	genrid(z, 5, 8);	/* V2.60  NG */
    break;
  default:	gfeernoir(z, p);
    break;
  }
} /* gen_bl_piece_aux */

static void orig_gen_bl_piece(square z, piece p) { /* V3.71 TM */
  piece pp;

  /* if (CondFlag[madras]) { */
  if (flag_madrasi) {				/* V3.60  TLi */
    if (! libre(z, true))
      return;
  }
  if (TSTFLAG(PieSpExFlags,Paralyse))		/* V2.90  TLi */
    if (paralysiert(z))
      return;

  if (anymars||anyantimars) {			/* V3.46  SE/TLi */
    square mren;
    Flags psp;

    if (is_phantomchess) {	/* V3.47  NG */
      numecoup	anf1, anf2, l1, l2;
      anf1= nbcou;
      /* generate standard moves first */
      flagactive= false;
      flagpassive= false;
      flagcapture= false;

      gen_bl_piece_aux(z, p);

      /* Kings normally don't move from their rebirth-square */
      if (p == e[rn] && !rex_phan)
        return;
      /* generate moves from rebirth square */
      flagactive= true;
      mren=(*marsrenai)(p,psp=spec[z],z,initsquare,initsquare,blanc);
      /* if rebirth square is where the piece stands,    */
      /* we've already generated all the relevant moves. */
      if (mren == z)
        return;
      if (e[mren] == vide) {
        anf2= nbcou;
        pp= e[z];	       /* Mars/Neutral bug V3.50 SE */
        e[z]= vide;
        spec[z]= EmptySpec;
        spec[mren]= psp;
        e[mren]= p;
        marsid= z;

        gen_bl_piece_aux(mren, p);

        e[mren]= vide;
        spec[z]= psp;
        e[z]= pp;
        flagactive= false;
        /* Unfortunately we have to check for */
        /* duplicate generated moves now.     */
        /* there's only ONE duplicate per arrival field possible ! */
        for (l1= anf1 + 1; l1 <= anf2; l1++)
          for (l2= anf2 + 1; l2 <= nbcou; l2++)
            if (ca[l1] == ca[l2]) {
              ca[l2]= initsquare;
              break;	/* remember: ONE duplicate ! */
            }
      }
    } else {
      /* generate noncapturing moves first */
      flagpassive= true;
      flagcapture= false;

      gen_bl_piece_aux(z, p);

      /* generate capturing moves now */
      flagpassive= false;
      flagcapture= true;
      more_ren=0;
      do {		  /* V3.50 SE Echecs Plus */
        mren=(*marsrenai)(p,psp=spec[z],z,initsquare,initsquare,blanc);
        if ((mren == z) || (e[mren] == vide)) {
          pp= e[z];		       /* Mars/Neutral bug V3.50 SE */
          e[z]= vide;
          spec[z]= EmptySpec;
          spec[mren]= psp;
          e[mren]= p;
          marsid= z;

          gen_bl_piece_aux(mren, p);

          e[mren]= vide;
          spec[z]= psp;
          e[z]= pp;
        }	/* if */
      } while (more_ren);
      flagcapture= false;
    }
  } else
    gen_bl_piece_aux(z, p);

  if (CondFlag[messigny] && !(z == rn && rex_mess_ex)) {	 /* V3.55  TLi */
    square *bnp;
    for (bnp= boardnum; *bnp; bnp++)
      if (e[*bnp] == - p)
        empile(z,*bnp,maxsquare+1);
  }
} /* gen_bl_piece */

void singleboxtype3_gen_bl_piece(square z, piece p) { /* V3.71 TM */
  numecoup save_nbcou = nbcou;
  unsigned int latent_prom = 0;
  square sq;
  for (sq = next_latent_pawn(initsquare,noir);
       sq!=initsquare;
       sq = next_latent_pawn(sq,noir))
  {
    piece pprom;
    for (pprom = next_singlebox_prom(vide,noir);
         pprom!=vide;
         pprom = next_singlebox_prom(pprom,noir))
    {
      numecoup save_nbcou = nbcou;
      ++latent_prom;
      e[sq] = -pprom;
      orig_gen_bl_piece(z, sq==z ? -pprom : p);
      e[sq] = pn;

      for (++save_nbcou; save_nbcou<=nbcou; ++save_nbcou)
      {
        sb3[save_nbcou].where = sq;
        sb3[save_nbcou].what = -pprom;
      }
    }
  }

  if (latent_prom==0)
  {
    orig_gen_bl_piece(z,p);
    for (++save_nbcou; save_nbcou<=nbcou; ++save_nbcou)
    {
      sb3[save_nbcou].where = initsquare;
      sb3[save_nbcou].what = vide;
    }
  }
} /* singleboxtype3_gen_bl_piece */

void (*gen_bl_piece)(square z, piece p) = &orig_gen_bl_piece; /* V3.71 TM */

void genmove(couleur camp)
{

  /* Abbruch waehrend der gesammten Laufzeit mit <ESC>	V2.70 TLi */
#ifdef ATARI
#	include <osbind.h>
#	define STOP_ON_ESC
#	define interupt (Bconstat(2) && (Bconin(2) == 27))
#endif /* ATARI */

#ifdef DOS
#   ifndef Windows
#	ifdef __TURBOC__
#	define STOP_ON_ESC
#	define interupt ((bioskey(1) != 0) && ((bioskey(0) >> 8) == 1))
#	endif /* __TURBOC__ */
#   endif

#	ifdef MSC
#	define STOP_ON_ESC
#	define interupt (_bios_keybrd(_KEYBRD_READY) && ((_bios_keybrd(_KEYBRD_READ) >> 8) == 1))
#	endif	/* MSC */

#	ifdef GCC					/* V3.11  NG */
#	define STOP_ON_ESC
#	define interupt (kbhit() && (getkey() == 27))	/* ESC == 27 */
#	endif	/* GCC */
#endif	/* DOS */

#ifdef OS2
#	ifdef GCC					/* V3.11  NG */
#	endif	/* GCC */
#endif /* OS2 */

#ifdef STOP_ON_ESC
  if (interupt) {
    PrintTime(InterMessage);		/* V2.90  NG */
    StdString("\n\n");
    CloseInput();				/* V2.90  NG */
    /* for some other purposes I need a return value
    ** different from 0.  TLi
    ** exit(0);
    */
    exit(1);
  }
#endif	/* STOP_ON_ESC */

  if (TSTFLAG(PieSpExFlags,Neutral))
    initneutre(advers(camp));
  nextply();
  trait[nbply]= camp;
  /* flagminmax= false;		 V2.90, V3.44  TLi */
  /* flag_minmax[nbply]= false;		       V3.44  TLi */
  we_generate_exact = false;		    /* V3.20  TLi */
  initkiller();

  if (CondFlag[exclusive]) {
    smallint nbrmates= 0;

    mateallowed[nbply]= true;

    if (camp == blanc)
      gen_wh_ply();
    else
      gen_bl_ply();

    while (encore()) {
      if (jouecoup() && (*stipulation)(camp))
        nbrmates++;
      repcoup();
    }

    mateallowed[nbply]= nbrmates < 2;
  }

  /* exact and consequent maximummers  V3.20  TLi */
  if (camp == blanc) {
    /* let's first generate consequent moves */
    if (wh_exact) {				  /* V3.20  TLi */
      we_generate_exact = true;
      there_are_consmoves = false;
      gen_wh_ply();
      if (CondFlag[whforsqu] && CondFlag[whconforsqu]
          && !there_are_consmoves) {
        /* There are no consequent moves.
        ** Now let's look for ``normal'' forced moves,
        ** but first reset nbcou etc.
        */
        we_generate_exact = false;
        finply();
        nextply();
        /* flagminmax= false;	  V3.44  TLi */
        /* flag_minmax[nbply]= false;	 V3.44	TLi */
        initkiller();
        gen_wh_ply();
        /* Puh - let's pray ! */
      }
      we_generate_exact = false;
    } else
      gen_wh_ply();
  } else {
    /* let's first generate consequent moves */
    if (bl_exact) {				  /* V3.20  TLi */
      we_generate_exact = true;
      there_are_consmoves = false;
      gen_bl_ply();
      if (CondFlag[blforsqu] && CondFlag[blconforsqu]
          && !there_are_consmoves) {
        /* There are no consequent moves.
        ** Now let's look for ``normal'' forced moves,
        ** but first reset nbcou etc.
        */
        we_generate_exact = false;
        finply();
        nextply();
        /* flagminmax= false;	  V3.44  TLi */
        /* flag_minmax[nbply]= false;	V3.44  TLi */
        initkiller();
        gen_bl_ply();
      }
      we_generate_exact = false;
    } else
      gen_bl_ply();
  }
  finkiller();

} /* genmove(camp) */

void joueparrain(void)	       /* H.D. 10.02.93 */
{
  piece	p= pprise[nbply-1];
  Flags	pspec= pprispec[nbply-1];
  square	cren= cp[repere[nbply]] + ca[nbcou] - cd[nbcou];

  if (e[cren] == vide) {
    sqrenais[nbply]= cren;
    ren_parrain[nbply]= e[cren]= p;
    spec[cren]= pspec;

    if (is_pawn(p) && PromSq(advers(trait[nbply-1]), cren)) {  /* V3.43  TLi */
      /* captured white pawn on eighth rank: promotion ! */
      /* captured black pawn on first rank: promotion ! */
      piece	pprom= cir_prom[nbply];

      if (TSTFLAG(pspec, Chameleon))	       /* V3.46  TLi */
        cir_cham_prom[nbply]= true;
      if (pprom == vide)
        cir_prom[nbply]= pprom= getprompiece[vide];
      if (p < vide)
        pprom= -pprom;
      e[cren]= pprom;
      nbpiece[pprom]++;
      if (cir_cham_prom[nbply])		/* V3.1  TLi */
        SETFLAG(pspec, Chameleon);
      spec[cren]= pspec;
    } else
      nbpiece[p]++;
    if (TSTFLAG(pspec, Neutral))	/* V3.65  SE */ /* bug reported by Kevin Begley */
      setneutre(cren);
  }
} /* end of joueparrain */

boolean testdblmate= False;		/* V3.50 SE */
piece pdisp[maxply+1];
Flags pdispspec[maxply+1];
square sqdep[maxply+1];

boolean patience_legal()	    /* V3.50 SE */
{
  square bl_last_vacated= initsquare, wh_last_vacated= initsquare;
  ply nply;
  /* n.b. inventor rules that R squares are forbidden after
     castling but not yet implemented */

  for (nply= nbply - 1 ; nply > 1 && !bl_last_vacated ; nply--)
    if (trait[nply] == noir)
      bl_last_vacated= sqdep[nply];
  for (nply= nbply - 1 ; nply > 1 && !wh_last_vacated ; nply--)
    if (trait[nply] == blanc)
      wh_last_vacated= sqdep[nply];
  return !((wh_last_vacated && e[wh_last_vacated]) ||
           (bl_last_vacated && e[bl_last_vacated]));
}

void find_mate_square(couleur camp);

int direction(square from, square to) {			/* V3.65  TLi */
  int dir= to-from, hori= to%24-from%24, vert= to/24-from/24, i=7;
  while ((hori%i) || (vert%i))
    i--;

  return dir/i;
} /* direction */

/* AMU V3.70 SE */
square blpc;

boolean eval_spec(square id, square ia, square ip)
{
  return id==blpc;
}

boolean att_once(square id)
{
  int i,j, cnt=0;
  square z=bas;
  piece p;
  square rb_=rb;
  rb=id ;

  for (i= 8; i > 0; i--, z+= 16)		/* V2.90  NG */
	for (j= 8; j > 0; j--, z++) {		/* V2.90  NG */
      if ((p = e[z]) != vide) {
        if (p < -obs)
        {
          blpc=z;
          if (rbechec(eval_spec))
            if (!++cnt)
              break;	     /* could modify to return int no. of attacks */
        }
      }
    }
  rb=rb_;
  return cnt==1;
}

square next_latent_pawn(square s, couleur c) { /* V3.71 TM */
  piece	pawn;
  int		i, delta;

  pawn=  c==blanc ? pb : pn;
  delta= c==blanc ? -1 : 1;

  if (s==initsquare) {
	i = 0;
	s = c==blanc ? haut : bas;
  }
  else {
	i = c==blanc ? haut-s+1 : s-bas+1;
	s += delta;
  }

  for (; i<8; ++i, s+=delta) {
	if (e[s]==pawn) {
      return s;
	}
  }

  return initsquare;
}

piece next_singlebox_prom(piece p, couleur c) {
  piece pprom;
  for (pprom = getprompiece[p];
       pprom!=vide;
       pprom = getprompiece[pprom])
  {
    assert(pprom<boxsize);
    if (nbpiece[c==blanc ? pprom : -pprom] < maxinbox[pprom])
      return pprom;
  }

  return vide;
}

#ifdef DEBUG
static  int nbrtimes = 0;
#endif

boolean jouecoup(void) {
  square  i,			/* case dep. */
    j,			/* case arr. */
    ip,			/* case pri. */
    cren= initsquare,	/* case ren. */
    /* initialised !  V3.50  NG */
    _rb,
    _rn;
  piece   pp,			/* piece pri. */
    pj,			/* piece joue */
    pja,		/* piece joue arr. */
    pprom = vide,
    pren,
    ph;
  Flags   ppspec;		/* piece prise */
  Flags   pjspec;		/* piece joue */
  ply     icount;

  int     oldnbpiece[derbla];				/* V3.50 SE */
  boolean flag_repub;					/* V3.50 SE */
  square  hurdle;					/* V3.64 SE */


#ifdef DEBUG
  nbrtimes++;
#endif

  hurdle= 0;		/* avoid compiler warning about use of
                       uninitialized variable. See the comment
                       at the first use of this variable
                       ElB, 2001-12-18.
                    */

  /* Orphans/refl. KK !!!! V3.0  TLi */
  /* now also for phantomchess  V3.47  NG - schoen krampfig */

  if (orph_refking || is_phantomchess) {
	while (ca[nbcou] == initsquare) {
      nbcou--;
	}
  }

  if (CondFlag[extinction]) {
	piece p;
	for (p= roib; p < derbla; p++) {
      oldnbpiece[p]= nbpiece[ trait[nbply]==blanc ? p : -p];
	}
  }

  _rb= RB_[nbply]= rb;	/* H.D. 10.02.93 */
  _rn= RN_[nbply]= rn;

  i= sqdep[nbply]= cd[nbcou];   /* V2.90  TLi */
  j= ca[nbcou];   /* V2.90  TLi */
  ip= cp[nbcou];

  if (CondFlag[amu])
	att_1[nbply]= att_once(i);

  if (CondFlag[imitators])				/* V2.4d  TM */
  {
    if (ip == maxsquare + 3)
      joueim(1);
    else if (ip != maxsquare + 2) /* joueim(0) (do nothing) if OO */
      joueim(j - i);
  }

  pjspec= jouespec[nbply]= spec[i];			/* V3.1  TLi */
  pja= pj= pjoue[nbply]= e[i];			/* V2.4c  NG */

  ppspec= pprispec[nbply]= spec[ip];			/* V3.1  TLi */
  pp= pprise[nbply]= e[ip];				/* V3.1  TLi */

  if (anyantimars && i == ip)
  {
    ppspec= pprispec[nbply]= 0;
    pp= pprise[nbply]= vide;
  }

  pdisp[nbply]= vide;					/* V3.50 SE */
  pdispspec[nbply]=0;

  if (trait[nbply]==noir && CondFlag[blsupertrans_king] && ctrans[nbcou]!=vide)
  {
    rn=initsquare;
    pja=ctrans[nbcou];
  }
  if (trait[nbply]==blanc && CondFlag[whsupertrans_king] && ctrans[nbcou]!=vide)
  {
    rb=initsquare;
    pja=ctrans[nbcou];
  }


  if (sbtype3 && sb3[nbcou].what!=vide) {		/* V3.71 TM */
	--nbpiece[e[sb3[nbcou].where]];
	e[sb3[nbcou].where] = sb3[nbcou].what;
	++nbpiece[sb3[nbcou].what];
	if (i==sb3[nbcou].where) {
      pj = pja = sb3[nbcou].what;
	}
  }

  if (TSTFLAG(pjspec, ColourChange)) {      /* V3.64 SE, V3.65  NG */
	/* this is a conditional initialisation
	   of 'hurdle'. So other places where this
	   variable is used, may not see propper
	   values. But other uses are done under
	   the same condition, tested above. As
	   long as this condition doesn't change
	   to true when its here tested as false,
	   everything works out fine. As far as I
	   could check the code, this assumption
	   is true, hence the code correct.
       ElB, 2001-12-18.
	*/
	hurdle= chop[nbcou];				 /* V3.64 SE */
  }

  switch (ip) {		 /* V3.55  TLi */

  case maxsquare+1:
	/* Messigny chess exchange */
	pprise[nbply]= e[i]= e[j];
	pprispec[nbply]= spec[i]= spec[j];
	jouearr[nbply]= e[j]= pj;
	spec[j]= pjspec;
	if (rb == i) {
      rb= j;
	}
	else {
      if (rb == j)
		rb= i;
	}
	if (rn == i) {
      rn= j;
	}
	else {
      if (rn == j) {
		rn= i;
      }
	}
	goto legality_test;
	break;

  case maxsquare+2: /* 0-0 */
	if (CondFlag[einstein]) {	  /* V3.44  NG */
      if (i == square_e1) {
		nbpiece[tb]--;
		if (CondFlag[reveinstein]) {
          e[square_f1]= db;
          nbpiece[db]++;
		}
		else {
          e[square_f1]= fb;
          nbpiece[fb]++;
		}
      }
      else {
		nbpiece[tn]--;
		if (CondFlag[reveinstein]) {
          e[square_f8]= dn;
          nbpiece[dn]++;
		}
		else {
          e[square_f8]= fn;
          nbpiece[fn]++;
		}
      }
	}
	else {
      e[i+1]= e[i+3];
	}
	spec[i+1]= spec[i+3];
	e[i+3]= vide;
	CLEARFL(spec[i+3]);
	if (i == square_e1) {
      CLRFLAGMASK(castling_flag[nbply],whk_castling);
	}
	else {
      CLRFLAGMASK(castling_flag[nbply],blk_castling);
	}
	break;

  case maxsquare+3: /* 0-0-0 */
	if (CondFlag[einstein]) {	  /* V3.44  NG */
      if (i == square_e1) {	      /* white */
		nbpiece[tb]--;
		if (CondFlag[reveinstein]) {
          e[square_d1]= db;
          nbpiece[db]++;
		}
		else {
          e[square_d1]= fb;
          nbpiece[fb]++;
		}
      }
      else {		   /* black */
		nbpiece[tn]--;
		if (CondFlag[reveinstein]) {
          e[square_d8]= dn;
          nbpiece[dn]++;
		}
		else {
          e[square_d8]= fn;
          nbpiece[fn]++;
		}
      }
	}
	else {
      e[i-1]= e[i-4];
	}
	spec[i-1]= spec[i-4];
	e[i-4]= vide;
	CLEARFL(spec[i-4]);
	if (i == square_e1) {
      CLRFLAGMASK(castling_flag[nbply],whq_castling);
	}
	else {
      CLRFLAGMASK(castling_flag[nbply],blq_castling);
	}
	break;
  } /* switch (ip) */

  e[i]= CondFlag[haanerchess] ? obs : vide;
  spec[i]= 0;

  if (PatienceB) {			  /* V3.50 SE */
	ply nply;
	e[i]= obs;
	for (nply= nbply - 1 ; nply > 1 ; nply--) {
      if (trait[nply] == trait[nbply]) {
		e[sqdep[nply]]= vide;
      }
	}
  }

  if (change_moving_piece) {
	/* Now pawn-promotion (even into 'kamikaze'-pieces)
	   is possible	!  NG */
	if (TSTFLAG(pjspec, Kamikaze)) {     /* V1.3c  NG, V2.4d  TM */
      if (pp != vide) {				/* V2.4c  NG */
		if (!anycirce) {	    /* V2.4d  TM , V3.1  TLi */
          pjspec= 0;
          pja= vide;
          /* to avoid useless promotions of a
             vanishing pawn */
		}
      }
	}
	if (CondFlag[linechamchess])			/* V3.64  TLi */
      pja= linechampiece(pja, j);

	if (CondFlag[chamchess])			/* V3.32  TLi */
      pja= champiece(pja);

	if (TSTFLAG(pjspec, Chameleon))			 /* V3.1  TLi */
      pja= champiece(pja);

	if (CondFlag[norsk])				 /* V3.1  TLi */
      pja= norskpiece(pja);

  } /* change_moving_piece */

  if (abs(pj) == andergb) {				/* V3.65  TLi */
    square sq= j - direction(i, j);

    if (!TSTFLAG(spec[sq], Neutral) && (sq != rb) && (sq != rn)) {
      change(sq);
      CHANGECOLOR(spec[sq]);
    }
  } /* andergb */

  ep[nbply]= ep2[nbply]= initsquare;			/* V3.37  TLi */
  if (is_pawn(pj)) {
    /* ep capture  V3.22  TLi */
	if (is_phantomchess) {				/* V3.52  NG */
      smallint col_diff, rank_j;

      col_diff= j%24 - i%24,
	    rank_j= j/24;

      if (rank_j == 11) {	/* 4th rank */
		switch (col_diff) {
        case 0:
          if (pj == pb && i != j-24)		/* V3.53  NG */
			ep[nbply]= j-24;
          break;

        case -2:
          if (pj == pbb && i != j-25)		/* V3.53  NG */
			ep[nbply]= j-25;
          break;

        case 2:
          if (pj == pbb && i != j-23)		/* V3.53  NG */
			ep[nbply]= j-23;
          break;
		}	/* switch (col_diff) */
      }
      else if (rank_j == 12) {	/* 5th rank */
		switch (col_diff) {
        case 0:
          if (pj == pn && i != j+24)		/* V3.53  NG */
			ep[nbply]= j+24;
          break;
        case -2:
          if (pj == pbn && i != j+23)		/* V3.53  NG */
			ep[nbply]= j+23;
          break;
        case 2:
          if (pj == pbn && i != j+25)		/* V3.53  NG */
			ep[nbply]= j+25;
          break;
		}	/* switch (col_diff) */
      }
	}	/* is_phantomchess */
	else {
      square ii;
      if (anyantimars)
        ii= cmren[nbcou];
      else
        ii= i;
      switch (abs(pj)) {
      case Pawn:
		switch (abs(ii - j)) {
        case 48: /* ordinary or Einstein double step */
          ep[nbply]= (ii + j) / 2;
          break;
        case 72: /* Einstein triple step */
          ep[nbply]= (ii + j + j) / 3;
          ep2[nbply]= (ii + ii + j) / 3;
          break;
		} /* end switch (abs(ii-j)) */
		break;
      case BerolinaPawn:
		if (abs(ii - j) > 25) {
		  /* The length of a single step is 23, 24 or 25.
		  ** Therefore it's a double step!
		  */
		  ep[nbply]= (ii + j) / 2;
		}
		break;
      } /* end switch (abs(pj)) */
	}

	/* promotion */
	if (PromSq(trait[nbply], j)) {			/* V3.39  TLi */
      /* moved to here because of anticirce
       */
      if ((pja= norm_prom[nbply]) == vide) {	/* V3.02  TLi */
		if (!CondFlag[noiprom] && Iprom[nbply]) {   /* V2.4d  TM */
          if (inum[nbply] == maxinum)
			FtlMsg(ManyImitators);
          for (icount = nbply; icount <= maxply; inum[icount++]++)
			;
          isquare[inum[nbply]-1] = j;
		}
		else {
          if (   sbtype1 /* V3.71 TM */
                 || sbtype2
                 || sbtype3)
          {
			pja = next_singlebox_prom(vide,trait[nbply]);
			assert(!sbtype1 || pja!=vide);
			if (pja==vide)
              /* pja will be recolored later if pj is black! */
              pja = abs(pj);		/* V3.76  ThM */
            /* pja = pj; */
          }
          else {
			pja= getprompiece[vide];

			if (CondFlag[frischauf])    /* V3.41b  TLi */
              SETFLAG(pjspec, FrischAuf);

			/* V3.1  TLi */
			if (pp != vide && anyanticirce) {
#ifdef BETTER_READABLE
              /* this coding seems to be better redable */
              do {
				cren= (*antirenai)(pja,
                                   pjspec, ip, i, advers(trait[nbply]));
				if (cren == i)
                  break;
				if (LegalAntiCirceMove(cren, ip, i))
                  /* V3.62  TLi */
                  break;
				pja= getprompiece[pja];
              } while (1);
#endif /*BETTER_READABLE*/

              while ((cren= (*antirenai)(pja, pjspec, ip, i, j, advers(trait[nbply]))) != i && !LegalAntiCirceMove(cren, ip, i))  /* V3.62  TLi */
              {
                pja= getprompiece[pja];
                if (!pja && CondFlag[antisuper])
                {
                  super[nbply]++;
                  pja= getprompiece[vide];
                }
              }
			}
          }
          norm_prom[nbply]= pja;
		}
      }
      else {				       /* V3.41b  TLi */
		if (CondFlag[frischauf])	       /* V3.41b  TLi */
          SETFLAG(pjspec, FrischAuf);
      }

      if (!(!CondFlag[noiprom] && Iprom[nbply])) {
		if (TSTFLAG(pjspec, Chameleon) && is_pawn(pj)) { /* V3.44  TLi */
          norm_cham_prom[nbply]= true;
		}
		if (pj < vide) {
          pja = -pja;
		}

		/* so also promoted neutral pawns
		 * have the correct color and an
		 * additional call to setneutre is
		 * not required.	TLi
		 */
		/* V3.22  TLi
		** pend2[nbply]= initsquare; */		/* V3.1  TLi */
		if (norm_cham_prom[nbply])		/* V3.1  TLi */
          SETFLAG(pjspec, Chameleon);
      }
      else {
		pja= vide;	/* imitator-promotion */
      }
	}
  } /* is_pawn() */

  /* V3.02  TLi */
  if (j != ip) {
	e[ip]= vide;
	spec[ip]= 0;
  }

  if (pp != vide) {			      /* V2.4c	NG, V3.1  TLi */
	/* V3.22  TLi
	** pend2[nbply]= initsquare;  V3.22  TLi */	 /* V3.1  TLi */
	nbpiece[pp]--;				/* V1.4c  NG */

	if (ip == rb)				/* V3.44  SE/TLi */
      rb= initsquare;			/* V3.44  SE/TLi */
	if (ip == rn)				/* V3.44  SE/TLi */
      rn= initsquare;			/* V3.44  SE/TLi */
  }

  if (change_moving_piece) {			/* V3.1  TLi */
	if (CondFlag[degradierung]		/* V3.1  TLi */
        && !is_pawn(pj)
        && i != _rn
        && i != _rb
        && (trait[nbply] == noir
            ? ((j > haut-32) && (j<haut - 8))
            : ((j < bas+32) && (j>bas + 8))))
	{
      if (pj < vide)
		pja= pn;
      else
		pja= pb;
	}

	if (	(CondFlag[tibet]
             &&  pp != vide
             && pja != -pp				 /* V2.90 TLi */
             && (	((trait[nbply] == noir)
                     && (i != _rn))
                    || ((trait[nbply] == blanc)
                        && CondFlag[dbltibet]
                        && (i != _rb))))
            || (CondFlag[andernach]
                && pp != vide
                &&  i != _rn
                &&  i != _rb))
	{
      SETCOLOR(pjspec, ppspec);
      if (CondFlag[volage] && !CondFlag[hypervolage])
		CLRFLAG(pjspec, Volage);

      /* nasty neutral implementation */
      if (pja < vide)
		pja= -pja;

      /* now the piece is white */
      /* has it to be changed? */
      if (    TSTFLAG(pjspec, Black)
              && (!TSTFLAG(pjspec, White) || neutcoul == noir))
      {
		pja= -pja;
      }
	}

	/* this seems to be misplaced -- it's checked every time TLi */
	/* therefore moved to this place !		   V3.60  NG */
	if (   CondFlag[antiandernach]
           && pp == vide
           && i != _rn
           && i != _rb )
	{
      /* the following also copes correctly with neutral
      ** pieces	V3.62  TLi */
      CLRFLAG(pjspec, Black);
      CLRFLAG(pjspec, White);
      CLRFLAG(pjspec, Neutral);
      if (trait[nbply] == noir) {
		SETFLAG(pjspec, White);
		pja= abs(pja);
      }
      else {
		SETFLAG(pjspec, Black);
		pja= -abs(pja);
      }
	}	/* CondFlag[antiandernach] ... */

	/* V3.33  TLi */
	if (  (   CondFlag[traitor]
              && trait[nbply] == noir			 /* V3.1  TLi */
              && j < square_a5
              && !TSTFLAG(pjspec, Neutral))
          || (   TSTFLAG(pjspec, Volage)
                 && SquareCol(i) != SquareCol(j))
          || (TSTFLAG(sq_spec[j], MagicSq)
              && _rn != i
              && _rb != i))
	{
      CHANGECOLOR(pjspec);
      if (/* CondFlag[volage] && */ !CondFlag[hypervolage])
		CLRFLAG(pjspec, Volage);
      pja= -pja;
	}

	if (CondFlag[einstein]				/* V3.1  TLi */
	    && !(CondFlag[antieinstein] && pp != vide)) /* V3.50  TLi */
	{
      pja= (pp == vide) ^ CondFlag[reveinstein]
        ?  dec_einstein(pja)
        : inc_einstein(pja);
	}

  } /* change_moving_piece */

  if (i == _rb) {					/* V2.1c  NG */
	if (rb!=initsquare) rb= j;
	CLRFLAGMASK(castling_flag[nbply],ke1_cancastle);
  }
  if (i == _rn) {
	if (rn!=initsquare) rn= j;
	CLRFLAGMASK(castling_flag[nbply],ke8_cancastle);
  }

  /* Needed for castling */				/* V3.35  NG */
  if (castling_supported) {
	/* pieces vacating a1, h1, a8, h8    V3.77  TLi */
	if (i == square_h1) {
      CLRFLAGMASK(castling_flag[nbply],rh1_cancastle);
	}
	else if (i == square_a1) {
      CLRFLAGMASK(castling_flag[nbply],ra1_cancastle);
	}
	else if (i == square_h8) {
      CLRFLAGMASK(castling_flag[nbply],rh8_cancastle);
	}
	else if (i == square_a8) {
      CLRFLAGMASK(castling_flag[nbply],ra8_cancastle);
	}
	/* pieces arriving at a1, h1, a8, h8 and possibly capturing a rook */	/* V3.77  TLi */
	if (j == square_h1) {
      CLRFLAGMASK(castling_flag[nbply],rh1_cancastle);
	}
	else if (j == square_a1) {
      CLRFLAGMASK(castling_flag[nbply],ra1_cancastle);
	}
	else if (j == square_h8) {
      CLRFLAGMASK(castling_flag[nbply],rh8_cancastle);
	}
	else if (j == square_a8) {
      CLRFLAGMASK(castling_flag[nbply],ra8_cancastle);
	}
  }	    /* castling_supported */

  if (   (CondFlag[andernach] && pp!=vide)
         || (CondFlag[antiandernach] && pp==vide)	 /* V3.60 SE */
         || (CondFlag[norsk])				/* V3.74  NG */
    )
  {
	if (castling_supported) {
      if (abs(pja) == Rook) {           /* V3.77  TLi */
        if (TSTFLAG(pjspec, White)) { /* V3.77  TLi */
          /* new white/neutral rook */
          if (j == square_h1)             /* h1 */
            SETFLAGMASK(castling_flag[nbply],rh1_cancastle);
          else if (j == square_a1)	      /* a1 */
            SETFLAGMASK(castling_flag[nbply],ra1_cancastle);
        }
        if (TSTFLAG(pjspec, Black)) { /* V3.77  TLi */
          /* new black/neutral rook */
          if (j == square_h8)	      /* h8 */
            SETFLAGMASK(castling_flag[nbply],rh8_cancastle);
          else if (j == square_a8)	      /* a8 */
            SETFLAGMASK(castling_flag[nbply],ra8_cancastle);
        }
      }
	} /* castling_supported */
  } /* andernach || antiandernach ... */

  if (TSTFLAG(pjspec, HalfNeutral))	     /* V3.57 SE */
  {
	if (TSTFLAG(pjspec, Neutral)) {
      CLRFLAG(pjspec, trait[nbply] == blanc ? Black : White);   /* V3.62  TLi */

      CLRFLAG(pjspec, Neutral);
      pja= trait[nbply]==noir ? -abs(pja) : abs(pja);

      if (rn == j && trait[nbply] == blanc)
        rn= initsquare;

      if (rb == j && trait[nbply] == noir)
        rb= initsquare;
	}
	else if (trait[nbply]==noir) {
      if (TSTFLAG(pjspec, Black)) {
		SETFLAG(pjspec, Neutral);
		SETFLAG(pjspec, White);		/* V3.62  TLi */
		pja= abs(pja);
		if (rn == j)
          rb = j;
      }
	}
	else if (trait[nbply]==blanc) {
      if (TSTFLAG(pjspec, White)) {
		SETFLAG(pjspec, Neutral);
		SETFLAG(pjspec, Black);		/* V3.62  TLi */
		pja= -abs(pja);
		if (rb == j)
          rn = j;
      }
	}
  }

  e[j]= pja;
  spec[j] = pjspec;			    /* V3.1  TLi */
  jouearr[nbply]= pja;		    /* V3.1  TLi */

  if (pj != pja) {			    /* V3.1  TLi */
	nbpiece[pj]--;
	nbpiece[pja]++;
  }

  if (sbtype2) { /* 3.71 TM */
	couleur adv = advers(trait[nbply]);

	if (sb2[nbply].where==initsquare) {
      assert(sb2[nbply].what==vide);
      sb2[nbply].where = next_latent_pawn(initsquare,adv);
      if (sb2[nbply].where!=initsquare) {
		sb2[nbply].what = next_singlebox_prom(vide,adv);
		if (sb2[nbply].what==vide)
          sb2[nbply].where = initsquare;
      }
	}

	if (sb2[nbply].where!=initsquare) {
      assert(e[sb2[nbply].where] == (adv==blanc ? pb : pn));
      assert(sb2[nbply].what!=vide);
      --nbpiece[e[sb2[nbply].where]];
      e[sb2[nbply].where] =   adv==blanc
        ? sb2[nbply].what
        : -sb2[nbply].what;
      ++nbpiece[e[sb2[nbply].where]];
    }
  }

  if (TSTFLAG(pjspec, HalfNeutral) && TSTFLAG(pjspec, Neutral))   /* V3.57 SE */
	setneutre(j);

  /* moved outside "jouegenre"  V3.74  NG */

  /* Duellantenschach   V3.0  TLi */
  if (CondFlag[duellist]) {
    if (trait[nbply] == noir) {
      whduell[nbply]= whduell[nbply - 1];
      blduell[nbply]= j;
    }
    else {
      blduell[nbply]= blduell[nbply - 1];
      whduell[nbply]= j;
    }
  }

  if (jouegenre) {				    /* V2.4c  NG */
	couleur traitnbply= trait[nbply];	/* V3.1  TLi */
	/* AntiCirce  V3.1  TLi */
	if (pp != vide && anyanticirce) {
      cren= (*antirenai)(pja, pjspec, ip, i, j, advers(traitnbply));
      if (CondFlag[antisuper])
      {
        while (!LegalAntiCirceMove(cren, ip, i)) cren++;
        super[nbply]= cren;
      }
      e[j]= vide;
      spec[j]= 0;
      crenkam[nbply]= cren;
      if (    is_pawn(pj)			       /* V3.39  TLi */
              && PromSq(traitnbply, cren))
      {
		/* white pawn on eighth rank or
           black pawn on first rank - promotion ! */
		nbpiece[pja]--;
		pja= norm_prom[nbply];
		if (pja == vide)
          norm_prom[nbply]= pja= getprompiece[vide];
		if (pj < vide)
          pja= -pja;
		nbpiece[pja]++;
      }
      e[cren]= pja;
      spec[cren]= pjspec;
      if (i == _rb)
		rb= cren;
      if (i == _rn)
		rn= cren;

      if (castling_supported) {			/* V3.77  TLi */
		piece abspja= abs(pja);
		if (abspja == King) {
          if (TSTFLAG(pjspec, White) && cren == square_e1
              && (!CondFlag[dynasty] || nbpiece[roib]==1)) { /* V4.02 TM */
            /* white king new on e1 */
            SETFLAGMASK(castling_flag[nbply],ke1_cancastle);
          }
          else if (TSTFLAG(pjspec, Black) && cren == square_e8
                   && (!CondFlag[dynasty] || nbpiece[roin]==1)) { /* V4.02 TM */
            /* black king new on e8 */
            SETFLAGMASK(castling_flag[nbply],ke8_cancastle);
          }
		}
		else if (abspja == Rook) {
          if (TSTFLAG(pjspec, White)) {
			/* new white/neutral rook */
            if (cren == square_h1) {
              SETFLAGMASK(castling_flag[nbply],rh1_cancastle);
            }
            else if (cren == square_a1) {
              SETFLAGMASK(castling_flag[nbply],ra1_cancastle);
            }
          }
          if (TSTFLAG(pjspec, Black)) {
			/* new black/neutral rook */
            if (cren == square_h8) {
              SETFLAGMASK(castling_flag[nbply],rh8_cancastle);
            }
            else if (cren == square_a8) {
              SETFLAGMASK(castling_flag[nbply],ra8_cancastle);
            }
          }
		}
      } /* castling_supported */
	} /* AntiCirce */

	if (CondFlag[sentinelles]) {			/* V2.90 TLi */
      if (    i > bas + 8
              && i < haut - 8
              && !is_pawn(pj))		    /* V3.64  NG,TLi */
      {
		if (SentPionNeutral) {
          if (TSTFLAG(pjspec, Neutral)) {
			nbpiece[e[i]= sentinelb]++;
			SETFLAG(spec[i], Neutral);
			setneutre(i);
          }
          else if ((traitnbply == noir) ^ SentPionAdverse) {
			nbpiece[e[i]= sentineln]++;
			SETFLAG(spec[i], Black);
          }
          else {
			nbpiece[e[i]= sentinelb]++;
			SETFLAG(spec[i], White);
          }
          if (   nbpiece[sentinelb] > max_pb
                 || nbpiece[sentineln] > max_pn
                 || nbpiece[sentinelb]+nbpiece[sentineln] > max_pt)
          {	/* V3.60 SE */
			/* rules for sentinelles + neutrals not yet
			   written but it's very likely this logic will
			   need to be refined
            */
			nbpiece[e[i]]--;
			e[i]= vide;
			spec[i]= 0;
          }
          else {
			senti[nbply]= true;
          }
		}
		else if ((traitnbply == noir) ^ SentPionAdverse) {
          /* V3.50 SE */
          if (   nbpiece[sentineln] < max_pn
                 && nbpiece[sentinelb]+nbpiece[sentineln]<max_pt
                 && (  !flagparasent
                       ||(nbpiece[sentineln]
                          <= nbpiece[sentinelb]
                          +(pp==sentinelb?1:0))))
          {	/* V3.62 SE */
			nbpiece[e[i]= sentineln]++;
			SETFLAG(spec[i], Black);	/* V3.39  TLi */
			senti[nbply]= true;
          }
		}
		else if (   nbpiece[sentinelb] < max_pb
                    && nbpiece[sentinelb]+nbpiece[sentineln]<max_pt
                    && (  !flagparasent
                          ||(nbpiece[sentinelb]
                             <= nbpiece[sentineln]
                             + (pp==sentineln?1:0))))/* V3.62 SE */
		{
          nbpiece[e[i]= sentinelb]++;
          SETFLAG(spec[i], White);  /* V3.39	TLi */
          senti[nbply]= true;
		}
      }
	}

	if (anycirce) {			/* V2.4c  NG */
      /* circe-rebirth of moving kamikaze-piece */
      if (TSTFLAG(pjspec, Kamikaze) && (pp != vide)) {
		if (CondFlag[couscous]) {
          cren= (*circerenai)(pp, ppspec, ip, i, j, traitnbply);
		}
		else {
          cren= (*circerenai)(pja, pjspec, ip, i, j, advers(traitnbply));
		}
		if (cren != j) {
          e[j]= vide;
          spec[j]= 0;
          if ((e[cren] == vide)		/* V3.1  TLi */
              && !(	CondFlag[contactgrid]
                    && nogridcontact(cren)))	/* V2.5c  NG */
          {
			crenkam[nbply]= cren;
			e[cren]= pja;
			spec[cren]= pjspec;
			if (rex_circe) {
              if (i == _rb)
				rb= cren;
              if (i == _rn)
				rn= cren;

              if (castling_supported
                  && (abs(pja) == King)) {    /* V3.77  TLi */
				if (TSTFLAG(pjspec, White) && cren == square_e1
                    && (!CondFlag[dynasty] || nbpiece[roib]==1)) { /* V4.02 TM */
                  /* white king reborn on e1 */
                  SETFLAGMASK(castling_flag[nbply],ke1_cancastle);
				}
				else if (TSTFLAG(pjspec, Black) && cren == square_e8
                         && (!CondFlag[dynasty] || nbpiece[roin]==1)) { /* V4.02 TM */
                  /* black king reborn on e8 */
                  SETFLAGMASK(castling_flag[nbply],ke8_cancastle);
				}
              }
			}
			if (castling_supported
                && (abs(pja) == Rook)) {	/* V3.77  TLi */
              if (TSTFLAG(pjspec, White)) {
				if (cren == square_h1) {
                  /* white rook reborn on h1 */
                  SETFLAGMASK(castling_flag[nbply],rh1_cancastle);
				}
				else if (cren == square_a1) {
                  /* white rook reborn on a1 */
                  SETFLAGMASK(castling_flag[nbply],ra1_cancastle);
				}
              }
              if (TSTFLAG(pjspec, Black)) {
				if (cren == square_h8) {
                  /* black rook reborn on h8 */
                  SETFLAGMASK(castling_flag[nbply],rh8_cancastle);
				}
				else if (cren == square_a8) {
                  /* black rook reborn on a8 */
                  SETFLAGMASK(castling_flag[nbply],ra8_cancastle);
				}
              }
			}
          } else
			nbpiece[pja]--;
		}
      } /* Kamikaze */

      /* normal circe-rebirth of captured piece */
      /* moved to repcoup  V3.1  TLi
       * sqrenais[nbply]= initsquare;    V2.60  NG
       */
      if (CondFlag[parrain]) {
		if ((pprise[nbply-1] != vide))
          joueparrain();
      }
      else {
		if (pp != vide) {    /* V2.4c  NG */
          /* V3.1  TLi */
          if (anyclone && i != _rn && i != _rb) {
			/* that's rubish  TLi  V3.22
			** pren= -pj;
			** ppspec= pjspec;
			*/
			/* Circe Clone - new implementation V3.22  TLi
			** captured pieces are reborn as pieces
			** of the same type as the capturing piece
			** if the latter one is not royal.
			*/
			/* change type of pieces according to colour */
			pren = (pj * pp < 0) ? -pj : pj;
			/* If it is a pawn give it the pawn-attribut.
			** Otherwise delete it - the captured piece may
			** have been a pawn, but is not anymore.
			*/
          }
          else if (anytraitor) {
            pdispspec[nbply]=ppspec;
            pren= -pp;
            CHANGECOLOR(ppspec);
          }
          else {
			if (CondFlag[chamcirce]) {	/* V3.50  TLi */
              pren= ChamCircePiece(pp);
			}
			else if (CondFlag[antieinstein]) {
              pren= inc_einstein(pp);
			}
			else {
              pren= pp;
			}
          }

          if (CondFlag[couscous])	    /* V3.1  TLi */
			cren= (*circerenai)(pja, pjspec, ip, i, j, advers(traitnbply));
          else
			cren= (*circerenai)(pren, ppspec, ip, i, j, traitnbply);

          if (    !rex_circe
                  && (flag_testlegality || CondFlag[brunner])
                  && /* V3.50 SE */
                  (ip == _rb || ip == _rn))
          {
            /* ordinary circe and (isardam, brunner or
            ** ohneschach) it can happen that the king is
            ** captured while testing the legality of a
            ** move. Therefore prevent the king from being
            ** reborn.
            */
            cren= initsquare;
          }

          if ( (e[cren] == vide || flagAssassin)/* V2.4c NG */
               && !(	CondFlag[contactgrid]
                        && nogridcontact(cren)))
          { /* V2.5c	NG */
			sqrenais[nbply]= cren;
			if (rex_circe) {    /* V3.33  TLi */
              /* neutral K  V3.02  TLi */
              if (_rb == ip) {
				rb= cren;
              }
              if (_rn == ip) {
				rn= cren;
              }

              if (castling_supported
                  && (abs(pren) == King)) {/* V3.77  TLi */
				if (TSTFLAG(ppspec, White) && cren == square_e1
                    && (!CondFlag[dynasty] || nbpiece[roib]==1)) { /* V4.02 TM */
                  /* white king reborn on e1 */
                  SETFLAGMASK(castling_flag[nbply],ke1_cancastle);
				}
				else if (TSTFLAG(ppspec, Black) && cren == square_e8
                         && (!CondFlag[dynasty] || nbpiece[roin]==1)) { /* V4.02 TM */
                  /* black king reborn on e8 */
                  SETFLAGMASK(castling_flag[nbply],ke8_cancastle);
				}
              }
			}

			if (castling_supported
                && (abs(pren) == Rook)) {	/* V3.77  TLi */
              if (TSTFLAG(ppspec, White)) {
				if (cren == square_h1) {
                  /* white rook reborn on h1 */
                  SETFLAGMASK(castling_flag[nbply],rh1_cancastle);
				}
				else if (cren == square_a1) {
                  /* white rook reborn on a1 */
                  SETFLAGMASK(castling_flag[nbply],ra1_cancastle);
				}
              }
              if (TSTFLAG(ppspec, Black)) {
				if (cren == square_h8) {
                  /* black rook reborn on h8 */
                  SETFLAGMASK(castling_flag[nbply],rh8_cancastle);
				}
				else if (cren == square_a8) {
                  /* black rook reborn on a8 */
                  SETFLAGMASK(castling_flag[nbply],ra8_cancastle);
				}
              }
			}
			if (   anycirprom
                   && is_pawn(pp)
                   && PromSq(advers(traitnbply), cren))
			{				/* V3.39  TLi */
              /* captured white pawn on eighth rank: promotion ! */
              /* captured black pawn on first rank: promotion ! */
              pprom= cir_prom[nbply];
              if (pprom == vide) {
				cir_prom[nbply]= pprom= getprompiece[vide];
              }
              pren = pren < vide ? -pprom : pprom;   /* V3.33  TLi */
              if (cir_cham_prom[nbply]) { /* V3.1  TLi */
				SETFLAG(ppspec, Chameleon);
              }
			}
			if (  TSTFLAG(ppspec, Volage)
                  && SquareCol(cren) != SquareCol(ip))  /* V3.22  TLi */
			{
              pren= -pren;	  /* V3.33  TLi */
              CHANGECOLOR(ppspec);  /* V3.33  TLi */
              if (!CondFlag[hypervolage]) {
				CLRFLAG(ppspec, Volage);
              }
			}
			if (flagAssassin) {		  /* V3.50 SE */
              nbpiece[pdisp[nbply]=e[cren]]--;
              pdispspec[nbply]=spec[cren];
			}
			nbpiece[e[cren]= pren]++;    /* V3.33  TLi */
			spec[cren]= ppspec;	     /* V3.33  TLi */
          }
		}
      }
	}

	if (bl_royal_sq != initsquare)	/* V3.50  TLi */
      rn= bl_royal_sq;

	if (wh_royal_sq != initsquare)	/* V3.50  TLi */
      rb= wh_royal_sq;

	if (CondFlag[republican]) {	    /* V3.50 SE */
      /* V3.53 Code involving flag_repub has no effect as it was
         included to ensure that if a side can place a mated K,
         it must. This is apparently not the rule, but code left
         in in case anybody wants alternative rule (return
         statement from jouecoup would need modifying) - SE
      */
      flag_repub= (super[nbply] < bas);
      if (trait[nbply]==blanc) {
        if (flag_writinglinesolution && repub_k[nbply] != initsquare) {      /* v3.53 SE */
          rn= repub_k[nbply];
          e[rn]= roin;
          nbpiece[roin]++;
		}
		else if (!rn && !flag_dontaddk) {
          flag_dontaddk= true;
          find_mate_square(blanc);
          flag_dontaddk= false;
          repub_k[nbply]= super[nbply]<= haut ? super[nbply] : initsquare;
          if (!flag_repub) {
			flag_repub= !(rn==initsquare);
          }
		}
		else
        {
          repub_k[nbply]= initsquare;
          super[nbply]= haut + 1;
        }
      }
      else {
		if (flag_writinglinesolution && repub_k[nbply] != initsquare) {		/* v3.53 SE */
          rb= repub_k[nbply];
          e[rb]= roib;
          nbpiece[roib]++;
		}
		else if (!rb && !flag_dontaddk) {
          flag_dontaddk= true;
          find_mate_square(noir);
          flag_dontaddk= false;
          repub_k[nbply]= super[nbply]<= haut ? super[nbply] : initsquare;
          if (!flag_repub) {
			flag_repub= !(rb==initsquare);
          }
		}
		else
        {
          repub_k[nbply]= initsquare;
          super[nbply]= haut + 1;
        }
      }
	} /* republican */

	if (CondFlag[actrevolving])
      RotateMirror(rot270);

	if (CondFlag[arc]) {	   /* V3.62 SE */

      square id, ia;
      id=i; ia=j;
      if (id==square_d4 || id==square_e4
          || id==square_d5 || id==square_e5
          || ia==square_d4 || ia==square_e4
          || ia==square_d5 || ia==square_e5)
      {
		Flags temp=spec[square_d4];
		piece ptemp=e[square_d4];
		e[square_d4]= e[square_e4];
		spec[square_d4]= spec[square_e4];
		e[square_e4]= e[square_e5];
		spec[square_e4]= spec[square_e5];
		e[square_e5]= e[square_d5];
		spec[square_e5]= spec[square_d5];
		e[square_d5]= ptemp;
		spec[square_d5]= temp;
		if (rb==square_d4) {
		  rb= square_d5;
		}
		else if (rb==square_d5) {
		  rb= square_e5;
		}
		else if (rb==square_e5) {
		  rb= square_e4;
		}
		else if (rb==square_e4) {
		  rb= square_d4;
		}
		if (rn==square_d4) {
		  rn= square_d5;
		}
		else if (rn==square_d5) {
		  rn= square_e5;
		}
		else if (rn==square_e5) {
		  rn= square_e4;
		}
		else if (rn==square_e4) {
		  rn= square_d4;
		}
      }
	}

	if (trait[nbply]==blanc ? CondFlag[white_oscillatingKs] : CondFlag[black_oscillatingKs]) {
      /* V3.78 SE */
      boolean priorcheck= false;
      square temp= rb;
      piece temp1= e[rb];
      Flags temp2= spec[rb];

      oscillatedKs[nbply]= false;
      if (OscillatingKingsTypeB[trait[nbply]])
        priorcheck= echecc(trait[nbply]);
      if ((oscillatedKs[nbply]= (!OscillatingKingsTypeC[trait[nbply]] || echecc(advers(trait[nbply]))))) /* V3.80  SE */
      {
		e[rb]= e[rn];
		spec[rb]= spec[rn];

		e[rn]= temp1;
		spec[rn]= temp2;
		rb= rn;
		rn= temp;
        CLRFLAGMASK(castling_flag[nbply],ke1_cancastle|ke8_cancastle);  /* V3.81  SE */
        if (rb==square_e1)
          SETFLAGMASK(castling_flag[nbply],ke1_cancastle);
        if (rn==square_e8)
          SETFLAGMASK(castling_flag[nbply],ke8_cancastle);
		if (OscillatingKingsTypeB[trait[nbply]] && priorcheck)
          return false;
      }
	}

	if (TSTFLAG(pjspec, ColourChange)) {		 /* V3.64 SE */
      if (abs(ph= e[hurdle]) > roib) {
		nbpiece[ph]--;
		e[hurdle]= -ph;
		nbpiece[-ph]++;
		CHANGECOLOR(spec[hurdle]);
      }
	}

  } /* if (jouegenre) */

legality_test:

#ifdef	DEBUG
  marge= 4 * nbply;
  Tabulate();
  ecritcoup();
  StdChar('\n');
#endif	/* DEBUG */

  if (CondFlag[extinction]) {
	piece p;
	for (p= roib; p<derbla; p++) {
      if (    oldnbpiece[p]
              && !nbpiece[trait[nbply]==blanc ? p : -p])
      {
        return false;
      }
	}
  }
  /*
    if (trait[nbply]==noir && !echecc(blanc)) return false;
  */	/* V3.62  SE */

  if (CondFlag[dynasty]) { /* V4.02 TM */
    /* adjust rn, rb and/or castling flags */
    square *bnp;
    square s;

    if (nbpiece[roib]==1) {
      if (rb==initsquare)
        for (bnp= boardnum; *bnp; bnp++) {
          s = *bnp;
          if (e[s] == roib) {
            if (rb==initsquare && s==square_e1)
              SETFLAGMASK(castling_flag[nbply],ke1_cancastle);
            rb = *bnp;
            break;
          }
        }
    }
    else
      rb = initsquare;

    if (nbpiece[roin]==1) {
      if (rn==initsquare)
        for (bnp= boardnum; *bnp; bnp++) {
          s = *bnp;
          if (e[s] == roin) {
            if (rn==initsquare && s==square_e8)
              SETFLAGMASK(castling_flag[nbply],ke8_cancastle);
            rn = *bnp;
            break;
          }
        }
    }
    else
      rn = initsquare;
  }

  return (!jouetestgenre				/* V3.50 SE */
          || (
            ((!flag_testlegality) || pos_legal())
            /* V3.44, 3.51  SE/TLi */
            && (!flagAssassin || (cren != rb && cren != rn))
            /* V3.50 SE */
            && (!testdblmate || (rb!=initsquare && rn!=initsquare))
            /* V3.50 SE */
            && (!CondFlag[patience] || PatienceB || patience_legal())
            /* V3.50 SE */
            /* don't call patience_legal if TypeB as obs > vide ! */
            /* &&(!CondFlag[republican] || flag_repub) */
            /* V3.50 SE, removed v3.53 SE - rule clarification  */
            && (  !CondFlag[blackultraschachzwang]
                  || trait[nbply]==blanc
                  || echecc(blanc))
            && (  !CondFlag[whiteultraschachzwang]
                  || trait[nbply]==noir
                  || echecc(noir))
            ));
} /* end of jouecoup */

void IncrementMoveNbr(void) {		 /* V3.44  TLi */
  if (MoveNbr++ < RestartNbr)
	return;

  sprintf(GlobalStr,"%3d  (", MoveNbr);
  StdString(GlobalStr);
  ecritcoup();			    /* V3.02  TLi */
  if (!flag_regression) {		/* V3.74  NG */
	StdString("   ");
	PrintTime(TimeString);		    /* V2.90  NG */
  }
#ifdef HASHRATE
  StdString("   ");
  HashStats(0, NULL);
#endif
  StdString(")\n");
} /* IncrementMoveNbr */

void repcoup(void) {
  square	i, j, rencas, ip;
  piece	pj, pp, pprom, ph;
  ply		icount;
  Flags	pjspec;			     /*, pjspec_  V3.62  TLi */
  boolean	next_prom = true;	    /* V3.50 SE */ /* 3.71 TM*/
  square	nextsuper;				 /* V3.52 SE */
  square	hurdle;					/* V3.65  NG */

  nextsuper = 0;
  /* we initialize the variable here to avoid
     compiler messages about usage of potential
     uninitialized variables.
     ElB, 2001-12-18.
  */


  if (jouegenre) {				       /* V3.53  TLi */

    /*	if (trait[nbply]==blanc ? CondFlag[white_oscillatingKs] : CondFlag[black_oscillatingKs])	*/
    if (oscillatedKs[nbply])  /* V3.80  SE - for Osc Type C */
	{
      square temp= rb;                   /* V3.78 SE */
      piece temp1= e[rb];
      Flags temp2= spec[rb];
      e[rb]= e[rn];
      spec[rb]= spec[rn];
      e[rn]= temp1;
      spec[rn]= temp2;
      rb= rn;
      rn= temp;
	}

	if (CondFlag[actrevolving]) {
      RotateMirror(rot90);
	}

	if (CondFlag[arc]) {	       /* V3.62 SE */
      /* RotateMirror(rot90);	*/	/* V3.62  SE */
      square id=cd[nbcou];
      square ia=ca[nbcou];
      if ( id==square_d4 || id==square_e4
           || id==square_d5 || id==square_e5
           || ia==square_d4 || ia==square_e4
           || ia==square_d5 || ia==square_e5)
      {
		Flags temp=spec[square_d4];
		piece ptemp=e[square_d4];
		e[square_d4]= e[square_d5];
		spec[square_d4]= spec[square_d5];
		e[square_d5]= e[square_e5];
		spec[square_d5]= spec[square_e5];
		e[square_e5]= e[square_e4];
		spec[square_e5]= spec[square_e4];
		e[square_e4]= ptemp;
		spec[square_e4]= temp;
		if (rb==square_d4) {
          rb= square_e4;
		}
		else if (rb==square_d5) {
          rb= square_d4;
		}
		else if (rb==square_e5) {
          rb= square_d5;
		}
		else if (rb==square_e4) {
          rb= square_e5;
		}
		if (rn==square_d4) {
          rn= square_e4;
		}
		else if (rn==square_d5) {
          rn= square_d4;
		}
		else if (rn==square_e5) {
          rn= square_d5;
		}
		else if (rn==square_e4) {
          rn= square_e5;
		}
      }
	}
  } /* jouegenre */

  i= cd[nbcou];
  j= ca[nbcou];
  ip= cp[nbcou];
  pp= pprise[nbply];
  pj= pjoue[nbply];
  pjspec= jouespec[nbply];
  /*	 pjspec_= spec[j];   V3.62  TLi */

  if (sbtype2) { /* 3.71 TM */

	sb2[nbply+1].where = initsquare;
	sb2[nbply+1].what = vide;

	if (sb2[nbply].where!=initsquare) {
      couleur adv = advers(trait[nbply]);

      assert(sb2[nbply].what!=vide);
      --nbpiece[e[sb2[nbply].where]];
      e[sb2[nbply].where] = adv==blanc ? pb : pn;
      ++nbpiece[e[sb2[nbply].where]];

      sb2[nbply].what = next_singlebox_prom(sb2[nbply].what,adv);
      if (sb2[nbply].what==vide) {
		sb2[nbply].where = next_latent_pawn(sb2[nbply].where,adv);
		if (sb2[nbply].where!=initsquare) {
          sb2[nbply].what = next_singlebox_prom(vide,adv);
          assert(sb2[nbply].what!=vide);
		}
      }

      next_prom = sb2[nbply].where==initsquare;
	}
  }

  if (sbtype3 && sb3[nbcou].what!=vide) { /* V3.71 TM */
	piece pawn = trait[nbply]==blanc ? pb : pn;
	e[sb3[nbcou].where] = pawn;
	if (i!=sb3[nbcou].where) {
      --nbpiece[sb3[nbcou].what];
      ++nbpiece[pawn];
	}
  }

  if (jouegenre) {				/* V3.64 SE */
	if (TSTFLAG(pjspec, ColourChange)) {
      hurdle= chop[nbcou];		/* V3.65  NG */
      if (abs(ph= e[hurdle]) > roib) {	/* V3.65  NG */
		nbpiece[ph]--;
		e[hurdle]= -ph;
		nbpiece[-ph]++;
		CHANGECOLOR(spec[hurdle]);
      }
	}
  }

  castling_flag[nbply]= castling_flag[nbply-1];    /* V3.55  TLi */

  switch (ip) {	/* V3.55  TLi */
  case maxsquare+1: /* Messigny chess exchange */
	e[j]= e[i];
	spec[j]= spec[i];
	e[i]= pj;
	spec[i]= pjspec;
	nbcou--;
	rb= RB_[nbply];
	rn= RN_[nbply];
	return;

  case maxsquare+2: /* 0-0 */
	if (CondFlag[einstein]) {	  /* V3.44  NG */
      if (i == square_e1) {   /* white */
		e[square_h1]= tb;
		nbpiece[tb]++;
		if (CondFlag[reveinstein])
          nbpiece[db]--;
		else
          nbpiece[fb]--;
      }
      else {    /* black */
		e[square_h8]= tn;
		nbpiece[tn]++;
		if (CondFlag[reveinstein])
          nbpiece[dn]--;
		else
          nbpiece[fn]--;
      }
	}
	else {
      e[i+3]= e[i+1];
	}
	spec[i+3]= spec[i+1];
	e[i+1]= vide;
	CLEARFL(spec[i+1]);
	/* reset everything */	/* V3.35  NG */
	break;

  case maxsquare+3: /* 0-0-0 */
	if (CondFlag[einstein]) {	  /* V3.44  NG */
      if (i == square_e1) {    /* white */
		e[square_a1]= tb;
		nbpiece[tb]++;
		if (CondFlag[reveinstein])
          nbpiece[db]--;
		else
          nbpiece[fb]--;
      }
      else {     /* black */
		e[square_a8]= tn;
		nbpiece[tn]++;
		if (CondFlag[reveinstein])
          nbpiece[dn]--;
		else
          nbpiece[fn]--;
      }
	}
	else {
      e[i-4]= e[i-1];
	}
	spec[i-4]= spec[i-1];
	e[i-1]= vide;
	CLEARFL(spec[i-1]);
	/* reset everything */	/* V3.35  NG */
	break;

  } /* switch (ip) */

  /* the supercirce rebirth square has to be evaluated here in the
  ** position after the capture. Otherwise it is more difficult to
  ** allow the rebirth on the original square of the capturing piece
  ** or in connection with locust or e.p. captures.
  ** V3.22  TLi
  */
  if (   (CondFlag[supercirce] && pp != vide)
         || isapril[abs(pp)]  || (CondFlag[antisuper] && pp != vide))
  {
	/* V3.1, 3.52  TLi, SE */
	nextsuper= super[nbply];
	while ((e[++nextsuper] != vide) && (nextsuper < haut))
      ;
    if (CondFlag[antisuper] && AntiCirCheylan && nextsuper==ip)  /* V3.81  SE */
      while ((e[++nextsuper] != vide) && (nextsuper < haut))
	    ;
  }

  if (CondFlag[republican]) {				/* V3.50 SE */
	square sq= repub_k[nbply];
	if (sq != initsquare) {
      e[sq]= vide;
      if (sq==rn) {
        rn=initsquare;
        nbpiece[roin]--;
      }
      if (sq==rb) {
        rb=initsquare;
        nbpiece[roib]--;
      }
	}
  }

#ifdef SUPERFLUOUS   /* V3.62  TLi */
  /* pjspec_ is a assigned a value that is not used later ... */
  if (TSTFLAG(pjspec_, HalfNeutral)) {		 /* V3.57 SE */
	if (TSTFLAG(pjspec_, Neutral)) {
      SETFLAG(pjspec_, trait[nbply]==noir ? Black : White);
      CLRFLAG(pjspec_, Neutral);
	}
	else if (trait[nbply]==noir) {
      if (TSTFLAG(pjspec_, Black)) {
		SETFLAG(pjspec_, Neutral);
		CLRFLAG(pjspec_, Black);
      }
	}
	else if (trait[nbply]==blanc) {
      if (TSTFLAG(pjspec_, White)) {
		SETFLAG(pjspec_, Neutral);
		CLRFLAG(pjspec_, White);
      }
	}
  }
#endif	/* SUPERFLUOUS */

  /* first delete all changes */
  if (repgenre) {					/* V2.4c  NG */
	if (senti[nbply]) {				/* V2.90 TLi */
      --nbpiece[e[i]];
      senti[nbply]= false;
	}
	if (CondFlag[imitators])				/* V2.4d  TM */
	{
      if (ip == maxsquare + 3)
        joueim(-1);
      else if (ip != maxsquare + 2) /* joueim(0) (do nothing) if OO */
        joueim(i - j);			   /* verschoben TLi */
	}

	if (TSTFLAG(PieSpExFlags, Neutral)) {
      /* the following is faster !  TLi
       * initneutre((pj > vide) ? blanc : noir);
       */

      if (TSTFLAG(pjspec, Neutral) &&
          (pj < vide ? noir : blanc) != neutcoul)
        pj= -pj;
      if (TSTFLAG(pprispec[nbply], Neutral) &&
          (pp < vide ? noir : blanc) != neutcoul)
        pp= -pp;
	}
	if ((rencas= sqrenais[nbply]) != initsquare) {
      sqrenais[nbply]= initsquare;	    /* V3.1  TLi */
      if (rencas != j) {	    /* V2.60  NG */
		nbpiece[e[rencas]]--;
		e[rencas]= vide;
		spec[rencas]= 0;
      }
      if (flagAssassin && pdisp[nbply]) {		 /* V3.50 SE */
		nbpiece[e[rencas]= pdisp[nbply]]++;
		spec[rencas]= pdispspec[nbply];
      }
      if (anytraitor)
		spec[rencas]= pdispspec[nbply];
	}
	/* V3.1  TLi */
	if ((rencas= crenkam[nbply]) != initsquare) {
      /* Kamikaze and AntiCirce */
      crenkam[nbply]= initsquare;     /* V2.60  NG */
      if (rencas != j) {
		nbpiece[e[rencas]]--;
		e[rencas]= vide;
		spec[rencas]= 0;
      }
	}
  } /* if (repgenre) */

  nbpiece[e[j]]--;

  /* now reset old position */
  e[i]= pj;
  nbpiece[pj]++;
  spec[i] = pjspec;
  if (j != ip) {
	e[j]= vide;
	spec[j] = 0;
  }
  if (PatienceB) {			  /* V3.50 SE */
	ply nply;
	for (nply= nbply - 1 ; nply > 1 ; nply--) {
      if (trait[nply] == trait[nbply]) {
		e[sqdep[nply]]= obs;
      }
	}
  }
  if (!anyantimars || ip != i)
  {
    e[ip]= pp;
    spec[ip]= pprispec[nbply];
  }

  if (pp != vide)
	nbpiece[pp]++;

  rb= RB_[nbply];		/* H.D. 15.02.93 */
  rn= RN_[nbply];

  if (abs(pj) == andergb) {			  /* V3.65  TLi */
    square sq= j - direction(i, j);

    if (!TSTFLAG(spec[sq], Neutral) && (sq != rb) && (sq != rn)) {
      change(sq);
      CHANGECOLOR(spec[sq]);
    }
  } /* andergb */

  /* at last modify promotion-counters and decrement nbcou */
  /* ortho- und pwc-Umwandlung getrennt  V3.02  TLi */
  if (CondFlag[republican]) {
	next_prom= (super[nbply] > haut);
	if (next_prom) {
      super[nbply]= superbas;
	}
  }

  if (next_prom) {
	if ((pprom= norm_prom[nbply]) != vide) {
      if (   sbtype1
             || sbtype2)					 /* V3.71 TM */
      {
	    pprom = next_singlebox_prom(pprom,trait[nbply]);
      }
      else {
	    pprom= getprompiece[pprom];

	    if (pp != vide && anyanticirce) {		/* V3.1  TLi */
          /* CLRFLAG(pjspec, APawn);  V3.22  TLi */
          while (pprom != vide) {
		    rencas= (*antirenai)(pprom,
                                 pjspec, ip, i, j, advers(trait[nbply]));
		    if (rencas == i)
              break;
		    if (LegalAntiCirceMove(rencas, ip, i)) {
              /* V3.62  TLi */
              break;
		    }
		    pprom= getprompiece[pprom];
          }
	    }
      }
      norm_prom[nbply]= pprom;

      if (  (pprom == vide)
            && TSTFLAG(PieSpExFlags, Chameleon)		/* V3.1  TLi */
            && !norm_cham_prom[nbply])
      {
	    pprom= getprompiece[vide];
	    if (pp != vide && anyanticirce)		 /* V3.1  TLi */
          while (pprom != vide &&
                 (rencas= (*antirenai)(pprom, pjspec, ip, i, j, advers(trait[nbply]))) != i &&
                 e[rencas] != vide)
            pprom= getprompiece[pprom];

	    norm_prom[nbply]= pprom;
	    norm_cham_prom[nbply]= true;
      }
      if ((pprom == vide) && !CondFlag[noiprom])   /* V2.4d  TM */
        Iprom[nbply]= true;
	}
	else if (!CondFlag[noiprom] && Iprom[nbply]) {	 /* V2.4d  TM */
      for (icount= nbply; icount <= maxply; inum[icount++]--);
      Iprom[nbply]= false;
	}

	if (pprom == vide) {
      norm_cham_prom[nbply]= false;	    /* V3.1  TLi */
      if (anycirprom && ((pprom= cir_prom[nbply]) != vide)) { /* V3.1  TLi */
		pprom= cir_prom[nbply]= getprompiece[pprom];
		if ( pprom == vide
             && TSTFLAG(PieSpExFlags, Chameleon)  /* V3.1	TLi */
             && !cir_cham_prom[nbply])
		{
          cir_prom[nbply]= pprom= getprompiece[vide];
          cir_cham_prom[nbply]= true;
		}
      }
      if (   pprom == vide
             && !(!CondFlag[noiprom]
                  && Iprom[nbply]))				 /* V2.4d  TM */
      {
		if (   (CondFlag[supercirce] && pp != vide)
               || isapril[abs(pp)] || (CondFlag[antisuper] && pp != vide))		  /* V3.52 SE */
		{
          super[nbply]= nextsuper;
          if ((super[nbply] > haut) || (CondFlag[antisuper] && !LegalAntiCirceMove(nextsuper, ip, i))) {
            super[nbply]= superbas;
			nbcou--;
          }
		}
		else {
          nbcou--;
		}
      }
	}
  } /* next_prom*/
} /* end of repcoup */

boolean pattencore(couleur camp, square** pattfld) {	  /* V3.50 SE */
  square i;
  piece p;

  if (encore())
	return true;

  if (TSTFLAG(PieSpExFlags,Neutral))
	initneutre(advers(camp));

  while ((i= *(*pattfld)++)) {
	if ((p= e[i]) != vide) {
      if (TSTFLAG(spec[i], Neutral))
		p= -p;
      if (camp == blanc) {
		if ((p > obs) && (i != rb)) {
          gen_wh_piece(i, p);
		}
      }
      else if ((p < vide) && (i != rn)) {
		gen_bl_piece(i, p);
      }
      if (encore()) {
		return true;
      }
	}
  }
  return false;
} /* pattencore */

boolean patt(couleur camp)
{
  square *pattfield= boardnum;  /* local so allows nested calls to patt */

  if ((camp == blanc ? !wh_exact : !bl_exact) &&	      /* 3.20  NG */
      !flag_testlegality) {				   /* V3.45  TLi */
    nextply();
    cakil= initsquare;
    /* initialize flagkil too, otherwise some maximummer moves
    ** may be "eaten" */
    flagkil= false;		     /* V3.44  TLi */
    trait[nbply]= camp;
    /* flagminmax= false;		     V2.90, V3.44  TLi */
    /* flag_minmax[nbply]= false;	     V3.44  TLi */
    if (TSTFLAG(PieSpExFlags,Neutral))
      initneutre(advers(camp));
    if (camp == blanc) {
      if (rb != initsquare)
        gen_wh_piece(rb, abs(e[rb]));	    /* V3.02  TLi */
    } else if (rn != initsquare)
      gen_bl_piece(rn, -abs(e[rn]));	  /* V3.02  TLi */

    if (CondFlag[MAFF] || CondFlag[OWU]) {					/* V3.78 SE */
      int k_fl= 0, w_unit= 0;
      while (encore()) {
        if (jouecoup()) {
          if (camp==noir ? pprise[nbply]>=roib : pprise[nbply]<=roib)
            w_unit++;        /* assuming OWU is OBU for checks to wK !! */
          if (!echecc(camp))
            k_fl++;
        }
        repcoup();
      }
      if ( (CondFlag[OWU] && (k_fl!=0 || w_unit!=1)) ||
           (CondFlag[MAFF] && (k_fl!=1)) ) {
        finply();
        return false;
      }
    }

    while (pattencore(camp, &pattfield)) {
      if (jouecoup()
          || (CondFlag[blackultraschachzwang] && (camp == noir))
          || (CondFlag[whiteultraschachzwang] && (camp == blanc))
	    ) {       /* V3.44	SE/TLi */
        if (! echecc(camp)) {
          repcoup();
          finply();
          return false;
        }
      }
      repcoup();
    }
    finply();
  } else {
    couleur ad= advers(camp);		/* V3.53  TLi */

    /* exact-maxis, ohneschach */
    optimize= false;      /* V3.44	TLi */
    if (!CondFlag[ohneschach]) {		/* V3.53  TLi */
      genmove(camp);
      while (encore()) {
        if (jouecoup()) {	/* V3.44  SE/TLi */
          if (! echecc(camp)) {
            repcoup();
            finply();
            return false;
          }
        }
        repcoup();
      }
      finply();

    } else {				/* V3.53  TLi */
      genmove(camp);
      while (encore()) {
        CondFlag[ohneschach]= false;
        jouecoup();
        CondFlag[ohneschach]= true;
        if (!echecc(ad) && pos_legal()) {
          repcoup();
          finply();
          return false;
        }
        repcoup();
      }
      finply();
      optimize= false;
      genmove(camp);
      while (encore()) {
        CondFlag[ohneschach]= false;
        jouecoup();
        CondFlag[ohneschach]= true;
        if (echecc(ad) && pos_legal()) {
          repcoup();
          finply();
          return false;
        }
        repcoup();
      }
      finply();
    }
  }
  return true;
} /* patt */

boolean stip_target(couleur camp)
{
  return	ca[nbcou] == TargetField
	&& crenkam[nbply] == initsquare    /* V3.22  TLi */
	&& !echecc(camp);
}

boolean stip_circuit(couleur camp) {
  square cazz, renkam;

  cazz = ca[nbcou];
  renkam= crenkam[nbply];

  return
    (((renkam == initsquare && (DiaRen(spec[cazz]) == cazz))
      || (renkam != initsquare && (DiaRen(spec[renkam]) == renkam)))
     && !echecc(camp));
}

boolean stip_circuitB(couleur camp) {
  square sqren= sqrenais[nbply];

  return
	(    (sqren != initsquare && (DiaRen(spec[sqren]) == sqren))
         && !echecc(camp));
}

boolean stip_exchange(couleur camp) {
  square cazz, sq, renkam;

  cazz = ca[nbcou];
  renkam= crenkam[nbply];

  if ( renkam == initsquare
       && (DiaRen(spec[sq= DiaRen(spec[cazz])]) == cazz)
       && ((camp == blanc) ? (e[sq] >= roib) : (e[sq] <= roin))
       && sq!=cazz)
  {
	return !echecc(camp);
  }
  if ( renkam != initsquare
       && (DiaRen(spec[sq= DiaRen(spec[renkam])]) == renkam)
       && ((camp == blanc) ? (e[sq] >= roib) : (e[sq] <= roin))
       && sq!=renkam)
  {
	return !echecc(camp);
  }
  return false;
}

boolean stip_exchangeB(couleur camp) {
  square sqren= sqrenais[nbply];
  square sq;

  return sqren != initsquare
    && (DiaRen(spec[sq= DiaRen(spec[sqren])]) == sqren)
    && ((camp == noir) ? (e[sq] >= roib) : (e[sq] <= roin))
    && sq!=sqren
    && !echecc(camp);
}

boolean stip_capture(couleur camp) {
  return pprise[nbply] != vide && !echecc(camp);
}

boolean stip_mate(couleur camp) {
  boolean flag;
  couleur ad= advers(camp);

  if (CondFlag[amu] && !att_1[nbply])
	return false;

  if (TSTFLAG(PieSpExFlags,Paralyse)) {
	if (!echecc(ad) || echecc(camp) || !patt(ad))
      return false;
	genmove(ad);
	flag= encore();
	finply();
	return flag;
  }
  else {
	return (echecc(ad) && !echecc(camp) && patt(ad));
  }
}

boolean para_patt(couleur camp) {	  /* V3.50 SE */
  if (echecc(camp)) {
	boolean flag;
	genmove(camp);
	flag= !encore();
	finply();
	return flag;
  }
  else {
	return patt (camp);
  }
}

boolean stip_stale(couleur camp) {
  /* modifiziert fuer paralysierende Steine	 V3.02	TLi */
  couleur ad= advers(camp);

  if (echecc(camp))
	return false;

  if (TSTFLAG(PieSpExFlags, Paralyse)) {
	return para_patt(ad);
  }
  else {
	return (!echecc(ad) && patt(ad));
  }
}

boolean stip_mate_or_stale(couleur camp) {	      /* V3.60 SE */
  /* modifiziert fuer paralysierende Steine	 V3.02	TLi */
  boolean flag;
  couleur ad= advers(camp);

  if (echecc(camp))
	return false;

  if (TSTFLAG(PieSpExFlags, Paralyse)) {
	flag= para_patt(ad);
  }
  else {
	flag= patt(ad);
  }
  if (flag) {
	mate_or_stale_patt= !echecc(ad);
  }

  return flag;
}


boolean stip_dblstale(couleur camp) {
  /* ich glaube, fuer paral. Steine sind hier keine
     Modifizierungen erforderlich			  V3.02  TLi */

  couleur ad= advers(camp);
  if (TSTFLAG(PieSpExFlags, Paralyse)) {		 /* V3.50 SE */
	return (para_patt(ad) && para_patt(camp));
  }
  else {
	return !echecc(ad) && !echecc(camp)
      && patt(ad) && patt(camp);
  }
}

boolean stip_autostale(couleur camp) {			  /* V3.50 SE */

  if (echecc(advers(camp))) {
	return false;
  }

  if (TSTFLAG(PieSpExFlags, Paralyse)) {
	return para_patt(camp);
  }
  else {
	return (!echecc(camp) && patt(camp));
  }
}

boolean stip_check(couleur camp)
{
  return (echecc(advers(camp)) && !echecc(camp));
}

boolean stip_steingewinn(couleur camp)
{
  return pprise[nbply] != vide
    && (!anycirce || (sqrenais[nbply] == initsquare))
    && !echecc(camp);
}

/* V3.1  TLi */
boolean stip_ep(couleur camp)
{
  return ca[nbcou] != cp[nbcou]
	&& is_pawn(pjoue[nbply])		       /* V3.31  TLi */
	&& !echecc(camp);
}

/* V3.32  TLi */
boolean stip_doublemate(couleur camp) {
  boolean flag;
  couleur ad= advers(camp);

  /* if (!echecc(ad) && !echecc(camp))  bug spotted by NG */

  if (!echecc(ad) || !echecc(camp))	   /* V3.35  TLi */
	return false;

  if (TSTFLAG(PieSpExFlags, Paralyse)) {
	genmove(ad);
	flag = encore();
	finply();
	if (!flag)
      return false;
	genmove(camp);
	flag = encore();
	finply();
	if (!flag)
      return false;
  }
  testdblmate= flag_nk;		/* Neutral K bug fix V3.50 SE */
  /*true;	      modified to allow isardam + ##  */
  /* may still have problem with isardam + nK + ##  !*/
  flag=patt(ad) && patt(camp);
  testdblmate=false;
  return flag;
}

/* V3.32  TLi */
boolean stip_castling(couleur camp) {
  unsigned char diff;

  diff= castling_flag[nbply-1]-castling_flag[nbply];

  return (diff == whk_castling
          || diff == whq_castling
          || diff == blk_castling
          || diff == blq_castling)
    && !echecc(camp);
} /* stip_castling */

boolean stip_any(couleur camp)
{
  return true;
}

void find_mate_square(couleur camp)			  /* V3.50 SE */
{
  square sq;

  if (camp == blanc) {
	while ((sq= ++super[nbply]) <= haut) {
      if (e[sq] == vide) {
		rn= sq;
		e[rn]= roin;
        nbpiece[roin]++;
		if ((*stipulation)(camp)) {		 /* V3.53  SE */
          return;
		}
        nbpiece[roin]--;
		e[rn]= vide;
      }
	}
	rn= initsquare;
	return;
  } else {
	while ((sq= ++super[nbply]) <= haut) {
      if (e[sq] == vide) {
		rb= sq;
		e[rb]= roib;
        nbpiece[roib]++;
		if ((*stipulation)(camp)) {		 /* V3.53  SE */
          return;
		}
        nbpiece[roib]--;
		e[rb]= vide;
      }
	}
	rb= initsquare;
	return;
  }
}

