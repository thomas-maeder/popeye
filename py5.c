/********************* MODIFICATIONS to py5.c **************************
 **
 ** Date       Who  What
 **
 ** 2006/05/09 SE   New conditions: SAT, StrictSAT, SAT X Y (invented L.Salai sr.)
 **
 ** 2006/05/14 SE   New Condition: TakeMake (invented H.Laue)
 **
 ** 2006/06/28 SE   New condition: Masand (invented P.Petkov)
 **
 ** 2006/06/30 SE   New condition: BGL (invented P.Petkov)
 **
 ** 2006/07/30 SE   New condition: Schwarzschacher  
 **
 ** 2007/01/28 SE   New condition: Annan Chess 
 **
 ** 2007/05/01 SE   Bugfix: StrictSAT bug 
 **
 ** 2007/05/04 SE   Bugfix: SAT + BlackMustCapture
 **
 ** 2007/05/04 SE   Bugfix: SAT + Ultraschachzwang
 **
 **************************** End of List ******************************/

#ifdef macintosh /* is always defined on macintosh's  SB */
# define SEGM2
# include "pymac.h"
#endif

#ifdef ASSERT
#include <assert.h> /* V3.71 TM */
#else
/* When ASSERT is not defined, eliminate assert calls.
 * This way, "#ifdef ASSERT" is not clobbering the source.
 *      ElB, 2001-12-17.
 */
#define assert(x)
#endif /* ASSERT */
#include <stdio.h>
#include <stdlib.h>  /* H.D. 10.02.93 prototype fuer exit */

#ifdef DOS   /* V2.90  ElB, TLi, NG */
# ifdef GCC
#  include <pc.h>
# else
#  include <bios.h>
# endif /* GCC */
#endif /* DOS */

#ifdef OS2   /* V3.11  NG */
# ifdef GCC
# endif /* GCC */
#endif /* OS2 */

#include "py.h"
#include "pyproc.h"
#include "pyhash.h"
#include "pydata.h"
#include "pymsg.h"

piece linechampiece(piece p, square sq) { /* V3.64  TLi */
  piece pja= p;
  if (CondFlag[leofamily]) {
    switch (abs(p)) {
    case leob: case maob: case vaob: case paob:
      switch(sq%onerow) {
      case 8:  case 15:   pja= paob; break;
      case 9:  case 14:   pja= maob; break;
      case 10: case 13:   pja= vaob; break;
      case 11:     pja= leob; break;
      }
    }
    return (pja != p && p < vide) ? - pja : pja;
  } else
    if (CondFlag[cavaliermajeur]) {      /* V3.01  NG */
      switch (abs(p)) {
      case db: case nb: case fb: case tb:
        switch(sq%onerow) {
        case 8:  case 15:   pja= tb;  break;
        case 9:  case 14:   pja= nb;  break;
        case 10: case 13:   pja= fb;  break;
        case 11:     pja= db;  break;
        }
      }
      return (pja != p && p < vide) ? - pja : pja;
    } else {
      switch (abs(p)) {
      case db: case cb: case fb: case tb:
        switch(sq%onerow) {
        case 8:  case 15:   pja= tb;  break;
        case 9:  case 14:   pja= cb;  break;
        case 10: case 13:   pja= fb;  break;
        case 11:     pja= db;  break;
        }
      }
      return (pja != p && p < vide) ? - pja : pja;
    }
} /* linechampiece */


piece champiece(piece p)
{
  /* function realisiert Figurenwechsel bei Chamaeleoncirce */
  if (CondFlag[leofamily]) {      /* V3.01  NG */
    switch (p) {
    case leob: return maob;
    case leon: return maon;
    case maob: return vaob;
    case maon: return vaon;
    case vaob: return paob;
    case vaon: return paon;
    case paob: return leob;
    case paon: return leon;
    }
  } else
    if (CondFlag[cavaliermajeur]) {      /* V3.01  NG */
      switch (p) {
      case db: return nb;
      case dn: return nn;
      case nb: return fb;
      case nn: return fn;
      case fb: return tb;
      case fn: return tn;
      case tb: return db;
      case tn: return dn;
      }
    } else {
      switch (p) {
      case db: return cb;
      case dn: return cn;
      case cb: return fb;
      case cn: return fn;
      case fb: return tb;
      case fn: return tn;
      case tb: return db;
      case tn: return dn;
      }
    }
  return p;
}

piece norskpiece(piece p)
{
  /* function realisiert Figurenwechsel bei NorskSjakk */
  if (CondFlag[leofamily]) {   /* V3.01  NG */
    switch (p) {
    case leob: return maob;
    case leon: return maon;
    case maob: return leob;
    case maon: return leon;
    case vaob: return paob;
    case vaon: return paon;
    case paob: return vaob;
    case paon: return vaon;
    }
  } else
    if (CondFlag[cavaliermajeur]) {  /* V3.01  NG */
      switch (p) {
      case db: return nb;
      case dn: return nn;
      case nb: return db;
      case nn: return dn;
      case fb: return tb;
      case fn: return tn;
      case tb: return fb;
      case tn: return fn;
      }
    } else {
      switch (p) {
      case db: return cb;
      case dn: return cn;
      case cb: return db;
      case cn: return dn;
      case fb: return tb;
      case fn: return tn;
      case tb: return fb;
      case tn: return fn;
      }
    }
  return p;
} /* norskpiece */

piece dec_einstein(piece p)
{
  /* function realisiert Figurenwechsel bei Einsteinschach abwaerts */
  switch (p) {
  case db: return tb;
  case dn: return tn;
  case tb: return fb;
  case tn: return fn;
  case fb: return cb;
  case fn: return cn;
  case cb: return pb;
  case cn: return pn;
  }
  return p;
} /* end of dec_einstein */

piece inc_einstein(piece p)
{
  /* function realisiert Figurenwechsel bei Einsteinschach aufwaerts */
  switch (p) {
  case pb: return cb;
  case pn: return cn;
  case cb: return fb;
  case cn: return fn;
  case fb: return tb;
  case fn: return tn;
  case tb: return db;
  case tn: return dn;
  }
  return p;
} /* end of inc_einstein */

/**********  V3.1  TLi begin  *********/
/* all renai-functions have the following parameters:
   p: piece captured
   pspec: specifications of the piece captured
   j: square where the capture takes place
   i: square whre the capturing piece just came from
   camp: colour of the moving=capturing side
*/
#ifdef DOS
# pragma warn -par
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
    default: k= -1; /* avoid compiler warning, ElB. */
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
  square sq= (j / onerow) & 1 ? rennormal (p, pspec, j, i, ia, camp) :
    renspiegel(p, pspec, j, i, ia, camp);
  return onerow * (j / onerow) + sq % onerow;
} /* renrank */

square renfile(piece p, Flags pspec, square j, square i, square ia, couleur camp)
{
  smallint col= j % onerow;

  if (camp == noir) { /* white piece captured */
    if (is_pawn(p))
      return (col + 216);
    else
      return (col + 192);
  } else {  /* black piece captured */
    if (is_pawn(p))
      return (col + 336);
    else
      return (col + 360);
  }
} /* renfile */

square renspiegelfile(piece p, Flags pspec, square j, square i, square ia, couleur camp)
{
  return renfile(p, pspec, j, i, ia, advers(camp));
} /* renspiegelfile */

square renpwc(piece p, Flags pspec, square j, square i, square ia, couleur camp)
{
  return i;
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

square rensymmetrie(piece p, Flags pspec,
                    square j, square i, square ia,
                    couleur camp)
{
  return (haut+bas) - j;
} /* rensymmetrie */

square renantipoden(piece p, Flags pspec,
                    square j, square i, square ia,
                    couleur camp)
{
  smallint const row= j/onerow - nr_of_slack_rows_below_board;
  smallint const file= j%onerow - nr_of_slack_files_left_of_board;
  
  i= j;

  if (row<nr_rows_on_board/2)
    i+= nr_rows_on_board/2*dir_up;
  else
    i+= nr_rows_on_board/2*dir_down;

  if (file<nr_files_on_board/2)
    i+= nr_files_on_board/2*dir_right;
  else
    i+= nr_files_on_board/2*dir_left;

  return i;
} /* renantipoden */

square rendiagramm(piece p, Flags pspec,
                   square j, square i, square ia,
                   couleur camp)
{
  return DiaRen(pspec);
} /* rendiagramm */

square rennormal(piece p, Flags pspec,
                 square j, square i, square ia,
                 couleur camp)
{
  square  Result;
  smallint col, ran;
  couleur  cou;

  col = j % onerow;
  ran = j / onerow;

  p= abs(p);    /* V3.1  TLi */

  if (CondFlag[circemalefiquevertical]) { /* V3.42  NG */
    col= onerow-1 - col;
    if (p == db)
      p= roib;
    else if (p == roib)
      p= db;
  }

  if ((ran&1) != (col&1))   /* V2.60  NG */
    cou = blanc;
  else
    cou = noir;

  if (CondFlag[cavaliermajeur])  /* V2.60  NG */
    if (p == nb)
      p = cb;

  /* Below is the reason for the define problems. What a "hack" !  V3.60  NG */
  if (CondFlag[leofamily] &&
      (p > Bishop) && (Vao >= p))
    p-= 4;

  if (camp == noir) {  /* captured white piece */
    if (is_pawn(p))
      Result= col + (nr_of_slack_rows_below_board+1)*onerow;
    else {
      if (!flagdiastip && TSTFLAG(pspec, FrischAuf)) {
        Result= (col
                 + (onerow
                    *(CondFlag[glasgow]
                      ? nr_of_slack_rows_below_board+nr_rows_on_board-2
                      : nr_of_slack_rows_below_board+nr_rows_on_board-1)));
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
        default: /* fairy piece */
          Result= (col
                   + (onerow
                      *(CondFlag[glasgow]
                        ? nr_of_slack_rows_below_board+nr_rows_on_board-2
                        : nr_of_slack_rows_below_board+nr_rows_on_board-1))); /* V3.39  TLi */
        }
    }
  } else {   /* captured black piece */
    if (is_pawn(p))
      Result= col + (nr_of_slack_rows_below_board+nr_rows_on_board-2)*onerow;
    else {
      if (!flagdiastip && TSTFLAG(pspec, FrischAuf)) {
        Result= (col
                 + (onerow
                    *(CondFlag[glasgow]
                      ? nr_of_slack_rows_below_board+1
                      : nr_of_slack_rows_below_board)));
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
        default: /* fairy piece */
          Result= (col
                   + (onerow
                      *(CondFlag[glasgow]
                        ? nr_of_slack_rows_below_board+1
                        : nr_of_slack_rows_below_board))); /* V3.39  TLi */
        }
    }
  }

  return(Result);
} /* rennormal */

square rendiametral(piece p, Flags pspec,
                    square j, square i, square ia,
                    couleur camp) {
  /* V3.44  TLi */
  return haut+bas - rennormal(p,pspec,j,i,ia,camp);
} /* rendiametral */

square renspiegel(piece p, Flags pspec,
                  square j, square i, square ia,
                  couleur camp)
{
  return rennormal(p, pspec, j, i, ia, advers(camp));
} /* renspiegel */

square rensuper(piece p, Flags pspec,
                square j, square i, square ia,
                couleur camp)
{
  return super[nbply];
} /* rensuper */
/**********  V3.1  TLi end  *********/
#ifdef DOS
# pragma warn +par
#endif

boolean is_pawn(piece p) /* V3.22  TLi */
{
  switch (abs(p)) {
  case Pawn:
  case BerolinaPawn:
  case SuperBerolinaPawn:
  case SuperPawn:  return True;
  default:   return False;
  }
} /* end of is_pawn */

void genrn_cast(void) {     /* V3.55  TLi */
  /* It works only for castling_supported == TRUE
     have a look at funtion verifieposition() in py6.c
  */

  boolean is_castling_possible;

  if (dont_generate_castling)
    return;

  if (TSTFLAGMASK(castling_flag[nbply],bl_castlings)>ke8_cancastle
      && e[square_e8]==roin 
      /* then the king on e8 and at least one rook can castle !!
         V3.55  NG */
      && !echecc(noir))
  {
    /* 0-0 */
    if (TSTFLAGMASK(castling_flag[nbply],blk_castling)==blk_castling
        && e[square_h8]==tn
        && e[square_f8]==vide
        && e[square_g8]==vide)
    {
      if (complex_castling_through_flag)  /* V3.80  SE */
      {
        numecoup sic_nbcou= nbcou;
        empile(square_e8,square_f8,square_f8);
        if (nbcou>sic_nbcou)
        {
          boolean ok= jouecoup() && !echecc(noir);
          repcoup();
          if (ok)
            empile(square_e8,square_g8,kingside_castling);
        }
      }
      else
      {
        e[square_e8]= vide;
        e[square_f8]= roin;
        rn= square_f8;

        is_castling_possible= !echecc(noir);

        e[square_e8]= roin;
        e[square_f8]= vide;
        rn= square_e8;

        if (is_castling_possible)
          empile(square_e8,square_g8,kingside_castling);  /* V3.55  TLi */
      }
    }

    /* 0-0-0 */
    if (TSTFLAGMASK(castling_flag[nbply],blq_castling)==blq_castling
        && e[square_a8]==tn
        && e[square_d8]==vide
        && e[square_c8]==vide
        && e[square_b8]==vide)
    {
      if (complex_castling_through_flag)  /* V3.80  SE */
      {
        numecoup sic_nbcou= nbcou;
        empile(square_e8,square_d8,square_d8);
        if (nbcou>sic_nbcou)
        {
          boolean ok= (jouecoup() && !echecc(noir));
          repcoup();
          if (ok)
            empile(square_e8,square_c8,queenside_castling);
        }
      }
      else
      {
        e[square_e8]= vide;
        e[square_d8]= roin;
        rn= square_d8;
        
        is_castling_possible= !echecc(noir);
        
        e[square_e8]= roin;
        e[square_d8]= vide;
        rn= square_e8;

        if (is_castling_possible)
          empile(square_e8,square_c8,queenside_castling);  /* V3.55  TLi */
      }
    }
  }
} /* genrn_cast */

void genrn(square sq_departure) {
  numvec k;
  boolean flag = false;  /* K im Schach ? */
  numecoup anf, l1, l2;

  VARIABLE_INIT(anf);

  if (CondFlag[blrefl_king] && !calctransmute) {
    /* K im Schach zieht auch */
    piece *ptrans;

    anf= nbcou;
    calctransmute= true;
    for (ptrans= transmpieces; *ptrans; ptrans++) {
      if (nbpiece[*ptrans]
          && (*checkfunctions[*ptrans])(sq_departure,*ptrans,eval_black))
      {
        flag = true;
        current_trans_gen=-*ptrans;
        gen_bl_piece(sq_departure,-*ptrans);
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

  if (CondFlag[sting])    /* V3.63  NG */
    gerhop(sq_departure,vec_queen_start,vec_queen_end,noir);

  for (k= vec_queen_end; k>=vec_queen_start; k--) {   /* V2.4c  NG */
    square sq_arrival= sq_departure+vec[k];
    if (e[sq_arrival]==vide || e[sq_arrival]>=roib)
      empile(sq_departure,sq_arrival,sq_arrival);
  }
  
  if (flag) {
    /* testempile nicht nutzbar */
    /* VERIFY: has anf always a propper value??
     */
    for (l1= anf+1; l1<=nbcou; l1++)
      if (move_generation_stack[l1].arrival != initsquare)
        for (l2= l1+1; l2<=nbcou; l2++)
          if (move_generation_stack[l1].arrival
              ==move_generation_stack[l2].arrival)
            move_generation_stack[l2].arrival= initsquare;
  }

  /* Now we test castling */    /* V3.35  NG */
  if (castling_supported)
    genrn_cast();
}

void gen_bl_ply(void) {
  square i, j, z;
  piece p;

  /* Don't try to "optimize" by hand. The double-loop is tested as the  */
  /* fastest way to compute (due to compiler-optimizations !) V3.14  NG */
  z= haut;     /* V2.90  NG */
  for (i= nr_rows_on_board; i > 0; i--, z-= onerow-nr_files_on_board) /* V2.90  NG */
    for (j= nr_files_on_board; j > 0; j--, z--) {   /* V2.90  NG */
      if ((p = e[z]) != vide) {
        if (TSTFLAG(spec[z], Neutral))
          p = -p;    /* V1.4c  NG */
        if (p < vide)    /* V2.70  TLi */
          gen_bl_piece(z, p);
      }
    }
  if (blacknull)
    empile(nullsquare, nullsquare, nullsquare);
} /* gen_bl_ply */

void gen_bl_piece_aux(square z, piece p) { /* V3.46  SE/TLi */

  if (CondFlag[annan]) {
    piece annan_p= e[z+onerow];
    /*    if (annan_p < vide) */
    if (blannan(z+onerow, z))
      p= annan_p;
  }

  switch(p) {
  case roin: genrn(z);
    break;
  case pn: genpn(z);
    break;
  case cn: genleap(z, vec_knight_start,vec_knight_end);
    break;
  case tn: genrid(z, vec_rook_start,vec_rook_end); /* V2.60  NG */
    break;
  case dn: genrid(z, vec_queen_start,vec_queen_end); /* V2.60  NG */
    break;
  case fn: genrid(z, vec_bishop_start,vec_bishop_end); /* V2.60  NG */
    break;
  default: gfeernoir(z, p);
    break;
  }
} /* gen_bl_piece_aux */

static void orig_gen_bl_piece(square sq_departure, piece p) { /* V3.71 TM */
  piece pi_departing;

  if (flag_madrasi) {     /* V3.60  TLi */
    if (!libre(sq_departure, true)) {    /* V3.44  TLi */
      return;
    }
  }

  if (TSTFLAG(PieSpExFlags,Paralyse)) {        /* V2.90c  TLi */
    if (paralysiert(sq_departure)) {
      return;
    }
  }

  if (anymars||anyantimars) {         /* V3.46  SE/TLi */
    square sq_rebirth;
    Flags spec_departing;

    if (CondFlag[phantom]) {    /* V3.47  NG */
      numecoup     anf1, anf2, l1, l2;
      anf1= nbcou;
      /* generate standard moves first */
      flagactive= false;
      flagpassive= false;
      flagcapture= false;

      gen_bl_piece_aux(sq_departure,p);

      /* Kings normally don't move from their rebirth-square */
      if (p == e[rn] && !rex_phan) {
        return;
      }
      /* generate moves from rebirth square */
      flagactive= true;
      spec_departing=spec[sq_departure];
      sq_rebirth= (*marsrenai)(p,spec_departing,sq_departure,initsquare,initsquare,blanc);
      /* if rebirth square is where the piece stands,
         we've already generated all the relevant moves.
      */
      if (sq_rebirth==sq_departure) {
        return;
      }
      if (e[sq_rebirth] == vide) {
        anf2= nbcou;
        pi_departing=e[sq_departure];   /* Mars/Neutral bug V3.50 SE */
        e[sq_departure]= vide;
        spec[sq_departure]= EmptySpec;
        spec[sq_rebirth]= spec_departing;
        e[sq_rebirth]= p;
        marsid= sq_departure;

        gen_bl_piece_aux(sq_rebirth, p);

        e[sq_rebirth]= vide;
        spec[sq_departure]= spec_departing;
        e[sq_departure]= pi_departing;
        flagactive= false;
        /* Unfortunately we have to check for
           duplicate generated moves now.
           there's only ONE duplicate per arrival field
           possible !
        */
        for (l1= anf1 + 1; l1 <= anf2; l1++) {
          for (l2= anf2 + 1; l2 <= nbcou; l2++) {
            if (move_generation_stack[l1].arrival
                == move_generation_stack[l2].arrival) {
              move_generation_stack[l2].arrival= initsquare;
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

      gen_bl_piece_aux(sq_departure, p);

      /* generate capturing moves now */
      flagpassive= false;
      flagcapture= true;
      more_ren=0;
      do {   /* V3.50 SE Echecs Plus */
        spec_departing= spec[sq_departure];
        sq_rebirth= (*marsrenai)(p,
                                 spec_departing,
                                 sq_departure,
                                 initsquare,
                                 initsquare,
                                 blanc);
        if (sq_rebirth==sq_departure || e[sq_rebirth]==vide) {
          pi_departing= e[sq_departure]; /* Mars/Neutral bug V3.50 SE */
          
          e[sq_departure]= vide;
          spec[sq_departure]= EmptySpec;
          
          spec[sq_rebirth]= spec_departing;
          e[sq_rebirth]= p;
          
          marsid= sq_departure;

          gen_bl_piece_aux(sq_rebirth,p);

          e[sq_rebirth]= vide;
          
          spec[sq_departure]= spec_departing;
          e[sq_departure]= pi_departing;
        }
      } while (more_ren);
      flagcapture= false;
    }
  }
  else
    gen_bl_piece_aux(sq_departure,p);

  if (CondFlag[messigny] && !(rn==sq_departure && rex_mess_ex)) {
    /* V3.55  TLi */

    square *bnp;
    for (bnp= boardnum; *bnp; bnp++)
      if (e[*bnp]==-p)
        empile(sq_departure,*bnp,messigny_exchange);
  }
} /* orig_gen_bl_piece */

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

  /* Abbruch waehrend der gesammten Laufzeit mit <ESC> V2.70 TLi */
#ifdef ATARI
# include <osbind.h>
# define STOP_ON_ESC
# define interupt (Bconstat(2) && (Bconin(2) == 27))
#endif /* ATARI */

#ifdef DOS
#   ifndef Windows
# ifdef __TURBOC__
# define STOP_ON_ESC
# define interupt ((bioskey(1) != 0) && ((bioskey(0) >> 8) == 1))
# endif /* __TURBOC__ */
#   endif

# ifdef MSC
# define STOP_ON_ESC
# define interupt (_bios_keybrd(_KEYBRD_READY) && ((_bios_keybrd(_KEYBRD_READ) >> 8) == 1))
# endif /* MSC */

# ifdef GCC     /* V3.11  NG */
# define STOP_ON_ESC
# define interupt (kbhit() && (getkey() == 27)) /* ESC == 27 */
# endif /* GCC */
#endif /* DOS */

#ifdef OS2
# ifdef GCC     /* V3.11  NG */
# endif /* GCC */
#endif /* OS2 */

#ifdef STOP_ON_ESC
  if (interupt) {
    StdString(GetMsgString(InterMessage));
    StdString(" ");
    PrintTime();  /* V2.90  NG */
    StdString("\n\n");
    CloseInput();    /* V2.90  NG */
    /* for some other purposes I need a return value
    ** different from 0.  TLi
    ** exit(0);
    */
    exit(1);
  }
#endif /* STOP_ON_ESC */

  if (TSTFLAG(PieSpExFlags,Neutral))
    initneutre(advers(camp));
  nextply();
  trait[nbply]= camp;
  /* flagminmax= false;   V2.90, V3.44  TLi */
  /* flag_minmax[nbply]= false;         V3.44  TLi */
  we_generate_exact = false;      /* V3.20  TLi */
  init_move_generation_optimizer();

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
    if (wh_exact) {      /* V3.20  TLi */
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
        /* flagminmax= false;   V3.44  TLi */
        /* flag_minmax[nbply]= false;  V3.44 TLi */
        init_move_generation_optimizer();
        gen_wh_ply();
        /* Puh - let's pray ! */
      }
      we_generate_exact = false;
    } else
      gen_wh_ply();
  } else {
    /* let's first generate consequent moves */
    if (bl_exact) {      /* V3.20  TLi */
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
        /* flagminmax= false;   V3.44  TLi */
        /* flag_minmax[nbply]= false; V3.44  TLi */
        init_move_generation_optimizer();
        gen_bl_ply();
      }
      we_generate_exact = false;
    } else
      gen_bl_ply();
  }
  finish_move_generation_optimizer();

} /* genmove(camp) */

void joueparrain(void)        /* H.D. 10.02.93 */
{
  piece p= pprise[nbply-1];
  Flags pspec= pprispec[nbply-1];
  square cren= move_generation_stack[repere[nbply]].capture + move_generation_stack[nbcou].arrival - move_generation_stack[nbcou].departure;

  if (e[cren] == vide) {
    sqrenais[nbply]= cren;
    ren_parrain[nbply]= e[cren]= p;
    spec[cren]= pspec;

    if (is_pawn(p) && PromSq(advers(trait[nbply-1]), cren)) {  /* V3.43  TLi */
      /* captured white pawn on eighth rank: promotion ! */
      /* captured black pawn on first rank: promotion ! */
      piece pprom= cir_prom[nbply];

      if (TSTFLAG(pspec, Chameleon))        /* V3.46  TLi */
        cir_cham_prom[nbply]= true;
      if (pprom == vide)
        cir_prom[nbply]= pprom= getprompiece[vide];
      if (p < vide)
        pprom= -pprom;
      e[cren]= pprom;
      nbpiece[pprom]++;
      if (cir_cham_prom[nbply])  /* V3.1  TLi */
        SETFLAG(pspec, Chameleon);
      spec[cren]= pspec;
    } else
      nbpiece[p]++;
    if (TSTFLAG(pspec, Neutral)) /* V3.65  SE */ /* bug reported by Kevin Begley */
      setneutre(cren);
  }
} /* end of joueparrain */

boolean testdblmate= False;  /* V3.50 SE */
piece pdisp[maxply+1];
Flags pdispspec[maxply+1];
square sqdep[maxply+1];

boolean patience_legal()     /* V3.50 SE */
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

int direction(square from, square to) {   /* V3.65  TLi */
  int dir= to-from;
  int hori= to%onerow-from%onerow;
  int vert= to/onerow-from/onerow;
  int i=7;
  while ((hori%i) || (vert%i))
    i--;

  return dir/i;
} /* direction */

/* AMU V3.70 SE */
square blpc;

boolean eval_spec(square sq_departure, square sq_arrival, square sq_capture) {
  return sq_departure==blpc;
}

boolean att_once(square id)
{
  int i,j, cnt=0;
  square z=bas;
  piece p;
  square rb_=rb;
  rb=id ;

  for (i= 8; i > 0; i--, z+= 16)  /* V2.90  NG */
    for (j= 8; j > 0; j--, z++) {  /* V2.90  NG */
      if ((p = e[z]) != vide) {
        if (p < -obs)
        {
          blpc=z;
          if (rbechec(eval_spec))
            if (!++cnt)
              break;      /* could modify to return int no. of attacks */
        }
      }
    }
  rb=rb_;
  return cnt==1;
}

square next_latent_pawn(square s, couleur c) { /* V3.71 TM */
  piece pawn;
  int  i, delta;

  pawn=  c==blanc ? pb : pn;
  delta= c==blanc ?+dir_left :+dir_right;

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

void jouecoup_no_test(void)
{
  jouetestgenre= false;
  jouecoup();
  jouetestgenre= jouetestgenre_save;
}

boolean jouecoup_ortho_test(void)
{
  boolean flag;
  boolean jtg1= jouetestgenre1; 
  jouetestgenre1= false;
  flag= jouecoup();
  jouetestgenre1= jtg1;
  return flag;
}

boolean jouecoup_legality_test(smallint oldnbpiece[derbla], square cren) {
  if (CondFlag[schwarzschacher] && trait[nbply]==noir)
    return echecc(blanc);  /* V3.62  SE */

  if (CondFlag[extinction]) {
    piece p;
    for (p= roib; p<derbla; p++) {
      if (oldnbpiece[p]
          && !nbpiece[trait[nbply]==blanc ? p : -p])
      {
        return false;
      }
    }
  }

  return (!jouetestgenre                                /* V3.50 SE */
          || (
            (!jouetestgenre1 || (
               (!CondFlag[blackultraschachzwang]
                || trait[nbply]==blanc
                || echecc(blanc))
               && (!CondFlag[whiteultraschachzwang]
                   || trait[nbply]==noir
                   || echecc(noir))
              ))
            &&
            ((!flag_testlegality) || pos_legal())
            /* V3.44, 3.51  SE/TLi */
            && (!flagAssassin || (cren != rb && cren != rn))
            /* V3.50 SE */
            && (!testdblmate || (rb!=initsquare && rn!=initsquare))
            /* V3.50 SE */
            && (!CondFlag[patience] || PatienceB || patience_legal())
            /* V3.50 SE */
            /* don't call patience_legal if TypeB as obs > vide ! */
            && (trait[nbply] == blanc ? BGL_white >= 0 : BGL_black >= 0) /* V4
                                                                            .06 SE */
            ));
}

boolean jouecoup(void) {
  square  sq_departure,
    sq_arrival,
    sq_capture,
    sq_rebirth= initsquare,  /* initialised !  V3.50  NG */
    sq_hurdle= initsquare,/* V3.64 SE */
    prev_rb,
    prev_rn;
  piece   pi_captured,
    pi_departing,
    pi_arriving,
    pi_reborn,
    pi_hurdle;
  Flags   spec_pi_captured;
  Flags   spec_pi_moving;

  smallint prev_nbpiece[derbla];    /* V3.50 SE */

  couleur traitnbply= trait[nbply]; /* V3.1  TLi */

  move_generation_elmt* move_gen_top = move_generation_stack+nbcou;

#ifdef DEBUG
  nbrtimes++;
#endif

  /* Orphans/refl. KK !!!! V3.0  TLi */
  /* now also for phantomchess  V3.47  NG - schoen krampfig */

  if (jouegenre)
  {
    if (exist[Orphan]
        || exist[Friend]    /* V3.65  TLi */
        || CondFlag[whrefl_king]
        || CondFlag[blrefl_king]
        || CondFlag[phantom]) {
      while (move_generation_stack[nbcou].arrival == initsquare) {
        nbcou--;
      }
      move_gen_top = move_generation_stack+nbcou;
    }

    if (CondFlag[extinction]) {
      piece p;
      for (p= roib; p < derbla; p++) {
        prev_nbpiece[p]= nbpiece[ traitnbply==blanc ? p : -p];
      }
    }
  }

  prev_rb= RB_[nbply]= rb; /* H.D. 10.02.93 */
  prev_rn= RN_[nbply]= rn;

  sq_departure= sqdep[nbply]= move_gen_top->departure;   /* V2.90  TLi */
  sq_arrival= move_gen_top->arrival;   /* V2.90  TLi */
  sq_capture= move_gen_top->capture;

  if (jouegenre)
  {
    if (CondFlag[amu])
      att_1[nbply]= att_once(sq_departure);

    if (CondFlag[imitators])    /* V2.4d  TM */
    {
      if (sq_capture == queenside_castling)
        joueim(+dir_right);
      else if (sq_capture!=kingside_castling) /* joueim(0) (do nothing) if OO */
        joueim(sq_arrival-sq_departure);
    }
  }

  spec_pi_moving= jouespec[nbply]= spec[sq_departure]; /* V3.1  TLi */
  pi_arriving= pi_departing= pjoue[nbply]= e[sq_departure]; /* V2.4c  NG */

  spec_pi_captured= pprispec[nbply]= spec[sq_capture];  /* V3.1  TLi */
  pi_captured= pprise[nbply]= e[sq_capture];    /* V3.1  TLi */

  if (sq_arrival==nullsquare)
    return true;      /* V3.70 SE */

  if (anyantimars && sq_departure==sq_capture)
  {
    spec_pi_captured= pprispec[nbply]= 0;
    pi_captured= pprise[nbply]= vide;
  }
  
  pdisp[nbply]= vide;     /* V3.50 SE */
  pdispspec[nbply]= 0;

  if (jouegenre)
  {
    if (CondFlag[blsupertrans_king]
        && traitnbply==noir
        && ctrans[nbcou]!=vide)
    {
      rn=initsquare;
      pi_arriving=ctrans[nbcou];
    }
    if (CondFlag[whsupertrans_king]
        && traitnbply==blanc
        && ctrans[nbcou]!=vide)
    {
      rb=initsquare;
      pi_arriving=ctrans[nbcou];
    }

    if (CondFlag[singlebox] && SingleBoxType==singlebox_type3
        && sb3[nbcou].what!=vide) {  /* V3.71 TM */
      --nbpiece[e[sb3[nbcou].where]];
      e[sb3[nbcou].where] = sb3[nbcou].what;
      ++nbpiece[sb3[nbcou].what];
      if (sq_departure==sb3[nbcou].where) {
        pi_departing = pi_arriving = sb3[nbcou].what;
      }
    }
  }

  if (TSTFLAG(spec_pi_moving, ColourChange)) {     /* V3.64 SE, V3.65  NG */
    sq_hurdle= chop[nbcou];     /* V3.64 SE */
  }

  switch (sq_capture) {   /* V3.55  TLi */

  case messigny_exchange:
    pprise[nbply]= e[sq_departure]= e[sq_arrival];
    pprispec[nbply]= spec[sq_departure]= spec[sq_arrival];
    jouearr[nbply]= e[sq_arrival]= pi_departing;
    spec[sq_arrival]= spec_pi_moving;
    if (rb == sq_departure) {
      rb= sq_arrival;
    }
    else {
      if (rb == sq_arrival)
        rb= sq_departure;
    }
    if (rn == sq_departure) {
      rn= sq_arrival;
    }
    else {
      if (rn == sq_arrival) {
        rn= sq_departure;
      }
    }
    
    return jouecoup_legality_test(prev_nbpiece,sq_rebirth);
    
  case kingside_castling:
    if (CondFlag[einstein]) {   /* V3.44  NG */
      if (sq_departure == square_e1) {
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
    else
    {
      e[sq_departure+dir_right]= e[sq_departure+3*dir_right];
    }
    spec[sq_departure+dir_right]= spec[sq_departure+3*dir_right];
    e[sq_departure+3*dir_right]= vide;
    CLEARFL(spec[sq_departure+3*dir_right]);
    if (sq_departure == square_e1) {
      CLRFLAGMASK(castling_flag[nbply],whk_castling);
    }
    else {
      CLRFLAGMASK(castling_flag[nbply],blk_castling);
    }
    break;

  case queenside_castling:
    if (CondFlag[einstein]) {   /* V3.44  NG */
      if (sq_departure == square_e1) {       /* white */
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
      else {     /* black */
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
    else
    {
      e[sq_departure+dir_left]= e[sq_departure+4*dir_left];
    }
    spec[sq_departure+dir_left]= spec[sq_departure+4*dir_left];
    e[sq_departure+4*dir_left]= vide;
    CLEARFL(spec[sq_departure+4*dir_left]);
    if (sq_departure == square_e1) {
      CLRFLAGMASK(castling_flag[nbply],whq_castling);
    }
    else {
      CLRFLAGMASK(castling_flag[nbply],blq_castling);
    }
    break;
  } /* switch (sq_capture) */

  e[sq_departure]= CondFlag[haanerchess] ? obs : vide;
  spec[sq_departure]= 0;

  if (PatienceB) {     /* V3.50 SE */
    ply nply;
    e[sq_departure]= obs;
    for (nply= nbply - 1 ; nply > 1 ; nply--) {
      if (trait[nply] == traitnbply) {
        e[sqdep[nply]]= vide;
      }
    }
  }

  if (change_moving_piece) {
    /* Now pawn-promotion (even into 'kamikaze'-pieces)
       is possible !  NG */
    if (TSTFLAG(spec_pi_moving, Kamikaze)) {     /* V1.3c  NG, V2.4d  TM */
      if (pi_captured != vide) {    /* V2.4c  NG */
        if (!anycirce) {     /* V2.4d  TM , V3.1  TLi */
          spec_pi_moving= 0;
          pi_arriving= vide;
          /* to avoid useless promotions of a
             vanishing pawn */
        }
      }
    }
    if (CondFlag[linechamchess])   /* V3.64  TLi */
      pi_arriving= linechampiece(pi_arriving, sq_arrival);

    if (CondFlag[chamchess])   /* V3.32  TLi */
      pi_arriving= champiece(pi_arriving);

    if (TSTFLAG(spec_pi_moving, Chameleon))    /* V3.1  TLi */
      pi_arriving= champiece(pi_arriving);

    if (CondFlag[norsk])     /* V3.1  TLi */
      pi_arriving= norskpiece(pi_arriving);

  } /* change_moving_piece */

  if (abs(pi_departing) == andergb) {    /* V3.65  TLi */
    square sq= sq_arrival - direction(sq_departure, sq_arrival);

    if (!TSTFLAG(spec[sq], Neutral) && (sq != rb) && (sq != rn)) {
      change(sq);
      CHANGECOLOR(spec[sq]);
    }
  } /* andergb */

  ep[nbply]= ep2[nbply]= initsquare;   /* V3.37  TLi */
  if (is_pawn(pi_departing)) {
    if (pi_captured==vide) {
      /* ep capture  V3.22  TLi */
      if (CondFlag[phantom]) {    /* V3.52  NG */
        smallint col_diff, rank_j;

        col_diff= sq_arrival%onerow - sq_departure%onerow,
          rank_j= sq_arrival/onerow;

        if (rank_j == 11) { /* 4th rank */
          switch (col_diff) {
          case 0:
            if (pi_departing==pb
                && sq_departure!=sq_arrival+dir_down)  /* V3.53  NG */
              ep[nbply]= sq_arrival+dir_down;
            break;

          case -2:
            if (pi_departing==pbb
                && sq_departure!=sq_arrival+dir_down+dir_left)/* V3.53  NG */
              ep[nbply]= sq_arrival+dir_down+dir_left;
            break;

          case 2:
            if (pi_departing==pbb
                && sq_departure!=sq_arrival+dir_down+dir_right)/* V3.53  NG */
              ep[nbply]= sq_arrival+dir_down+dir_right;
            break;
          } /* switch (col_diff) */
        }
        else if (rank_j == 12) { /* 5th rank */
          switch (col_diff) {
          case 0:
            if (pi_departing==pn
                && sq_departure!=sq_arrival+dir_up)  /* V3.53  NG */
              ep[nbply]= sq_arrival+dir_up;
            break;
          case -2:
            if (pi_departing==pbn
                && sq_departure!=sq_arrival+dir_up+dir_left) /* V3.53  NG */
              ep[nbply]= sq_arrival+dir_up+dir_left;
            break;
          case 2:
            if (pi_departing==pbn
                && sq_departure!=sq_arrival+dir_up+dir_right) /* V3.53  NG */
              ep[nbply]= sq_arrival+dir_up+dir_right;
            break;
          } /* switch (col_diff) */
        }
      } /* CondFlag[phantom] */
      else
      {
        square ii = anyantimars ? cmren[nbcou] : sq_departure;
        switch (abs(pi_departing)) {
        case Pawn:
          switch (abs(ii - sq_arrival)) {
          case 2*onerow: /* ordinary or Einstein double step */
            ep[nbply]= (ii + sq_arrival) / 2;
            break;
          case 3*onerow: /* Einstein triple step */
            ep[nbply]= (ii + sq_arrival + sq_arrival) / 3;
            ep2[nbply]= (ii + ii + sq_arrival) / 3;
            break;
          } /* end switch (abs(ii-sq_arrival)) */
          break;
        case BerolinaPawn:
          if (abs(ii - sq_arrival) > onerow+1) {
            /* It's a double step! */
            ep[nbply]= (ii + sq_arrival) / 2;
          }
          break;
        } /* end switch (abs(pi_departing)) */
      }

    }
    /* promotion */
    if (PromSq(traitnbply, sq_arrival)) {   /* V3.39  TLi */
      /* moved to here because of anticirce
       */
      if ((pi_arriving= norm_prom[nbply]) == vide) { /* V3.02  TLi */
        if (!CondFlag[noiprom] && Iprom[nbply]) {   /* V2.4d  TM */
          ply icount;
          if (inum[nbply] == maxinum)
            FtlMsg(ManyImitators);
          for (icount = nbply; icount <= maxply; inum[icount++]++)
            ;
          isquare[inum[nbply]-1] = sq_arrival;
        }
        else
        {
          if (CondFlag[singlebox])
          {
            pi_arriving = next_singlebox_prom(vide,traitnbply);
            assert(SingleBoxType!=singlebox_type1 || pi_arriving!=vide);
            if (pi_arriving==vide)
              /* pi_arriving will be recolored later if pi_departing
               * is black! */
              pi_arriving = abs(pi_departing);  /* V3.76  ThM */
          }
          else
          {
            pi_arriving= getprompiece[vide];

            if (CondFlag[frischauf])    /* V3.41b  TLi */
              SETFLAG(spec_pi_moving, FrischAuf);

            /* V3.1  TLi */
            if (pi_captured != vide && anyanticirce) {
#ifdef BETTER_READABLE
              /* this coding seems to be better redable */
              do {
                sq_rebirth= (*antirenai)(pi_arriving,
                                         spec_pi_moving,
                                         sq_capture,
                                         sq_departure,
                                         advers(traitnbply));
                if (sq_rebirth == sq_departure)
                  break;
                if (LegalAntiCirceMove(sq_rebirth, sq_capture, sq_departure))
                  /* V3.62  TLi */
                  break;
                pi_arriving= getprompiece[pi_arriving];
              } while (1);
#endif /*BETTER_READABLE*/

              while (((sq_rebirth= (*antirenai)(pi_arriving,
                                                spec_pi_moving,
                                                sq_capture,
                                                sq_departure,
                                                sq_arrival,
                                                advers(traitnbply)))
                      != sq_departure)
                     && !LegalAntiCirceMove(sq_rebirth,
                                            sq_capture,
                                            sq_departure))  /* V3.62  TLi */
              {
                pi_arriving= getprompiece[pi_arriving];
                if (!pi_arriving && CondFlag[antisuper])
                {
                  super[nbply]++;
                  pi_arriving= getprompiece[vide];
                }
              }
            }
          }
          norm_prom[nbply]= pi_arriving;
        }
      }
      else
      {           /* V3.41b  TLi */
        if (CondFlag[frischauf])        /* V3.41b  TLi */
          SETFLAG(spec_pi_moving, FrischAuf);
      }

      if (!(!CondFlag[noiprom] && Iprom[nbply])) {
        if (TSTFLAG(spec_pi_moving, Chameleon)
            && is_pawn(pi_departing)) { /* V3.44  TLi */
          norm_cham_prom[nbply]= true;
        }
        if (pi_departing < vide) {
          pi_arriving = -pi_arriving;
        }

        /* so also promoted neutral pawns
         * have the correct color and an
         * additional call to setneutre is
         * not required. TLi
         */
        /* V3.22  TLi
        ** pend2[nbply]= initsquare; */  /* V3.1  TLi */
        if (norm_cham_prom[nbply])  /* V3.1  TLi */
          SETFLAG(spec_pi_moving, Chameleon);
      }
      else {
        pi_arriving= vide; /* imitator-promotion */
      }
    }
  } /* is_pawn() */

  /* V3.02  TLi */
  if (sq_arrival != sq_capture) {
    e[sq_capture]= vide;
    spec[sq_capture]= 0;
  }

  if (pi_captured != vide) {         /* V2.4c NG, V3.1  TLi */
    /* V3.22  TLi
    ** pend2[nbply]= initsquare;  V3.22  TLi */  /* V3.1  TLi */
    nbpiece[pi_captured]--;    /* V1.4c  NG */

    if (sq_capture == rb)    /* V3.44  SE/TLi */
      rb= initsquare;   /* V3.44  SE/TLi */
    if (sq_capture == rn)    /* V3.44  SE/TLi */
      rn= initsquare;   /* V3.44  SE/TLi */
  }

  if (change_moving_piece) {   /* V3.1  TLi */
    if (CondFlag[degradierung]  /* V3.1  TLi */
        && !is_pawn(pi_departing)
        && sq_departure != prev_rn
        && sq_departure != prev_rb
        && (traitnbply == noir
            ? sq_arrival>=square_a7 && sq_arrival<=square_h7
            : sq_arrival>=square_a2 && sq_arrival<=square_h2))
    {
      if (pi_departing < vide)
        pi_arriving= pn;
      else
        pi_arriving= pb;
    }

    if ((CondFlag[tibet]
         && pi_captured != vide
         && pi_arriving != -pi_captured     /* V2.90 TLi */
         && (((traitnbply == noir)
              && (sq_departure != prev_rn))
             || ((traitnbply == blanc)
                 && CondFlag[dbltibet]
                 && (sq_departure != prev_rb))))
        || (CondFlag[andernach]
            && pi_captured != vide
            && sq_departure != prev_rn
            && sq_departure != prev_rb))
    {
      SETCOLOR(spec_pi_moving, spec_pi_captured);
      if (CondFlag[volage] && !CondFlag[hypervolage])
        CLRFLAG(spec_pi_moving, Volage);

      /* nasty neutral implementation */
      if (pi_arriving < vide)
        pi_arriving= -pi_arriving;

      /* now the piece is white */
      /* has it to be changed? */
      if (TSTFLAG(spec_pi_moving, Black)
          && (!TSTFLAG(spec_pi_moving, White) || neutcoul == noir))
      {
        pi_arriving= -pi_arriving;
      }
    }

    /* this seems to be misplaced -- it's checked every time TLi */
    /* therefore moved to this place !     V3.60  NG */
    if (CondFlag[antiandernach]
        && pi_captured == vide
        && sq_departure != prev_rn
        && sq_departure != prev_rb )
    {
      /* the following also copes correctly with neutral
      ** pieces V3.62  TLi */
      CLRFLAG(spec_pi_moving, Black);
      CLRFLAG(spec_pi_moving, White);
      CLRFLAG(spec_pi_moving, Neutral);
      if (traitnbply == noir) {
        SETFLAG(spec_pi_moving, White);
        pi_arriving= abs(pi_arriving);
      }
      else {
        SETFLAG(spec_pi_moving, Black);
        pi_arriving= -abs(pi_arriving);
      }
    } /* CondFlag[antiandernach] ... */

    /* V3.33  TLi */
    if ((CondFlag[traitor]
         && traitnbply == noir    /* V3.1  TLi */
         && sq_arrival<=square_h4
         && !TSTFLAG(spec_pi_moving, Neutral))
        || (TSTFLAG(spec_pi_moving, Volage)
            && SquareCol(sq_departure) != SquareCol(sq_arrival))
        || (TSTFLAG(sq_spec[sq_arrival], MagicSq)
            && prev_rn != sq_departure
            && prev_rb != sq_departure))
    {
      CHANGECOLOR(spec_pi_moving);
      if (/* CondFlag[volage] && */ !CondFlag[hypervolage])
        CLRFLAG(spec_pi_moving, Volage);
      pi_arriving= -pi_arriving;
    }

    if (CondFlag[einstein]    /* V3.1  TLi */
        && !(CondFlag[antieinstein] && pi_captured != vide)) /* V3.50  TLi */
    {
      pi_arriving= (pi_captured==vide) != CondFlag[reveinstein]
        ? dec_einstein(pi_arriving)
        : inc_einstein(pi_arriving);
    }

  } /* change_moving_piece */

  if (sq_departure == prev_rb) {     /* V2.1c  NG */
    if (rb!=initsquare)
      rb= sq_arrival;
    CLRFLAGMASK(castling_flag[nbply],ke1_cancastle);
  }
  if (sq_departure == prev_rn) {
    if (rn!=initsquare)
      rn= sq_arrival;
    CLRFLAGMASK(castling_flag[nbply],ke8_cancastle);
  }

  /* Needed for castling */    /* V3.35  NG */
  if (castling_supported) {
    /* pieces vacating a1, h1, a8, h8    V3.77  TLi */
    if (sq_departure == square_h1) {
      CLRFLAGMASK(castling_flag[nbply],rh1_cancastle);
    }
    else if (sq_departure == square_a1) {
      CLRFLAGMASK(castling_flag[nbply],ra1_cancastle);
    }
    else if (sq_departure == square_h8) {
      CLRFLAGMASK(castling_flag[nbply],rh8_cancastle);
    }
    else if (sq_departure == square_a8) {
      CLRFLAGMASK(castling_flag[nbply],ra8_cancastle);
    }
    /* pieces arriving at a1, h1, a8, h8 and possibly capturing a rook */ /* V3.77  TLi */
    if (sq_arrival == square_h1) {
      CLRFLAGMASK(castling_flag[nbply],rh1_cancastle);
    }
    else if (sq_arrival == square_a1) {
      CLRFLAGMASK(castling_flag[nbply],ra1_cancastle);
    }
    else if (sq_arrival == square_h8) {
      CLRFLAGMASK(castling_flag[nbply],rh8_cancastle);
    }
    else if (sq_arrival == square_a8) {
      CLRFLAGMASK(castling_flag[nbply],ra8_cancastle);
    }
  }     /* castling_supported */

  if ((CondFlag[andernach] && pi_captured!=vide)
      || (CondFlag[antiandernach] && pi_captured==vide)  /* V3.60 SE */
      || (CondFlag[norsk])    /* V3.74  NG */
    )
  {
    if (castling_supported) {
      if (abs(pi_arriving) == Rook) {           /* V3.77  TLi */
        if (TSTFLAG(spec_pi_moving, White)) { /* V3.77  TLi */
          /* new white/neutral rook */
          if (sq_arrival == square_h1)             /* h1 */
            SETFLAGMASK(castling_flag[nbply],rh1_cancastle);
          else if (sq_arrival == square_a1)       /* a1 */
            SETFLAGMASK(castling_flag[nbply],ra1_cancastle);
        }
        if (TSTFLAG(spec_pi_moving, Black)) { /* V3.77  TLi */
          /* new black/neutral rook */
          if (sq_arrival == square_h8)       /* h8 */
            SETFLAGMASK(castling_flag[nbply],rh8_cancastle);
          else if (sq_arrival == square_a8)       /* a8 */
            SETFLAGMASK(castling_flag[nbply],ra8_cancastle);
        }
      }
    } /* castling_supported */
  } /* andernach || antiandernach ... */

  if (TSTFLAG(spec_pi_moving, HalfNeutral))      /* V3.57 SE */
  {
    if (TSTFLAG(spec_pi_moving, Neutral)) {
      CLRFLAG(spec_pi_moving,
              traitnbply == blanc ? Black : White);   /* V3.62  TLi */

      CLRFLAG(spec_pi_moving, Neutral);
      pi_arriving= traitnbply==noir ? -abs(pi_arriving) : abs(pi_arriving);

      if (rn == sq_arrival && traitnbply == blanc)
        rn= initsquare;

      if (rb == sq_arrival && traitnbply == noir)
        rb= initsquare;
    }
    else if (traitnbply==noir) {
      if (TSTFLAG(spec_pi_moving, Black)) {
        SETFLAG(spec_pi_moving, Neutral);
        SETFLAG(spec_pi_moving, White);  /* V3.62  TLi */
        pi_arriving= abs(pi_arriving);
        if (rn == sq_arrival)
          rb = sq_arrival;
      }
    }
    else if (traitnbply==blanc) {
      if (TSTFLAG(spec_pi_moving, White)) {
        SETFLAG(spec_pi_moving, Neutral);
        SETFLAG(spec_pi_moving, Black);  /* V3.62  TLi */
        pi_arriving= -abs(pi_arriving);
        if (rb == sq_arrival)
          rn = sq_arrival;
      }
    }
  }

  e[sq_arrival]= pi_arriving;
  spec[sq_arrival] = spec_pi_moving;       /* V3.1  TLi */
  jouearr[nbply]= pi_arriving;      /* V3.1  TLi */

  if (pi_departing != pi_arriving) {       /* V3.1  TLi */
    nbpiece[pi_departing]--;
    nbpiece[pi_arriving]++;
  }

  if (jouegenre) {        /* V2.4c  NG */
    if (TSTFLAG(spec_pi_moving, HalfNeutral)
        && TSTFLAG(spec_pi_moving, Neutral))   /* V3.57 SE */
      setneutre(sq_arrival);

    /* Duellantenschach   V3.0  TLi */
    if (CondFlag[duellist]) {
      if (traitnbply == noir) {
        whduell[nbply]= whduell[nbply - 1];
        blduell[nbply]= sq_arrival;
      }
      else {
        blduell[nbply]= blduell[nbply - 1];
        whduell[nbply]= sq_arrival;
      }
    }

    if (CondFlag[singlebox] && SingleBoxType==singlebox_type2) { /* 3.71 TM */
      couleur adv = advers(traitnbply);

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

    /* AntiCirce  V3.1  TLi */
    if (pi_captured != vide && anyanticirce) {
      sq_rebirth= (*antirenai)(pi_arriving,
                               spec_pi_moving,
                               sq_capture,
                               sq_departure,
                               sq_arrival,
                               advers(traitnbply));
      if (CondFlag[antisuper])
      {
        while (!LegalAntiCirceMove(sq_rebirth, sq_capture, sq_departure))
          sq_rebirth++;
        super[nbply]= sq_rebirth;
      }
      e[sq_arrival]= vide;
      spec[sq_arrival]= 0;
      crenkam[nbply]= sq_rebirth;
      if (is_pawn(pi_departing)          /* V3.39  TLi */
          && PromSq(traitnbply, sq_rebirth))
      {
        /* white pawn on eighth rank or
           black pawn on first rank - promotion ! */
        nbpiece[pi_arriving]--;
        pi_arriving= norm_prom[nbply];
        if (pi_arriving == vide)
          norm_prom[nbply]= pi_arriving= getprompiece[vide];
        if (pi_departing < vide)
          pi_arriving= -pi_arriving;
        nbpiece[pi_arriving]++;
      }
      e[sq_rebirth]= pi_arriving;
      spec[sq_rebirth]= spec_pi_moving;
      if (sq_departure == prev_rb)
        rb= sq_rebirth;
      if (sq_departure == prev_rn)
        rn= sq_rebirth;

      if (castling_supported) {   /* V3.77  TLi */
        piece abspja= abs(pi_arriving);
        if (abspja == King) {
          if (TSTFLAG(spec_pi_moving, White)
              && sq_rebirth == square_e1
              && (!CondFlag[dynasty] || nbpiece[roib]==1)) { /* V4.02 TM */
            /* white king new on e1 */
            SETFLAGMASK(castling_flag[nbply],ke1_cancastle);
          }
          else if (TSTFLAG(spec_pi_moving, Black)
                   && sq_rebirth == square_e8
                   && (!CondFlag[dynasty] || nbpiece[roin]==1)) {/* V4.02 TM */
            /* black king new on e8 */
            SETFLAGMASK(castling_flag[nbply],ke8_cancastle);
          }
        }
        else if (abspja == Rook) {
          if (TSTFLAG(spec_pi_moving, White)) {
            /* new white/neutral rook */
            if (sq_rebirth == square_h1) {
              SETFLAGMASK(castling_flag[nbply],rh1_cancastle);
            }
            else if (sq_rebirth == square_a1) {
              SETFLAGMASK(castling_flag[nbply],ra1_cancastle);
            }
          }
          if (TSTFLAG(spec_pi_moving, Black)) {
            /* new black/neutral rook */
            if (sq_rebirth == square_h8) {
              SETFLAGMASK(castling_flag[nbply],rh8_cancastle);
            }
            else if (sq_rebirth == square_a8) {
              SETFLAGMASK(castling_flag[nbply],ra8_cancastle);
            }
          }
        }
      } /* castling_supported */
    } /* AntiCirce */

    if (CondFlag[sentinelles]) {   /* V2.90 TLi */
      if (sq_departure>=square_a2 && sq_departure<=square_h7
          && !is_pawn(pi_departing))      /* V3.64  NG,TLi */
      {
        if (SentPionNeutral) {
          if (TSTFLAG(spec_pi_moving, Neutral)) {
            nbpiece[e[sq_departure]= sentinelb]++;
            SETFLAG(spec[sq_departure], Neutral);
            setneutre(sq_departure);
          }
          else if ((traitnbply==noir) != SentPionAdverse) {
            nbpiece[e[sq_departure]= sentineln]++;
            SETFLAG(spec[sq_departure], Black);
          }
          else {
            nbpiece[e[sq_departure]= sentinelb]++;
            SETFLAG(spec[sq_departure], White);
          }
          if (   nbpiece[sentinelb] > max_pb
                 || nbpiece[sentineln] > max_pn
                 || nbpiece[sentinelb]+nbpiece[sentineln] > max_pt)
          { /* V3.60 SE */
            /* rules for sentinelles + neutrals not yet
               written but it's very likely this logic will
               need to be refined
            */
            nbpiece[e[sq_departure]]--;
            e[sq_departure]= vide;
            spec[sq_departure]= 0;
          }
          else {
            senti[nbply]= true;
          }
        }
        else if ((traitnbply==noir) != SentPionAdverse) {
          /* V3.50 SE */
          if (   nbpiece[sentineln] < max_pn
                 && nbpiece[sentinelb]+nbpiece[sentineln]<max_pt
                 && (  !flagparasent
                       ||(nbpiece[sentineln]
                          <= nbpiece[sentinelb]
                          +(pi_captured==sentinelb ? 1 : 0))))
          { /* V3.62 SE */
            nbpiece[e[sq_departure]= sentineln]++;
            SETFLAG(spec[sq_departure], Black); /* V3.39  TLi */
            senti[nbply]= true;
          }
        }
        else if ( nbpiece[sentinelb] < max_pb
                  && nbpiece[sentinelb]+nbpiece[sentineln]<max_pt
                  && (!flagparasent
                      ||(nbpiece[sentinelb]
                         <= nbpiece[sentineln]
                         + (pi_captured==sentineln ? 1 : 0))))/* V3.62 SE */
        {
          nbpiece[e[sq_departure]= sentinelb]++;
          SETFLAG(spec[sq_departure], White);  /* V3.39 TLi */
          senti[nbply]= true;
        }
      }
    }

    if (anycirce) {   /* V2.4c  NG */
      /* circe-rebirth of moving kamikaze-piece */
      if (TSTFLAG(spec_pi_moving, Kamikaze) && (pi_captured != vide)) {
        if (CondFlag[couscous]) {
          sq_rebirth= (*circerenai)(pi_captured,
                                    spec_pi_captured,
                                    sq_capture,
                                    sq_departure,
                                    sq_arrival,
                                    traitnbply);
        }
        else {
          sq_rebirth= (*circerenai)(pi_arriving,
                                    spec_pi_moving,
                                    sq_capture,
                                    sq_departure,
                                    sq_arrival,
                                    advers(traitnbply));
        }
        if (sq_rebirth != sq_arrival) {
          e[sq_arrival]= vide;
          spec[sq_arrival]= 0;
          if ((e[sq_rebirth] == vide)  /* V3.1  TLi */
              && !( CondFlag[contactgrid]
                    && nogridcontact(sq_rebirth))) /* V2.5c  NG */
          {
            crenkam[nbply]= sq_rebirth;
            e[sq_rebirth]= pi_arriving;
            spec[sq_rebirth]= spec_pi_moving;
            if (rex_circe) {
              if (sq_departure == prev_rb)
                rb= sq_rebirth;
              if (sq_departure == prev_rn)
                rn= sq_rebirth;

              if (castling_supported
                  && (abs(pi_arriving) == King)) {    /* V3.77  TLi */
                if (TSTFLAG(spec_pi_moving, White)
                    && sq_rebirth == square_e1
                    && (!CondFlag[dynasty] || nbpiece[roib]==1)) { /* V4.02 TM */
                  /* white king reborn on e1 */
                  SETFLAGMASK(castling_flag[nbply],ke1_cancastle);
                }
                else if (TSTFLAG(spec_pi_moving, Black)
                         && sq_rebirth == square_e8
                         && (!CondFlag[dynasty] || nbpiece[roin]==1)) { /* V4.02 TM */
                  /* black king reborn on e8 */
                  SETFLAGMASK(castling_flag[nbply],ke8_cancastle);
                }
              }
            }
            if (castling_supported
                && (abs(pi_arriving) == Rook)) { /* V3.77  TLi */
              if (TSTFLAG(spec_pi_moving, White)) {
                if (sq_rebirth == square_h1) {
                  /* white rook reborn on h1 */
                  SETFLAGMASK(castling_flag[nbply],rh1_cancastle);
                }
                else if (sq_rebirth == square_a1) {
                  /* white rook reborn on a1 */
                  SETFLAGMASK(castling_flag[nbply],ra1_cancastle);
                }
              }
              if (TSTFLAG(spec_pi_moving, Black)) {
                if (sq_rebirth == square_h8) {
                  /* black rook reborn on h8 */
                  SETFLAGMASK(castling_flag[nbply],rh8_cancastle);
                }
                else if (sq_rebirth == square_a8) {
                  /* black rook reborn on a8 */
                  SETFLAGMASK(castling_flag[nbply],ra8_cancastle);
                }
              }
            }
          } else
            nbpiece[pi_arriving]--;
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
        if (pi_captured != vide) {    /* V2.4c  NG */
          /* V3.1  TLi */
          if (anyclone
              && sq_departure != prev_rn && sq_departure != prev_rb) {
            /* that's rubish  TLi  V3.22
            ** pi_reborn= -pi_departing;
            ** spec_pi_captured= spec_pi_moving;
            */
            /* Circe Clone - new implementation V3.22  TLi
            ** captured pieces are reborn as pieces
            ** of the same type as the capturing piece
            ** if the latter one is not royal.
            */
            /* change type of pieces according to colour */
            pi_reborn = ((pi_departing * pi_captured < 0)
                         ? -pi_departing
                         : pi_departing);
            /* If it is a pawn give it the pawn-attribut.
            ** Otherwise delete it - the captured piece may
            ** have been a pawn, but is not anymore.
            */
          }
          else if (anytraitor) {
            pdispspec[nbply]=spec_pi_captured;
            pi_reborn= -pi_captured;
            CHANGECOLOR(spec_pi_captured);
          }
          else {
            if (CondFlag[chamcirce]) { /* V3.50  TLi */
              pi_reborn= ChamCircePiece(pi_captured);
            }
            else if (CondFlag[antieinstein]) {
              pi_reborn= inc_einstein(pi_captured);
            }
            else {
              pi_reborn= pi_captured;
            }
          }

          if (CondFlag[couscous])     /* V3.1  TLi */
            sq_rebirth= (*circerenai)(pi_arriving,
                                      spec_pi_moving,
                                      sq_capture,
                                      sq_departure,
                                      sq_arrival,
                                      advers(traitnbply));
          else
            sq_rebirth= (*circerenai)(pi_reborn,
                                      spec_pi_captured,
                                      sq_capture,
                                      sq_departure,
                                      sq_arrival,
                                      traitnbply);

          if (!rex_circe
              && (flag_testlegality || CondFlag[brunner])
              && /* V3.50 SE */
              (sq_capture == prev_rb || sq_capture == prev_rn))
          {
            /* ordinary circe and (isardam, brunner or
            ** ohneschach) it can happen that the king is
            ** captured while testing the legality of a
            ** move. Therefore prevent the king from being
            ** reborn.
            */
            sq_rebirth= initsquare;
          }

          if ( (e[sq_rebirth] == vide || flagAssassin)/* V2.4c NG */
               && !( CondFlag[contactgrid]
                     && nogridcontact(sq_rebirth)))
          { /* V2.5c NG */
            sqrenais[nbply]= sq_rebirth;
            if (rex_circe) {    /* V3.33  TLi */
              /* neutral K  V3.02  TLi */
              if (prev_rb == sq_capture) {
                rb= sq_rebirth;
              }
              if (prev_rn == sq_capture) {
                rn= sq_rebirth;
              }

              if (castling_supported
                  && (abs(pi_reborn) == King)) {/* V3.77  TLi */
                if (TSTFLAG(spec_pi_captured, White)
                    && sq_rebirth == square_e1
                    && (!CondFlag[dynasty] || nbpiece[roib]==1)) { /* V4.02 TM */
                  /* white king reborn on e1 */
                  SETFLAGMASK(castling_flag[nbply],ke1_cancastle);
                }
                else if (TSTFLAG(spec_pi_captured, Black)
                         && sq_rebirth == square_e8
                         && (!CondFlag[dynasty] || nbpiece[roin]==1)) { /* V4.02 TM */
                  /* black king reborn on e8 */
                  SETFLAGMASK(castling_flag[nbply],ke8_cancastle);
                }
              }
            }

            if (castling_supported
                && (abs(pi_reborn) == Rook)) { /* V3.77  TLi */
              if (TSTFLAG(spec_pi_captured, White)) {
                if (sq_rebirth == square_h1) {
                  /* white rook reborn on h1 */
                  SETFLAGMASK(castling_flag[nbply],rh1_cancastle);
                }
                else if (sq_rebirth == square_a1) {
                  /* white rook reborn on a1 */
                  SETFLAGMASK(castling_flag[nbply],ra1_cancastle);
                }
              }
              if (TSTFLAG(spec_pi_captured, Black)) {
                if (sq_rebirth == square_h8) {
                  /* black rook reborn on h8 */
                  SETFLAGMASK(castling_flag[nbply],rh8_cancastle);
                }
                else if (sq_rebirth == square_a8) {
                  /* black rook reborn on a8 */
                  SETFLAGMASK(castling_flag[nbply],ra8_cancastle);
                }
              }
            }
            if (anycirprom
                && is_pawn(pi_captured)
                && PromSq(advers(traitnbply), sq_rebirth))
            {    /* V3.39  TLi */
              /* captured white pawn on eighth rank: promotion ! */
              /* captured black pawn on first rank: promotion ! */
              piece pprom= cir_prom[nbply];
              if (pprom == vide) {
                cir_prom[nbply]= pprom= getprompiece[vide];
              }
              pi_reborn = pi_reborn < vide ? -pprom : pprom;   /* V3.33  TLi */
              if (cir_cham_prom[nbply]) { /* V3.1  TLi */
                SETFLAG(spec_pi_captured, Chameleon);
              }
            }
            if (TSTFLAG(spec_pi_captured, Volage)
                && SquareCol(sq_rebirth) != SquareCol(sq_capture))  /* V3.22  TLi */
            {
              pi_reborn= -pi_reborn;   /* V3.33  TLi */
              CHANGECOLOR(spec_pi_captured);  /* V3.33  TLi */
              if (!CondFlag[hypervolage]) {
                CLRFLAG(spec_pi_captured, Volage);
              }
            }
            if (flagAssassin) {    /* V3.50 SE */
              nbpiece[pdisp[nbply]=e[sq_rebirth]]--;
              pdispspec[nbply]=spec[sq_rebirth];
            }
            nbpiece[e[sq_rebirth]= pi_reborn]++;    /* V3.33  TLi */
            spec[sq_rebirth]= spec_pi_captured;      /* V3.33  TLi */
          }
        }
      }
    }

    if (bl_royal_sq != initsquare) /* V3.50  TLi */
      rn= bl_royal_sq;

    if (wh_royal_sq != initsquare) /* V3.50  TLi */
      rb= wh_royal_sq;

    if (CondFlag[republican]) {     /* V3.50 SE */
      if (traitnbply==blanc) {
        if (flag_writinglinesolution
            && repub_k[nbply] != initsquare) { /* v3.53 SE */
          rn= repub_k[nbply];
          e[rn]= roin;
          nbpiece[roin]++;
        }
        else if (!rn && !flag_dontaddk) {
          flag_dontaddk= true;
          find_mate_square(blanc);
          flag_dontaddk= false;
          repub_k[nbply]= super[nbply]<= haut ? super[nbply] : initsquare;
        }
        else
        {
          repub_k[nbply]= initsquare;
          super[nbply]= haut + 1;
        }
      }
      else {
        if (flag_writinglinesolution
            && repub_k[nbply] != initsquare) { /* v3.53 SE */
          rb= repub_k[nbply];
          e[rb]= roib;
          nbpiece[roib]++;
        }
        else if (!rb && !flag_dontaddk) {
          flag_dontaddk= true;
          find_mate_square(noir);
          flag_dontaddk= false;
          repub_k[nbply]= super[nbply]<= haut ? super[nbply] : initsquare;
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

    if (CondFlag[arc]) {    /* V3.62 SE */
      if (sq_departure==square_d4 || sq_departure==square_e4
          || sq_departure==square_d5 || sq_departure==square_e5
          || sq_arrival==square_d4 || sq_arrival==square_e4
          || sq_arrival==square_d5 || sq_arrival==square_e5)
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

    if (traitnbply==blanc
        ? CondFlag[white_oscillatingKs]
        : CondFlag[black_oscillatingKs]) {
      /* V3.78 SE */
      boolean priorcheck= false;
      square temp= rb;
      piece temp1= e[rb];
      Flags temp2= spec[rb];

      oscillatedKs[nbply]= false;
      if (OscillatingKingsTypeB[traitnbply])
        priorcheck= echecc(traitnbply);
      if ((oscillatedKs[nbply]= (!OscillatingKingsTypeC[traitnbply]
                                 || echecc(advers(traitnbply))))) /* V3.80  SE */
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
        if (OscillatingKingsTypeB[traitnbply] && priorcheck)
          return false;
      }
    }

    if (TSTFLAG(spec_pi_moving, ColourChange)) {   /* V3.64 SE */
      if (abs(pi_hurdle= e[sq_hurdle]) > roib) {
        nbpiece[pi_hurdle]--;
        e[sq_hurdle]= -pi_hurdle;
        nbpiece[-pi_hurdle]++;
        CHANGECOLOR(spec[sq_hurdle]);
      }
    }

    if (CondFlag[dynasty]) { /* V4.02 TM */
      /* adjust rn, rb and/or castling flags */
      square *bnp;
      square s;

      if (nbpiece[roib]==1) {
        if (rb==initsquare)
          for (bnp= boardnum; *bnp; bnp++) {
            s = *bnp;
            if (e[s] == roib) {
              if (s==square_e1)
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
              if (s==square_e8)
                SETFLAGMASK(castling_flag[nbply],ke8_cancastle);
              rn = *bnp;
              break;
            }
          }
      }
      else
        rn = initsquare;
    }

    if (CondFlag[strictSAT] && SATCheck)  /* V4.03 SE */
    {
      WhiteStrictSAT[nbply]= echecc_normal(blanc);
      BlackStrictSAT[nbply]= echecc_normal(noir);
    }

    if (CondFlag[masand]
        && echecc(advers(traitnbply))
        && observed(traitnbply == blanc ? rn : rb,
                    move_gen_top->arrival)) /* V4.06 SE */
      change_observed(move_gen_top->arrival);
        
    if (!BGL_whiteinfinity
        && (BGL_global || traitnbply == blanc)) /* V4.06 SE */
    {
      BGL_white -= BGL_move_diff_code[abs(move_gen_top->departure
                                          -move_gen_top->arrival)];
    }
    if (!BGL_blackinfinity && (BGL_global || traitnbply == noir))
    {
      BGL_black -= BGL_move_diff_code[abs(move_gen_top->departure
                                          -move_gen_top->arrival)];
    }
  } /* if (jouegenre) */


  return jouecoup_legality_test(prev_nbpiece,sq_rebirth);
} /* end of jouecoup */

void IncrementMoveNbr(void) {   /* V3.44  TLi */
  if (MoveNbr++ < RestartNbr)
    return;

  sprintf(GlobalStr,"%3d  (", MoveNbr);
  StdString(GlobalStr);
  ecritcoup();       /* V3.02  TLi */
  if (!flag_regression) {  /* V3.74  NG */
    StdString("   ");
    PrintTime();      /* V2.90  NG */
  }
#ifdef HASHRATE
  StdString("   ");
  HashStats(0, NULL);
#endif
  StdString(")\n");
} /* IncrementMoveNbr */

void repcoup(void) {
  square sq_rebirth;
  piece pi_departing, pi_captured, pi_arriving, pi_hurdle;
  Flags spec_pi_moving;        /*, pjspec_  V3.62  TLi */
  boolean next_prom = true;     /* V3.50 SE */ /* 3.71 TM*/
  square nextsuper= initsquare;     /* V3.52 SE */
  square sq_hurdle;     /* V3.65  NG */

  move_generation_elmt* move_gen_top = move_generation_stack+nbcou;

  square sq_departure= move_gen_top->departure;
  square sq_arrival= move_gen_top->arrival;
  square sq_capture= move_gen_top->capture;

  if (jouegenre) {           /* V3.53  TLi */
    if (!BGL_whiteinfinity && (BGL_global || trait[nbply] == blanc)) /* V4.06 SE */
    {
      BGL_white += BGL_move_diff_code[abs(sq_departure-sq_arrival)];
    }
    if (!BGL_blackinfinity && (BGL_global || trait[nbply] == noir))
    {
      BGL_black += BGL_move_diff_code[abs(sq_departure-sq_arrival)];
    }
        
    if (CondFlag[masand]
        && echecc(advers(trait[nbply]))
        && observed(trait[nbply] == blanc ? rn : rb,
                    sq_arrival)) /* V4.06 SE */
      change_observed(sq_arrival);

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

    if (CondFlag[arc]) {        /* V3.62 SE */
      /* RotateMirror(rot90); */ /* V3.62  SE */
      if (sq_departure==square_d4 || sq_departure==square_e4
          || sq_departure==square_d5 || sq_departure==square_e5
          || sq_arrival==square_d4 || sq_arrival==square_e4
          || sq_arrival==square_d5 || sq_arrival==square_e5)
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

    if (CondFlag[singlebox] && SingleBoxType==singlebox_type2) { /* 3.71 TM */
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
  } /* jouegenre */

  pi_captured= pprise[nbply];
  pi_departing= pjoue[nbply];
  spec_pi_moving= jouespec[nbply];

  if (sq_arrival==nullsquare)    /* V3.70 SE */
  {
    nbcou--;
    return;
  }

  if (jouegenre) {    /* V3.64 SE */
    if (CondFlag[singlebox] && SingleBoxType==singlebox_type3
        && sb3[nbcou].what!=vide) { /* V3.71 TM */
      piece pawn = trait[nbply]==blanc ? pb : pn;
      e[sb3[nbcou].where] = pawn;
      if (sq_departure!=sb3[nbcou].where) {
        --nbpiece[sb3[nbcou].what];
        ++nbpiece[pawn];
      }
    }

    if (TSTFLAG(spec_pi_moving, ColourChange)) {
      sq_hurdle= chop[nbcou];  /* V3.65  NG */
      if (abs(pi_hurdle= e[sq_hurdle]) > roib) { /* V3.65  NG */
        nbpiece[pi_hurdle]--;
        e[sq_hurdle]= -pi_hurdle;
        nbpiece[-pi_hurdle]++;
        CHANGECOLOR(spec[sq_hurdle]);
      }
    }
  }

  castling_flag[nbply]= castling_flag[nbply-1];    /* V3.55  TLi */

  switch (sq_capture) { /* V3.55  TLi */
  case messigny_exchange:
    e[sq_arrival]= e[sq_departure];
    spec[sq_arrival]= spec[sq_departure];
    e[sq_departure]= pi_departing;
    spec[sq_departure]= spec_pi_moving;
    nbcou--;
    rb= RB_[nbply];
    rn= RN_[nbply];
    return;

  case kingside_castling:
    if (CondFlag[einstein]) {   /* V3.44  NG */
      if (sq_departure == square_e1) {   /* white */
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
      e[sq_departure+3*dir_right]= e[sq_departure+dir_right];
    }
    spec[sq_departure+3*dir_right]= spec[sq_departure+dir_right];
    e[sq_departure+dir_right]= vide;
    CLEARFL(spec[sq_departure+dir_right]);
    /* reset everything */ /* V3.35  NG */
    break;

  case queenside_castling:
    if (CondFlag[einstein]) {   /* V3.44  NG */
      if (sq_departure == square_e1) {    /* white */
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
      e[sq_departure+4*dir_left]= e[sq_departure+dir_left];
    }
    spec[sq_departure+4*dir_left]= spec[sq_departure+dir_left];
    e[sq_departure+dir_left]= vide;
    CLEARFL(spec[sq_departure+dir_left]);
    /* reset everything */ /* V3.35  NG */
    break;

  } /* switch (sq_capture) */

  /* the supercirce rebirth square has to be evaluated here in the
  ** position after the capture. Otherwise it is more difficult to
  ** allow the rebirth on the original square of the capturing piece
  ** or in connection with locust or e.p. captures.
  ** V3.22  TLi
  */
  if ((CondFlag[supercirce] && pi_captured != vide)
      || isapril[abs(pi_captured)]
      || (CondFlag[antisuper] && pi_captured != vide))
  {
    /* V3.1, 3.52  TLi, SE */
    nextsuper= super[nbply];
    while ((e[++nextsuper] != vide) && (nextsuper < haut))
      ;
    if (CondFlag[antisuper]
        && AntiCirCheylan
        && nextsuper==sq_capture)  /* V3.81  SE */
      while ((e[++nextsuper] != vide) && (nextsuper < haut))
        ;
  }

  if (CondFlag[republican]) {    /* V3.50 SE */
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
  
  /* first delete all changes */
  if (repgenre) {     /* V2.4c  NG */
    if (senti[nbply]) {    /* V2.90 TLi */
      --nbpiece[e[sq_departure]];
      senti[nbply]= false;
    }
    if (CondFlag[imitators])    /* V2.4d  TM */
    {
      if (sq_capture == queenside_castling)
        joueim(+dir_left);
      else if (sq_capture != kingside_castling) /* joueim(0) (do nothing) if OO */
        joueim(sq_departure - sq_arrival);      /* verschoben TLi */
    }

    if (TSTFLAG(PieSpExFlags, Neutral)) {
      /* the following is faster !  TLi
       * initneutre((pi_departing > vide) ? blanc : noir);
       */

      if (TSTFLAG(spec_pi_moving, Neutral) &&
          (pi_departing < vide ? noir : blanc) != neutcoul)
        pi_departing= -pi_departing;
      if (TSTFLAG(pprispec[nbply], Neutral) &&
          (pi_captured < vide ? noir : blanc) != neutcoul)
        pi_captured= -pi_captured;
    }
    if ((sq_rebirth= sqrenais[nbply]) != initsquare) {
      sqrenais[nbply]= initsquare;     /* V3.1  TLi */
      if (sq_rebirth != sq_arrival) {     /* V2.60  NG */
        nbpiece[e[sq_rebirth]]--;
        e[sq_rebirth]= vide;
        spec[sq_rebirth]= 0;
      }
      if (flagAssassin && pdisp[nbply]) {   /* V3.50 SE */
        nbpiece[e[sq_rebirth]= pdisp[nbply]]++;
        spec[sq_rebirth]= pdispspec[nbply];
      }
      if (anytraitor)
        spec[sq_rebirth]= pdispspec[nbply];
    }
    /* V3.1  TLi */
    if ((sq_rebirth= crenkam[nbply]) != initsquare) {
      /* Kamikaze and AntiCirce */
      crenkam[nbply]= initsquare;     /* V2.60  NG */
      if (sq_rebirth != sq_arrival) {
        nbpiece[e[sq_rebirth]]--;
        e[sq_rebirth]= vide;
        spec[sq_rebirth]= 0;
      }
    }
  } /* if (repgenre) */

  nbpiece[e[sq_arrival]]--;

  /* now reset old position */
  if (sq_arrival != sq_capture) {
    e[sq_arrival]= vide;
    spec[sq_arrival] = 0;
  }
  e[sq_departure]= pi_departing;
  spec[sq_departure] = spec_pi_moving;
  nbpiece[pi_departing]++;

  if (PatienceB) {     /* V3.50 SE */
    ply nply;
    for (nply= nbply - 1 ; nply > 1 ; nply--) {
      if (trait[nply] == trait[nbply]) {
        e[sqdep[nply]]= obs;
      }
    }
  }
  
  if (!anyantimars || sq_capture != sq_departure)
  {
    e[sq_capture]= pi_captured;
    spec[sq_capture]= pprispec[nbply];
  }

  if (pi_captured != vide)
    nbpiece[pi_captured]++;

  rb= RB_[nbply];  /* H.D. 15.02.93 */
  rn= RN_[nbply];

  if (abs(pi_departing) == andergb) {     /* V3.65  TLi */
    square sq= sq_arrival - direction(sq_departure, sq_arrival);

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
    if ((pi_arriving= norm_prom[nbply]) != vide) {
      if (CondFlag[singlebox]
          && (SingleBoxType==singlebox_type1
              || SingleBoxType==singlebox_type2))   /* V3.71 TM */
      {
        pi_arriving = next_singlebox_prom(pi_arriving,trait[nbply]);
      }
      else {
        pi_arriving= getprompiece[pi_arriving];

        if (pi_captured != vide && anyanticirce) {  /* V3.1  TLi */
          /* CLRFLAG(spec_pi_moving, APawn);  V3.22  TLi */
          while (pi_arriving != vide) {
            sq_rebirth= (*antirenai)(pi_arriving,
                                     spec_pi_moving,
                                     sq_capture,
                                     sq_departure,
                                     sq_arrival,
                                     advers(trait[nbply]));
            if (sq_rebirth == sq_departure)
              break;
            if (LegalAntiCirceMove(sq_rebirth, sq_capture, sq_departure)) {
              /* V3.62  TLi */
              break;
            }
            pi_arriving= getprompiece[pi_arriving];
          }
        }
      }
      norm_prom[nbply]= pi_arriving;

      if ((pi_arriving == vide)
          && TSTFLAG(PieSpExFlags, Chameleon)  /* V3.1  TLi */
          && !norm_cham_prom[nbply])
      {
        pi_arriving= getprompiece[vide];
        if (pi_captured != vide && anyanticirce)   /* V3.1  TLi */
          while (pi_arriving != vide
                 && ((sq_rebirth= (*antirenai)(pi_arriving,
                                               spec_pi_moving,
                                               sq_capture,
                                               sq_departure,
                                               sq_arrival,
                                               advers(trait[nbply])))
                     != sq_departure)
                 && e[sq_rebirth] != vide)
            pi_arriving= getprompiece[pi_arriving];

        norm_prom[nbply]= pi_arriving;
        norm_cham_prom[nbply]= true;
      }
      if ((pi_arriving == vide) && !CondFlag[noiprom])   /* V2.4d  TM */
        Iprom[nbply]= true;
    }
    else if (!CondFlag[noiprom] && Iprom[nbply]) {  /* V2.4d  TM */
      ply icount;
      for (icount= nbply; icount <= maxply; inum[icount++]--);
      Iprom[nbply]= false;
    }

    if (pi_arriving == vide) {
      norm_cham_prom[nbply]= false;     /* V3.1  TLi */
      if (anycirprom
          && ((pi_arriving= cir_prom[nbply]) != vide)) { /* V3.1  TLi */
        pi_arriving= cir_prom[nbply]= getprompiece[pi_arriving];
        if (pi_arriving == vide
            && TSTFLAG(PieSpExFlags, Chameleon)  /* V3.1 TLi */
            && !cir_cham_prom[nbply])
        {
          cir_prom[nbply]= pi_arriving= getprompiece[vide];
          cir_cham_prom[nbply]= true;
        }
      }
      if (pi_arriving == vide
          && !(!CondFlag[noiprom] && Iprom[nbply]))  /* V2.4d  TM */
      {
        if ((CondFlag[supercirce] && pi_captured != vide)
            || isapril[abs(pi_captured)]
            || (CondFlag[antisuper] && pi_captured != vide))    /* V3.52 SE */
        {
          super[nbply]= nextsuper;
          if ((super[nbply] > haut)
              || (CondFlag[antisuper]
                  && !LegalAntiCirceMove(nextsuper,sq_capture,sq_departure))) {
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

/* Generate (piece by piece) candidate moves to check if stalemate has
 * been reached in a mate or stalemate problem. Do *not* generate
 * moves by the king of the couleur to be (stale)mated; it has already
 * been taken care of. */
boolean pattencore(couleur camp, square** pattfld) {   /* V3.50 SE */
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

/* Have we reached the stalemate position looked for in a mate or
 * stalemate problem. */
boolean patt(couleur camp)
{
  square *pattfield= boardnum;  /* local so allows nested calls to patt */

  boolean const whbl_exact= camp==blanc ? wh_exact : bl_exact;
  if (!whbl_exact       /* 3.20  NG */
      && !flag_testlegality) {       /* V3.45  TLi */
    nextply();
    current_killer_state= null_killer_state;
    trait[nbply]= camp;
    /* flagminmax= false;       V2.90, V3.44  TLi */
    /* flag_minmax[nbply]= false;      V3.44  TLi */
    if (TSTFLAG(PieSpExFlags,Neutral))
      initneutre(advers(camp));
    if (camp == blanc) {
      if (rb != initsquare)
        gen_wh_piece(rb, abs(e[rb]));     /* V3.02  TLi */
    } else {
      if (rn != initsquare)
        gen_bl_piece(rn, -abs(e[rn]));   /* V3.02  TLi */
    }

    if (CondFlag[MAFF] || CondFlag[OWU]) {     /* V3.78 SE */
      int k_fl= 0, w_unit= 0;
      while (encore()) {
        if (jouecoup()
            || (CondFlag[blackultraschachzwang] && (camp == noir))
            || (CondFlag[whiteultraschachzwang] && (camp == blanc))          ) {
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
        ) {       /* V3.44 SE/TLi */
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
    couleur ad= advers(camp);  /* V3.53  TLi */

    /* exact-maxis, ohneschach */
    move_generation_mode= move_generation_optimized_by_killer_move;      /* V3.44 TLi */
    if (!CondFlag[ohneschach]) {  /* V3.53  TLi */
      genmove(camp);
      while (encore()) {
        if (jouecoup()) { /* V3.44  SE/TLi */
          if (! echecc(camp)) {
            repcoup();
            finply();
            return false;
          }
        }
        repcoup();
      }
      finply();

    } else {    /* V3.53  TLi */
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
      move_generation_mode= move_generation_optimized_by_killer_move;
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
  return move_generation_stack[nbcou].arrival == TargetField
    && crenkam[nbply] == initsquare    /* V3.22  TLi */
    && !echecc(camp);
}

boolean stip_circuit(couleur camp) {
  square cazz, renkam;

  cazz = move_generation_stack[nbcou].arrival;
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

  cazz = move_generation_stack[nbcou].arrival;
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

boolean para_patt(couleur camp) {   /* V3.50 SE */
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
  /* modifiziert fuer paralysierende Steine  V3.02 TLi */
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

boolean stip_mate_or_stale(couleur camp) {       /* V3.60 SE */
  /* modifiziert fuer paralysierende Steine  V3.02 TLi */
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
     Modifizierungen erforderlich     V3.02  TLi */

  couleur ad= advers(camp);
  if (TSTFLAG(PieSpExFlags, Paralyse)) {   /* V3.50 SE */
    return (para_patt(ad) && para_patt(camp));
  }
  else {
    return !echecc(ad) && !echecc(camp)
      && patt(ad) && patt(camp);
  }
}

boolean stip_autostale(couleur camp) {     /* V3.50 SE */

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
  return move_generation_stack[nbcou].arrival != move_generation_stack[nbcou].capture
    && is_pawn(pjoue[nbply])         /* V3.31  TLi */
    && !echecc(camp);
}

/* V3.32  TLi */
boolean stip_doublemate(couleur camp) {
  boolean flag;
  couleur ad= advers(camp);

  /* if (!echecc(ad) && !echecc(camp))  bug spotted by NG */

  if (!echecc(ad) || !echecc(camp))    /* V3.35  TLi */
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
  testdblmate= flag_nk;  /* Neutral K bug fix V3.50 SE */
  /*true;       modified to allow isardam + ##  */
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

void find_mate_square(couleur camp)     /* V3.50 SE */
{
  square sq;

  if (camp == blanc) {
    while ((sq= ++super[nbply]) <= haut) {
      if (e[sq] == vide) {
        rn= sq;
        e[rn]= roin;
        nbpiece[roin]++;
        if ((*stipulation)(camp)) {   /* V3.53  SE */
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
        if ((*stipulation)(camp)) {   /* V3.53  SE */
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
