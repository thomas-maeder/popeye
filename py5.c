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
 ** 2007/11/08 SE   New conditions: Vaulting kings (invented: J.G.Ingram)
 **                 Transmuting/Reflecting Ks now take optional piece list
 **                 turning them into vaulting types
 **
 ** 2007/12/26 SE   New piece: Reverse Pawn (for below but independent)
 **                 New condition: Protean Chess
 **                 New piece type: Protean man (invent A.H.Kniest?)
 **                 (Differs from Frankfurt chess in that royal riders
 **                 are not 'non-passant'. Too hard to do but possibly
 **                 implement as an independent condition later).
 **
 ** 2008/01/01 SE   Bug fix: Circe Assassin + proof game (reported P.Raican)
 **
 ** 2008/02/10 SE   New condition: Cheameleon Pursuit (invented? : L.Grolman)  
 **
 ** 2008/02/25 SE   New piece type: Magic 
 **                 Adjusted Masand code
 ** 2008/03/13 SE   New condition: Castling Chess (invented: N.A.Bakke?)  
 **
 **
 **************************** End of List ******************************/

#if defined(macintosh) /* is always defined on macintosh's  SB */
# define SEGM2
# include "pymac.h"
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>  /* H.D. 10.02.93 prototype fuer exit */

#if defined(DOS)
# if defined(__GNUC__)
#  include <pc.h>
# else
#  include <bios.h>
# endif /* __GNUC__ */
#endif /* DOS */

#include "py.h"
#include "pyproc.h"
#include "pyhash.h"
#include "pydata.h"
#include "pymsg.h"
#include "pystip.h"
#include "pyleaf.h"
#include "trace.h"

piece linechampiece(piece p, square sq) {
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
    if (CondFlag[cavaliermajeur]) {
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
  if (CondFlag[leofamily]) {
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
    if (CondFlag[cavaliermajeur]) {
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
  if (CondFlag[leofamily]) {
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
    if (CondFlag[cavaliermajeur]) {
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
}

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
}

#if defined(DOS)
# pragma warn -par
#endif

square renplus(piece p_captured, Flags p_captured_spec,
               square sq_capture, square sq_departure, square sq_arrival,
               Side capturer) {
  /* Echecs plus */
  if (sq_capture==square_d4
      || sq_capture==square_e4
      || sq_capture==square_d5
      || sq_capture==square_e5)
  {
    square k;
    switch (more_ren)
    {
    case 0 : k= square_d4; break;
    case 1 : k= square_e4; break;
    case 2 : k= square_d5; break;
    case 3 : k= square_e5; break;
    default: k= -1; /* avoid compiler warning */
    }
    more_ren++;
    if (more_ren == 4)
      more_ren= 0;

    return k;
  }
  else
    return sq_capture;
}

square renrank(piece p_captured, Flags p_captured_spec,
               square sq_capture, square sq_departure, square sq_arrival,
               Side capturer) {
  square sq= ((sq_capture/onerow)%2==1
              ? rennormal(p_captured,p_captured_spec,
                          sq_capture,sq_departure,sq_arrival,capturer)
              : renspiegel(p_captured,p_captured_spec,
                           sq_capture,sq_departure,sq_arrival,capturer));
  return onerow*(sq_capture/onerow) + sq%onerow;
}

square renfile(piece p_captured, Flags p_captured_spec,
               square sq_capture, square sq_departure, square sq_arrival,
               Side capturer)
{
  int col= sq_capture % onerow;

  if (capturer==Black)
  {
    if (is_pawn(p_captured))
      return col + (nr_of_slack_rows_below_board+1)*onerow;
    else
      return col + nr_of_slack_rows_below_board*onerow;
  }
  else
  {
    if (is_pawn(p_captured))
      return col + (nr_of_slack_rows_below_board+nr_rows_on_board-2)*onerow;
    else
      return col + (nr_of_slack_rows_below_board+nr_rows_on_board-1)*onerow;
  }
}

square renspiegelfile(piece p_captured, Flags p_captured_spec,
                      square sq_capture,
                      square sq_departure, square sq_arrival,
                      Side capturer)
{
  return renfile(p_captured,p_captured_spec,
                 sq_capture,sq_departure,sq_arrival,advers(capturer));
} /* renspiegelfile */

square renpwc(piece p_captured, Flags p_captured_spec,
              square sq_capture, square sq_departure, square sq_arrival,
              Side capturer)
{
  return sq_departure;
} /* renpwc */

square renequipollents(piece p_captured, Flags p_captured_spec,
                       square sq_capture,
                       square sq_departure, square sq_arrival,
                       Side capturer)
{
  /* we have to solve the enpassant capture / locust capture problem in the future. */
#if defined(WINCHLOE)
  return sq_capture + sq_arrival - sq_departure;
#endif
  return sq_capture + sq_capture - sq_departure;
} /* renequipollents */

square renequipollents_anti(piece p_captured, Flags p_captured_spec,
                            square sq_capture,
                            square sq_departure, square sq_arrival,
                            Side capturer)
{
  /* we have to solve the enpassant capture / locust capture problem in the future. */
#if defined(WINCHLOE)
  return sq_arrival + sq_arrival - sq_departure;
#endif
  return sq_capture + sq_capture - sq_departure;
} /* renequipollents_anti */

square rensymmetrie(piece p_captured, Flags p_captured_spec,
                    square sq_capture,
                    square sq_departure, square sq_arrival,
                    Side capturer)
{
  return (haut+bas) - sq_capture;
} /* rensymmetrie */

square renantipoden(piece p_captured, Flags p_captured_spec,
                    square sq_capture,
                    square sq_departure, square sq_arrival,
                    Side capturer)
{
  int const row= sq_capture/onerow - nr_of_slack_rows_below_board;
  int const file= sq_capture%onerow - nr_of_slack_files_left_of_board;
  
  sq_departure= sq_capture;

  if (row<nr_rows_on_board/2)
    sq_departure+= nr_rows_on_board/2*dir_up;
  else
    sq_departure+= nr_rows_on_board/2*dir_down;

  if (file<nr_files_on_board/2)
    sq_departure+= nr_files_on_board/2*dir_right;
  else
    sq_departure+= nr_files_on_board/2*dir_left;

  return sq_departure;
} /* renantipoden */

square rendiagramm(piece p_captured, Flags p_captured_spec,
                   square sq_capture, square sq_departure, square sq_arrival,
                   Side capturer)
{
  return DiaRen(p_captured_spec);
}

square rennormal(piece p_captured, Flags p_captured_spec,
                 square sq_capture, square sq_departure, square sq_arrival,
                 Side capturer)
{
  square  Result;
  int col, ran;
  Side  cou;

  col = sq_capture % onerow;
  ran = sq_capture / onerow;

  p_captured= abs(p_captured);

  if (CondFlag[circemalefiquevertical]) {
    col= onerow-1 - col;
    if (p_captured == db)
      p_captured= roib;
    else if (p_captured == roib)
      p_captured= db;
  }

  if ((ran&1) != (col&1))
    cou = White;
  else
    cou = Black;

  if (CondFlag[cavaliermajeur])
    if (p_captured == nb)
      p_captured = cb;

  /* Below is the reason for the define problems. What a "hack" ! */
  if (CondFlag[leofamily] &&
      (p_captured > Bishop) && (Vao >= p_captured))
    p_captured-= 4;

  if (capturer == Black)
  {
    if (is_pawn(p_captured))
      Result= col + (nr_of_slack_rows_below_board+1)*onerow;
    else {
      if (!flagdiastip && TSTFLAG(p_captured_spec, FrischAuf)) {
        Result= (col
                 + (onerow
                    *(CondFlag[glasgow]
                      ? nr_of_slack_rows_below_board+nr_rows_on_board-2
                      : nr_of_slack_rows_below_board+nr_rows_on_board-1)));
      }
      else
        switch(p_captured) {
        case roib:
          Result= square_e1;
          break;
        case cb:
          Result= cou == White ? square_b1 : square_g1;
          break;
        case tb:
          Result= cou == White ? square_h1 : square_a1;
          break;
        case db:
          Result= square_d1;
          break;
        case fb:
          Result= cou == White ? square_f1 : square_c1;
          break;
        default: /* fairy piece */
          Result= (col
                   + (onerow
                      *(CondFlag[glasgow]
                        ? nr_of_slack_rows_below_board+nr_rows_on_board-2
                        : nr_of_slack_rows_below_board+nr_rows_on_board-1)));
        }
    }
  }
  else
  {
    if (is_pawn(p_captured))
      Result= col + (nr_of_slack_rows_below_board+nr_rows_on_board-2)*onerow;
    else {
      if (!flagdiastip && TSTFLAG(p_captured_spec, FrischAuf)) {
        Result= (col
                 + (onerow
                    *(CondFlag[glasgow]
                      ? nr_of_slack_rows_below_board+1
                      : nr_of_slack_rows_below_board)));
      }
      else
        switch(p_captured) {
        case fb:
          Result= cou == White ? square_c8 : square_f8;
          break;
        case db:
          Result= square_d8;
          break;
        case tb:
          Result= cou == White ? square_a8 : square_h8;
          break;
        case cb:
          Result= cou == White ? square_g8 : square_b8;
          break;
        case roib:
          Result= square_e8;
          break;
        default: /* fairy piece */
          Result= (col
                   + (onerow
                      *(CondFlag[glasgow]
                        ? nr_of_slack_rows_below_board+1
                        : nr_of_slack_rows_below_board)));
        }
    }
  }

  return(Result);
} /* rennormal */

square rendiametral(piece p_captured, Flags p_captured_spec,
                    square sq_capture,
                    square sq_departure, square sq_arrival,
                    Side capturer) {
  return (haut+bas
          - rennormal(p_captured,p_captured_spec,
                      sq_capture,sq_departure,sq_arrival,capturer));
}

square renspiegel(piece p_captured, Flags p_captured_spec,
                  square sq_capture,
                  square sq_departure, square sq_arrival,
                  Side capturer)
{
  return rennormal(p_captured,p_captured_spec,
                   sq_capture,sq_departure,sq_arrival,advers(capturer));
}

square rensuper(piece p_captured, Flags p_captured_spec,
                square sq_capture,
                square sq_departure, square sq_arrival,
                Side capturer)
{
  return super[nbply];
}

#if defined(DOS)
# pragma warn +par
#endif

boolean is_short(piece p)
{
  switch (abs(p)) {
  case  Pawn:
  case  BerolinaPawn:
  case  ReversePawn: 
  case  Mao:
  case  Moa:
  case  Skylla:
  case  Charybdis:
  case  ChinesePawn:
    return  True;
  default:
    return  False;
  }
}

boolean is_pawn(piece p)
{
  switch (abs(p)) {
  case  Pawn:
  case  BerolinaPawn:
  case  SuperBerolinaPawn:
  case  SuperPawn:        
  case  ReversePawn: 
    return  True;
  default:
    return  False;
  }
}

boolean is_forwardpawn(piece p)
{
  switch (abs(p)) {
  case  Pawn:
  case  BerolinaPawn:
  case  SuperBerolinaPawn:
  case  SuperPawn:        
    return  True;
  default:
    return  False;
  }
}

boolean is_reversepawn(piece p)
{
  switch (abs(p)) {
  case  ReversePawn: 
    return  True;
  default:
    return  False;
  }
}

void genrn_cast(void) {
  /* It works only for castling_supported == TRUE
     have a look at funtion verifieposition() in py6.c
  */

  boolean is_castling_possible;

  if (dont_generate_castling)
    return;

  if (TSTFLAGMASK(castling_flag[nbply],bl_castlings)>ke8_cancastle
      && e[square_e8]==roin 
      /* then the king on e8 and at least one rook can castle !! */
      && !echecc(Black))
  {
    /* 0-0 */
    if (TSTFLAGMASK(castling_flag[nbply],blk_castling)==blk_castling
        && e[square_h8]==tn
        && e[square_f8]==vide
        && e[square_g8]==vide)
    {
      if (complex_castling_through_flag)
      {
        numecoup sic_nbcou= nbcou;

        /* temporarily deactivate maximummer etc. */
        boolean sic_flagblackmummer = flagblackmummer;
        flagblackmummer = false;
        empile(square_e8,square_f8,square_f8);
        flagblackmummer = sic_flagblackmummer;
        if (nbcou>sic_nbcou)
        {
          boolean ok= jouecoup() && !echecc(Black);
          repcoup();
          if (ok)
            empile(square_e8,square_g8,kingside_castling);
        }
      }
      else
      {
        e[square_e8]= vide;
        e[square_f8]= roin;
        if (rn!=initsquare)
          rn= square_f8;

        is_castling_possible= !echecc(Black);

        e[square_e8]= roin;
        e[square_f8]= vide;
        if (rn!=initsquare)
          rn= square_e8;

        if (is_castling_possible)
          empile(square_e8,square_g8,kingside_castling);
      }
    }

    /* 0-0-0 */
    if (TSTFLAGMASK(castling_flag[nbply],blq_castling)==blq_castling
        && e[square_a8]==tn
        && e[square_d8]==vide
        && e[square_c8]==vide
        && e[square_b8]==vide)
    {
      if (complex_castling_through_flag)
      {
        numecoup sic_nbcou= nbcou;

        /* temporarily deactivate maximummer etc. */
        boolean sic_flagblackmummer = flagblackmummer;
        flagblackmummer = false;
        empile(square_e8,square_d8,square_d8);
        flagblackmummer = sic_flagblackmummer;
        if (nbcou>sic_nbcou)
        {
          boolean ok= (jouecoup() && !echecc(Black));
          repcoup();
          if (ok)
            empile(square_e8,square_c8,queenside_castling);
        }
      }
      else
      {
        e[square_e8]= vide;
        e[square_d8]= roin;
        if (rn!=initsquare)
          rn= square_d8;
        
        is_castling_possible= !echecc(Black);
        
        e[square_e8]= roin;
        e[square_d8]= vide;
        if (rn!=initsquare)
          rn= square_e8;

        if (is_castling_possible)
          empile(square_e8,square_c8,queenside_castling);
      }
    }
  }
} /* genrn_cast */

void genrn(square sq_departure) {
  numvec k;
  boolean flag = false;  /* K im Schach ? */
  numecoup anf, l1, l2;

  VARIABLE_INIT(anf);

  if (calc_blrefl_king && !calctransmute) {
    /* K im Schach zieht auch */
    piece *ptrans;

    anf= nbcou;
    calctransmute= true;
    if (!blacknormaltranspieces && echecc(Black))
    {
      for (ptrans= blacktransmpieces; *ptrans; ptrans++) {
        flag = true;
        current_trans_gen=-*ptrans;
        gen_bl_piece(sq_departure, -*ptrans);
        current_trans_gen=vide;
      }
    }
    else if (blacknormaltranspieces)
    {
      for (ptrans= blacktransmpieces; *ptrans; ptrans++) {
        if (nbpiece[*ptrans]>0
            && (*checkfunctions[*ptrans])(sq_departure,*ptrans,eval_black))
        {
          flag = true;
          current_trans_gen=-*ptrans;
          gen_bl_piece(sq_departure, -*ptrans);
          current_trans_gen=vide;
        }
      }
    }
    calctransmute= false;

    if (flag && nbpiece[orphanb]>0) {
      piece king= e[rn];
      e[rn]= dummyn;
      if (!echecc(Black)) {
        /* black king checked only by an orphan
        ** empowered by the king */
        flag= false;
      }
      e[rn]= king;
    }

    /* K im Schach zieht nur */
    if (calc_bltrans_king && flag)
      return;
  }

  if (CondFlag[sting])
    gerhop(sq_departure,vec_queen_start,vec_queen_end,Black);

  for (k= vec_queen_end; k>=vec_queen_start; k--) {
    square sq_arrival= sq_departure+vec[k];
    if (e[sq_arrival]==vide || e[sq_arrival]>=roib)
      empile(sq_departure,sq_arrival,sq_arrival);
  }
  
  if (flag) {
    /* testempile nicht nutzbar */
    /* VERIFY: has anf always a proper value??
     */
    for (l1= anf+1; l1<=nbcou; l1++)
      if (move_generation_stack[l1].arrival != initsquare)
        for (l2= l1+1; l2<=nbcou; l2++)
          if (move_generation_stack[l1].arrival
              ==move_generation_stack[l2].arrival)
            move_generation_stack[l2].arrival= initsquare;
  }

  /* Now we test castling */
  if (castling_supported)
    genrn_cast();

  if (CondFlag[castlingchess] && !echecc(Black)) {
    for (k= vec_queen_end; k>= vec_queen_start; k--) {
      square sq_passed, sq_castler, sq_arrival;  
      piece p;
      sq_passed= sq_departure + vec[k]; 
      sq_arrival= sq_passed + vec[k];
         
      finligne(sq_departure,vec[k], p, sq_castler);
      if (sq_castler != sq_passed && sq_castler != sq_arrival && abs(p) >= roib)
      {
        if (complex_castling_through_flag)  /* V3.80  SE */
        {
          numecoup sic_nbcou= nbcou;
          empile (sq_departure, sq_passed, sq_passed);
          if (nbcou > sic_nbcou)
          {
            boolean ok= (jouecoup() && !echecc(Black));
            repcoup();
            if (ok)
              empile(sq_departure, sq_arrival, maxsquare+sq_castler);
          }
        }
        else
        {
          boolean checked;
          e[sq_departure]= vide;
          e[sq_passed]= roin;
          if (rn!=initsquare)
            rn= sq_passed;
          checked = echecc(Black);
          if (!checked) {
            empile(sq_departure, sq_arrival, maxsquare+sq_castler);
            if (0) {
              char buf[100];
              sprintf(buf, "%i %i %i \n", sq_departure, sq_arrival, sq_castler);
              StdString(buf);
            }
          }
          e[sq_departure]= roin;
          e[sq_passed]= vide;
          if (rn!=initsquare)
            rn= sq_departure;
        }
      }
    }
  }
}

void gen_bl_ply(void) {
  square i, j, z;
  piece p;

  /* Don't try to "optimize" by hand. The double-loop is tested as the  */
  /* fastest way to compute (due to compiler-optimizations !) */
  z= haut;
  for (i= nr_rows_on_board; i > 0; i--, z-= onerow-nr_files_on_board)
    for (j= nr_files_on_board; j > 0; j--, z--) {
      if ((p = e[z]) != vide) {
        if (TSTFLAG(spec[z], Neutral))
          p = -p;
        if (p < vide)
          gen_bl_piece(z, p);
      }
    }
  if (blacknull)
    empile(nullsquare, nullsquare, nullsquare);
} /* gen_bl_ply */

void gen_bl_piece_aux(square z, piece p) {
  if (CondFlag[annan]) {
    piece annan_p= e[z+onerow];
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
  case tn: genrid(z, vec_rook_start,vec_rook_end);
    break;
  case dn: genrid(z, vec_queen_start,vec_queen_end);
    break;
  case fn: genrid(z, vec_bishop_start,vec_bishop_end);
    break;
  default: gfeernoir(z, p);
    break;
  }
}

static void orig_gen_bl_piece(square sq_departure, piece p) {
  piece pi_departing;

  if (flag_madrasi) {
    if (!libre(sq_departure, true)) {
      return;
    }
  }

  if (TSTFLAG(PieSpExFlags,Paralyse)) {
    if (paralysiert(sq_departure)) {
      return;
    }
  }

  if (anymars||anyantimars) {
    square sq_rebirth;
    Flags spec_departing;

    if (CondFlag[phantom]) {
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
      sq_rebirth= (*marsrenai)(p,spec_departing,sq_departure,initsquare,initsquare,White);
      /* if rebirth square is where the piece stands,
         we've already generated all the relevant moves.
      */
      if (sq_rebirth==sq_departure) {
        return;
      }
      if (e[sq_rebirth] == vide) {
        anf2= nbcou;
        pi_departing=e[sq_departure];   /* Mars/Neutral bug */
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
           there's only ONE duplicate per arrival square
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
      do {   /* Echecs Plus */
        spec_departing= spec[sq_departure];
        sq_rebirth= (*marsrenai)(p,
                                 spec_departing,
                                 sq_departure,
                                 initsquare,
                                 initsquare,
                                 White);
        if (sq_rebirth==sq_departure || e[sq_rebirth]==vide) {
          pi_departing= e[sq_departure]; /* Mars/Neutral bug */
          
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

    square *bnp;
    for (bnp= boardnum; *bnp; bnp++)
      if (e[*bnp]==-p)
        empile(sq_departure,*bnp,messigny_exchange);
  }
} /* orig_gen_bl_piece */

void singleboxtype3_gen_bl_piece(square z, piece p) {
  numecoup save_nbcou = nbcou;
  unsigned int latent_prom = 0;
  square sq;
  for (sq = next_latent_pawn(initsquare,Black);
       sq!=initsquare;
       sq = next_latent_pawn(sq,Black))
  {
    piece pprom;
    for (pprom = next_singlebox_prom(vide,Black);
         pprom!=vide;
         pprom = next_singlebox_prom(pprom,Black))
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
}

void (*gen_bl_piece)(square z, piece p) = &orig_gen_bl_piece;

void genmove(Side camp)
{
  /* TODO hide away in one module per platform */
  /* Abbruch waehrend der gesammten Laufzeit mit <ESC> */
#if defined(ATARI)
# include <osbind.h>
# define STOP_ON_ESC
# define interupt (Bconstat(2) && (Bconin(2) == 27))
#endif /* ATARI */

#if defined(DOS)
#  if !defined(Windows)
#    if defined(__TURBOC__)
#      define STOP_ON_ESC
#      define interupt ((bioskey(1) != 0) && ((bioskey(0) >> 8) == 1))
#    endif /* __TURBOC__ */
#  endif

#  if defined(_MSC_VER)
#    define STOP_ON_ESC
#    define interupt (_bios_keybrd(_KEYBRD_READY) && ((_bios_keybrd(_KEYBRD_READ) >> 8) == 1))
#  endif /* _MSC_VER */

#  if defined(__GNUC__)
#    define STOP_ON_ESC
#    define interupt (kbhit() && (getkey() == 27)) /* ESC == 27 */
#  endif /* __GNUC__ */
#endif /* DOS */

#if defined(STOP_ON_ESC)
  if (interupt) {
    StdString(GetMsgString(InterMessage));
    StdString(" ");
    PrintTime();
    StdString("\n\n");
    CloseInput();
    /* for some other purposes I need a return value
    ** different from 0.  TLi
    ** exit(0);
    */
    exit(1);
  }
#endif /* STOP_ON_ESC */

  if (TSTFLAG(PieSpExFlags,Neutral))
    initneutre(advers(camp));
  if (nbply == 1)
    PushMagicViews();
  nextply();
  trait[nbply]= camp;
  we_generate_exact = false;
  init_move_generation_optimizer();

  if (CondFlag[exclusive]) {
    int nbrmates= 0;

    mateallowed[nbply]= true;

    if (camp == White)
      gen_wh_ply();
    else
      gen_bl_ply();

    while (encore()) {
      if (jouecoup() && leaf_is_goal_reached(camp,1))
        nbrmates++;
      repcoup();
    }

    mateallowed[nbply]= nbrmates < 2;
  }

  /* exact and consequent maximummers */
  if (camp == White) {
    /* let's first generate consequent moves */
    if (wh_exact) {
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
        init_move_generation_optimizer();
        gen_wh_ply();
      }
      we_generate_exact = false;
    } else
      gen_wh_ply();
  } else {
    /* let's first generate consequent moves */
    if (bl_exact) {
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
        init_move_generation_optimizer();
        gen_bl_ply();
      }
      we_generate_exact = false;
    } else
      gen_bl_ply();
  }
  finish_move_generation_optimizer();

} /* genmove(camp) */

void joueparrain(void)
{
  piece p= pprise[nbply-1];
  Flags pspec= pprispec[nbply-1];
  square cren= move_generation_stack[repere[nbply]].capture + move_generation_stack[nbcou].arrival - move_generation_stack[nbcou].departure;

  if (e[cren] == vide) {
    sqrenais[nbply]= cren;
    ren_parrain[nbply]= e[cren]= p;
    spec[cren]= pspec;

    if ((is_forwardpawn(p)
         && PromSq(advers(trait[nbply-1]), cren))
        || (is_reversepawn(p)
            && ReversePromSq(advers(trait[nbply-1]), cren))) {
      /* captured white pawn on eighth rank: promotion ! */
      /* captured black pawn on first rank: promotion ! */
      piece pprom= cir_prom[nbply];

      if (TSTFLAG(pspec, Chameleon))
        cir_cham_prom[nbply]= true;
      if (pprom == vide)
        cir_prom[nbply]= pprom= getprompiece[vide];
      if (p < vide)
        pprom= -pprom;
      e[cren]= pprom;
      nbpiece[pprom]++;
      if (cir_cham_prom[nbply])
        SETFLAG(pspec, Chameleon);
      spec[cren]= pspec;
    } else
      nbpiece[p]++;
    if (TSTFLAG(pspec, Neutral))
      setneutre(cren);
  }
} /* end of joueparrain */

piece pdisp[maxply+1];
Flags pdispspec[maxply+1];
square sqdep[maxply+1];

boolean patience_legal()
{
  square bl_last_vacated= initsquare, wh_last_vacated= initsquare;
  ply nply;
  /* n.b. inventor rules that R squares are forbidden after
     castling but not yet implemented */

  for (nply= nbply - 1 ; nply > 1 && !bl_last_vacated ; nply--)
    if (trait[nply] == Black)
      bl_last_vacated= sqdep[nply];
  for (nply= nbply - 1 ; nply > 1 && !wh_last_vacated ; nply--)
    if (trait[nply] == White)
      wh_last_vacated= sqdep[nply];
  return !((wh_last_vacated && e[wh_last_vacated]) ||
           (bl_last_vacated && e[bl_last_vacated]));
}

void find_mate_square(Side camp);

int direction(square from, square to) {
  int dir= to-from;
  int hori= to%onerow-from%onerow;
  int vert= to/onerow-from/onerow;
  int i=7;
  while ((hori%i) || (vert%i))
    i--;

  return dir/i;
}

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

  for (i= 8; i > 0; i--, z+= 16)
    for (j= 8; j > 0; j--, z++) {
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

square next_latent_pawn(square s, Side c) {
  piece pawn;
  int  i, delta;

  pawn=  c==White ? pb : pn;
  delta= c==White ?+dir_left :+dir_right;

  if (s==initsquare) {
    i = 0;
    s = c==White ? haut : bas;
  }
  else {
    i = c==White ? haut-s+1 : s-bas+1;
    s += delta;
  }

  for (; i<8; ++i, s+=delta) {
    if (e[s]==pawn) {
      return s;
    }
  }

  return initsquare;
}

piece next_singlebox_prom(piece p, Side c) {
  piece pprom;
  for (pprom = getprompiece[p];
       pprom!=vide;
       pprom = getprompiece[pprom])
  {
    assert(pprom<boxsize);
    if (nbpiece[c==White ? pprom : -pprom] < maxinbox[pprom])
      return pprom;
  }

  return vide;
}

#if defined(DEBUG)
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

boolean jouecoup_legality_test(unsigned int oldnbpiece[derbla],
                               square sq_rebirth) {
  if (CondFlag[schwarzschacher] && trait[nbply]==Black)
    return echecc(White);

  if (CondFlag[extinction]) {
    piece p;
    for (p= roib; p<derbla; p++) {
      if (oldnbpiece[p]>0
          && !nbpiece[trait[nbply]==White ? p : -p])
      {
        return false;
      }
    }
  }

  return (!jouetestgenre
          || (
            (!jouetestgenre1 || (
               (!CondFlag[blackultraschachzwang]
                || trait[nbply]==White
                || echecc(White))
               && (!CondFlag[whiteultraschachzwang]
                   || trait[nbply]==Black
                   || echecc(Black))
              ))
            &&
            ((!flag_testlegality) || pos_legal())
            && (!flagAssassin || (sq_rebirth != rb && sq_rebirth != rn))
            && (!testdblmate || (rb!=initsquare && rn!=initsquare))
            && (!CondFlag[patience] || PatienceB || patience_legal())
            /* don't call patience_legal if TypeB as obs > vide ! */
            && (trait[nbply] == White ? BGL_white >= 0 : BGL_black >= 0)
            ));
}

boolean jouecoup(void) {
  square  sq_departure,
    sq_arrival,
    sq_capture,
    sq_rebirth= initsquare,  /* initialised ! */
    sq_hurdle= initsquare,
    prev_rb,
    prev_rn;
  piece   pi_captured,
    pi_departing,
    pi_arriving,
    pi_reborn,
    pi_hurdle;
  Flags   spec_pi_captured;
  Flags   spec_pi_moving;
  boolean rochade=false;

  unsigned int prev_nbpiece[derbla];

  Side traitnbply= trait[nbply];

  move_generation_elmt* move_gen_top = move_generation_stack+nbcou;

#if defined(DEBUG)
  nbrtimes++;
#endif

  /* Orphans/refl. KK !!!! */
  /* now also for phantomchess - schoen krampfig */

  if (jouegenre)
  {
    if (exist[Orphan]
        || exist[Friend]
        || calc_whrefl_king
        || calc_blrefl_king
        || CondFlag[phantom]) {
      while (move_generation_stack[nbcou].arrival == initsquare) {
        nbcou--;
      }
      move_gen_top = move_generation_stack+nbcou;
    }

    if (CondFlag[extinction]) {
      piece p;
      for (p= roib; p < derbla; p++) {
        prev_nbpiece[p]= nbpiece[ traitnbply==White ? p : -p];
      }
    }
  }

  prev_rb= RB_[nbply]= rb;
  prev_rn= RN_[nbply]= rn;

  sq_departure= sqdep[nbply]= move_gen_top->departure;
  sq_arrival= move_gen_top->arrival;
  sq_capture= move_gen_top->capture;
  
  if (jouegenre)
  {
    rochade_sq[nbcou]= initsquare;
    if (sq_capture >= maxsquare + bas)
    {
      rochade_sq[nbcou]= sq_capture - maxsquare;
      rochade_pc[nbcou]= e[rochade_sq[nbcou]];
      rochade_sp[nbcou]= spec[rochade_sq[nbcou]];
      sq_capture= sq_arrival;
      rochade= true;
    } 

    if (CondFlag[amu])
      att_1[nbply]= att_once(sq_departure);

    if (CondFlag[imitators])
    {
      if (sq_capture == queenside_castling)
        joueim(+dir_right);
      else if (rochade)
        joueim((3*sq_arrival - sq_departure - rochade_sq[nbcou]) / 2);
      else if (sq_capture!=kingside_castling) /* joueim(0) (do nothing) if OO */
        joueim(sq_arrival-sq_departure);
    }
  }

  spec_pi_moving= jouespec[nbply]= spec[sq_departure];
  pi_arriving= pi_departing= pjoue[nbply]= e[sq_departure];

  spec_pi_captured= pprispec[nbply]= spec[sq_capture];
  pi_captured= pprise[nbply]= e[sq_capture];

  if (sq_arrival==nullsquare)
    return true;

  if (anyantimars && sq_departure==sq_capture)
  {
    spec_pi_captured= pprispec[nbply]= 0;
    pi_captured= pprise[nbply]= vide;
  }
  
  pdisp[nbply]= vide;
  pdispspec[nbply]= 0;

  if (jouegenre)
  {
    if (CondFlag[blsupertrans_king]
        && traitnbply==Black
        && ctrans[nbcou]!=vide)
    {
      rn=initsquare;
      pi_arriving=ctrans[nbcou];
    }
    if (CondFlag[whsupertrans_king]
        && traitnbply==White
        && ctrans[nbcou]!=vide)
    {
      rb=initsquare;
      pi_arriving=ctrans[nbcou];
    }

    if (CondFlag[singlebox] && SingleBoxType==singlebox_type3
        && sb3[nbcou].what!=vide) {
      --nbpiece[e[sb3[nbcou].where]];
      e[sb3[nbcou].where] = sb3[nbcou].what;
      ++nbpiece[sb3[nbcou].what];
      if (sq_departure==sb3[nbcou].where) {
        pi_departing = pi_arriving = sb3[nbcou].what;
      }
    }
  }

  if (TSTFLAG(spec_pi_moving, ColourChange)) {
    sq_hurdle= chop[nbcou];
  }

  switch (sq_capture) {
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
    if (CondFlag[einstein]) {
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
    if (CondFlag[einstein]) {
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

  if (rochade)
  {
     square sq_castle= (sq_departure + sq_arrival) / 2;
     e[sq_castle] = e[rochade_sq[nbcou]];
     spec[sq_castle] = spec[rochade_sq[nbcou]];
     e[rochade_sq[nbcou]] = vide;
     CLEARFL(spec[rochade_sq[nbcou]]);
     if (rn == rochade_sq[nbcou])
       rn= sq_castle;
     if (rb == rochade_sq[nbcou])
       rb= sq_castle;
  }

  e[sq_departure]= CondFlag[haanerchess] ? obs : vide;
  spec[sq_departure]= 0;

  if (PatienceB) {
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
    if (TSTFLAG(spec_pi_moving, Kamikaze)) {
      if (pi_captured != vide) {
        if (!anycirce) {
          spec_pi_moving= 0;
          pi_arriving= vide;
          /* to avoid useless promotions of a vanishing pawn */
        }
      }
    }
    if (CondFlag[linechamchess])
      pi_arriving= linechampiece(pi_arriving, sq_arrival);

    if (CondFlag[chamchess])
      pi_arriving= champiece(pi_arriving);

    if (TSTFLAG(spec_pi_moving, Chameleon))
      pi_arriving= champiece(pi_arriving);

    if (CondFlag[norsk])
      pi_arriving= norskpiece(pi_arriving);

  if (pi_captured != vide
      && ((CondFlag[protean]
           && (!rex_protean_ex || !TSTFLAG(spec_pi_moving, Royal)))
          || TSTFLAG(spec_pi_moving, Protean))) {
    if (pi_departing < vide) {
       pi_arriving = -pi_captured;
       if (pi_arriving == pn)
         pi_arriving = reversepn;
       else if (pi_arriving == reversepn)
         pi_arriving = pn;
    } else {
       pi_arriving = -pi_captured;
       if (pi_arriving == pb)
         pi_arriving = reversepb;
       else if (pi_arriving == reversepb)
         pi_arriving = pb;
    }
  }

  } /* change_moving_piece */

  if (abs(pi_departing) == andergb) {
    square sq= sq_arrival - direction(sq_departure, sq_arrival);

    if (!TSTFLAG(spec[sq], Neutral) && (sq != rb) && (sq != rn)) {
      change(sq);
      CHANGECOLOR(spec[sq]);
    }
  } /* andergb */

  ep[nbply]= ep2[nbply]= initsquare;
  if (is_pawn(pi_departing)) {
    if (pi_captured==vide) {
      /* ep capture */
      if (CondFlag[phantom]) {
        int col_diff, rank_j;

        col_diff= sq_arrival%onerow - sq_departure%onerow,
          rank_j= sq_arrival/onerow;

        if (rank_j == 11) { /* 4th rank */
          switch (col_diff) {
          case 0:
            if (pi_departing==pb
                && sq_departure!=sq_arrival+dir_down)
              ep[nbply]= sq_arrival+dir_down;
            break;

          case -2:
            if (pi_departing==pbb
                && sq_departure!=sq_arrival+dir_down+dir_left)
              ep[nbply]= sq_arrival+dir_down+dir_left;
            break;

          case 2:
            if (pi_departing==pbb
                && sq_departure!=sq_arrival+dir_down+dir_right)
              ep[nbply]= sq_arrival+dir_down+dir_right;
            break;
          }
        }
        else if (rank_j == 12) { /* 5th rank */
          switch (col_diff) {
          case 0:
            if (pi_departing==pn
                && sq_departure!=sq_arrival+dir_up)
              ep[nbply]= sq_arrival+dir_up;
            break;
          case -2:
            if (pi_departing==pbn
                && sq_departure!=sq_arrival+dir_up+dir_left)
              ep[nbply]= sq_arrival+dir_up+dir_left;
            break;
          case 2:
            if (pi_departing==pbn
                && sq_departure!=sq_arrival+dir_up+dir_right)
              ep[nbply]= sq_arrival+dir_up+dir_right;
            break;
          }
        }
      } /* CondFlag[phantom] */
      else
      {
        square ii = anyantimars ? cmren[nbcou] : sq_departure;
        switch (abs(pi_departing)) {
        case Pawn:
        case ReversePawn:
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
        }
      }
    }
    /* promotion */
    if (PromSq(is_reversepawn(pi_departing)^trait[nbply],sq_arrival)
        && ((!CondFlag[protean] && !TSTFLAG(spec_pi_moving, Protean))
            || pi_captured == vide)) {
      /* moved to here because of anticirce
       */
      if ((pi_arriving= norm_prom[nbply]) == vide) {
        if (!CondFlag[noiprom] && Iprom[nbply]) {
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
              pi_arriving = abs(pi_departing);
          }
          else
          {
            pi_arriving= getprompiece[vide];

            if (CondFlag[frischauf])
              SETFLAG(spec_pi_moving, FrischAuf);

            if (pi_captured != vide && anyanticirce) {
#if defined(BETTER_READABLE)
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
                                            sq_departure))
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
      {
        if (CondFlag[frischauf])
          SETFLAG(spec_pi_moving, FrischAuf);
      }

      if (!(!CondFlag[noiprom] && Iprom[nbply])) {
        if (TSTFLAG(spec_pi_moving, Chameleon)
            && is_pawn(pi_departing)) {
          norm_cham_prom[nbply]= true;
        }
        if (pi_departing < vide) {
          pi_arriving = -pi_arriving;
        }

        /* so also promoted neutral pawns have the correct color and
         * an additional call to setneutre is not required.
         */
        if (norm_cham_prom[nbply])
          SETFLAG(spec_pi_moving, Chameleon);
      }
      else {
        pi_arriving= vide; /* imitator-promotion */
      }
    }
  } /* is_pawn() */

  if (sq_arrival != sq_capture) {
    e[sq_capture]= vide;
    spec[sq_capture]= 0;
  }

  if (pi_captured != vide) {
    nbpiece[pi_captured]--;

    if (sq_capture == rb)
      rb= initsquare;
    if (sq_capture == rn)
      rn= initsquare;
  }

  if (change_moving_piece) {
    if (CondFlag[degradierung]
        && !is_pawn(pi_departing)
        && sq_departure != prev_rn
        && sq_departure != prev_rb
        && (traitnbply == Black
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
         && pi_arriving != -pi_captured
         && (((traitnbply == Black)
              && (sq_departure != prev_rn))
             || ((traitnbply == White)
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
          && (!TSTFLAG(spec_pi_moving, White) || neutcoul == Black))
      {
        pi_arriving= -pi_arriving;
      }
    }

    /* this seems to be misplaced -- it's checked every time TLi */
    /* therefore moved to this place ! NG */
    if (CondFlag[antiandernach]
        && pi_captured == vide
        && sq_departure != prev_rn
        && sq_departure != prev_rb )
    {
      /* the following also copes correctly with neutral */
      CLRFLAG(spec_pi_moving, Black);
      CLRFLAG(spec_pi_moving, White);
      CLRFLAG(spec_pi_moving, Neutral);
      if (traitnbply == Black) {
        SETFLAG(spec_pi_moving, White);
        pi_arriving= abs(pi_arriving);
      }
      else {
        SETFLAG(spec_pi_moving, Black);
        pi_arriving= -abs(pi_arriving);
      }
    } /* CondFlag[antiandernach] ... */

    if (CondFlag[champursue]
      && sq_arrival == move_generation_stack[repere[nbply]].departure
      && sq_departure != prev_rn
      && sq_departure != prev_rb)
    {
      /* the following also copes correctly with neutral */
      CLRFLAG(spec_pi_moving, Black);
      CLRFLAG(spec_pi_moving, White);
      CLRFLAG(spec_pi_moving, Neutral);
      if (traitnbply == Black) {
        SETFLAG(spec_pi_moving, White);
        pi_arriving= abs(pi_arriving);
      }
      else {
        SETFLAG(spec_pi_moving, Black);
        pi_arriving= -abs(pi_arriving);
      }
    } /* CondFlag[antiandernach] ... */

    if ((CondFlag[traitor]
         && traitnbply == Black
         && sq_arrival<=square_h4
         && !TSTFLAG(spec_pi_moving, Neutral))
        || (TSTFLAG(spec_pi_moving, Volage)
            && SquareCol(sq_departure) != SquareCol(sq_arrival))
        || (TSTFLAG(sq_spec[sq_arrival], MagicSq)
            && prev_rn != sq_departure
            && prev_rb != sq_departure))
    {
      CHANGECOLOR(spec_pi_moving);
      if (!CondFlag[hypervolage])
        CLRFLAG(spec_pi_moving, Volage);
      pi_arriving= -pi_arriving;
    }

    if (CondFlag[einstein]
        && !(CondFlag[antieinstein] && pi_captured != vide))
    {
      pi_arriving= (pi_captured==vide) != CondFlag[reveinstein]
        ? dec_einstein(pi_arriving)
        : inc_einstein(pi_arriving);
    }

  } /* change_moving_piece */

  if (sq_departure == prev_rb) {
    if (rb!=initsquare)
      rb= sq_arrival;
    CLRFLAGMASK(castling_flag[nbply],ke1_cancastle);
  }
  if (sq_departure == prev_rn) {
    if (rn!=initsquare)
      rn= sq_arrival;
    CLRFLAGMASK(castling_flag[nbply],ke8_cancastle);
  }

  /* Needed for castling */
  if (castling_supported) {
    /* pieces vacating a1, h1, a8, h8 */
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
    /* pieces arriving at a1, h1, a8, h8 and possibly capturing a rook */
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
      || (CondFlag[antiandernach] && pi_captured==vide)
      || (CondFlag[champursue] && sq_arrival == move_generation_stack[repere[nbply]].departure)
      || (CondFlag[norsk])
      || (CondFlag[protean]
          && (pi_captured!=vide || abs(pi_departing)==ReversePawn))
    )
  {
    if (castling_supported) {
      if (abs(pi_arriving) == Rook) {
        if (TSTFLAG(spec_pi_moving, White)) {
          /* new white/neutral rook */
          if (sq_arrival == square_h1)
            SETFLAGMASK(castling_flag[nbply],rh1_cancastle);
          else if (sq_arrival == square_a1)
            SETFLAGMASK(castling_flag[nbply],ra1_cancastle);
        }
        if (TSTFLAG(spec_pi_moving, Black)) {
          /* new black/neutral rook */
          if (sq_arrival == square_h8)
            SETFLAGMASK(castling_flag[nbply],rh8_cancastle);
          else if (sq_arrival == square_a8)
            SETFLAGMASK(castling_flag[nbply],ra8_cancastle);
        }
      }
    } /* castling_supported */
  } /* andernach || antiandernach ... */

  if (TSTFLAG(spec_pi_moving, HalfNeutral))
  {
    if (TSTFLAG(spec_pi_moving, Neutral))
    {
      CLRFLAG(spec_pi_moving,advers(traitnbply));
      CLRFLAG(spec_pi_moving, Neutral);
      pi_arriving= traitnbply==Black ? -abs(pi_arriving) : abs(pi_arriving);

      if (rn == sq_arrival && traitnbply == White)
        rn= initsquare;

      if (rb == sq_arrival && traitnbply == Black)
        rb= initsquare;
    }
    else if (traitnbply==Black) {
      if (TSTFLAG(spec_pi_moving, Black)) {
        SETFLAG(spec_pi_moving, Neutral);
        SETFLAG(spec_pi_moving, White);
        pi_arriving= abs(pi_arriving);
        if (rn == sq_arrival)
          rb = sq_arrival;
      }
    }
    else if (traitnbply==White) {
      if (TSTFLAG(spec_pi_moving, White)) {
        SETFLAG(spec_pi_moving, Neutral);
        SETFLAG(spec_pi_moving, Black);
        pi_arriving= -abs(pi_arriving);
        if (rb == sq_arrival)
          rn = sq_arrival;
      }
    }
  }

  e[sq_arrival]= pi_arriving;
  spec[sq_arrival] = spec_pi_moving;
  jouearr[nbply]= pi_arriving;

  if (pi_departing != pi_arriving) {
    nbpiece[pi_departing]--;
    nbpiece[pi_arriving]++;
  }

  if (jouegenre) {
    if (TSTFLAG(spec_pi_moving, HalfNeutral)
        && TSTFLAG(spec_pi_moving, Neutral))
      setneutre(sq_arrival);

    /* Duellantenschach */
    if (CondFlag[duellist]) {
      if (traitnbply == Black) {
        whduell[nbply]= whduell[nbply - 1];
        blduell[nbply]= sq_arrival;
      }
      else {
        blduell[nbply]= blduell[nbply - 1];
        whduell[nbply]= sq_arrival;
      }
    }

    if (CondFlag[singlebox] && SingleBoxType==singlebox_type2) {
      Side adv = advers(traitnbply);

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
        assert(e[sb2[nbply].where] == (adv==White ? pb : pn));
        assert(sb2[nbply].what!=vide);
        --nbpiece[e[sb2[nbply].where]];
        e[sb2[nbply].where] =   adv==White
          ? sb2[nbply].what
          : -sb2[nbply].what;
        ++nbpiece[e[sb2[nbply].where]];
      }
    }

    /* AntiCirce */
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
      if ((is_forwardpawn(pi_departing)
           && PromSq(traitnbply,sq_rebirth))
          || (is_reversepawn(pi_departing)                 
              && ReversePromSq(traitnbply,sq_rebirth)))
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

      if (castling_supported) {
        piece abspja= abs(pi_arriving);
        if (abspja == King) {
          if (TSTFLAG(spec_pi_moving, White)
              && sq_rebirth == square_e1
              && (!CondFlag[dynasty] || nbpiece[roib]==1)) {
            /* white king new on e1 */
            SETFLAGMASK(castling_flag[nbply],ke1_cancastle);
          }
          else if (TSTFLAG(spec_pi_moving, Black)
                   && sq_rebirth == square_e8
                   && (!CondFlag[dynasty] || nbpiece[roin]==1)) {
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
    
    if (flag_outputmultiplecolourchanges)
    {
      colour_change_sp[nbply] = colour_change_sp[nbply - 1];
    }
        
    if (flag_magic)
    {
      PushMagicViews();
      ChangeMagic(nbply, flag_outputmultiplecolourchanges);
    }

    if (CondFlag[sentinelles]) {
      if (sq_departure>=square_a2 && sq_departure<=square_h7
          && !is_pawn(pi_departing))
      {
        if (SentPionNeutral) {
          if (TSTFLAG(spec_pi_moving, Neutral)) {
            nbpiece[e[sq_departure]= sentinelb]++;
            SETFLAG(spec[sq_departure], Neutral);
            setneutre(sq_departure);
          }
          else if ((traitnbply==Black) != SentPionAdverse) {
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
          {
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
        else if ((traitnbply==Black) != SentPionAdverse) {
          if (   nbpiece[sentineln] < max_pn
                 && nbpiece[sentinelb]+nbpiece[sentineln]<max_pt
                 && (  !flagparasent
                       ||(nbpiece[sentineln]
                          <= nbpiece[sentinelb]
                          +(pi_captured==sentinelb ? 1 : 0))))
          {
            nbpiece[e[sq_departure]= sentineln]++;
            SETFLAG(spec[sq_departure], Black);
            senti[nbply]= true;
          }
        }
        else if ( nbpiece[sentinelb] < max_pb
                  && nbpiece[sentinelb]+nbpiece[sentineln]<max_pt
                  && (!flagparasent
                      ||(nbpiece[sentinelb]
                         <= nbpiece[sentineln]
                         + (pi_captured==sentineln ? 1 : 0))))
        {
          nbpiece[e[sq_departure]= sentinelb]++;
          SETFLAG(spec[sq_departure], White);
          senti[nbply]= true;
        }
      }
    }

    if (anycirce) {
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
          if ((e[sq_rebirth] == vide)
              && !( CondFlag[contactgrid]
                    && nogridcontact(sq_rebirth)))
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
                  && (abs(pi_arriving) == King)) {
                if (TSTFLAG(spec_pi_moving, White)
                    && sq_rebirth == square_e1
                    && (!CondFlag[dynasty] || nbpiece[roib]==1)) {
                  /* white king reborn on e1 */
                  SETFLAGMASK(castling_flag[nbply],ke1_cancastle);
                }
                else if (TSTFLAG(spec_pi_moving, Black)
                         && sq_rebirth == square_e8
                         && (!CondFlag[dynasty] || nbpiece[roin]==1)) {
                  /* black king reborn on e8 */
                  SETFLAGMASK(castling_flag[nbply],ke8_cancastle);
                }
              }
            }
            if (castling_supported
                && (abs(pi_arriving) == Rook)) {
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

      /* normal circe-rebirth of captured piece moved to repcoup
       * sqrenais[nbply]= initsquare
       */
      if (CondFlag[parrain]) {
        if ((pprise[nbply-1] != vide))
          joueparrain();
      }
      else {
        if (pi_captured != vide) {
          if (anyclone
              && sq_departure != prev_rn && sq_departure != prev_rb) {
            /* Circe Clone - new implementation
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
            if (CondFlag[chamcirce]) {
              pi_reborn= ChamCircePiece(pi_captured);
            }
            else if (CondFlag[antieinstein]) {
              pi_reborn= inc_einstein(pi_captured);
            }
            else {
              pi_reborn= pi_captured;
            }
          }

          if (CondFlag[couscous])
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
              && (sq_capture == prev_rb || sq_capture == prev_rn))
          {
            /* ordinary circe and (isardam, brunner or
            ** ohneschach) it can happen that the king is
            ** captured while testing the legality of a
            ** move. Therefore prevent the king from being
            ** reborn.
            */
            sq_rebirth= initsquare;
          }

          if ( (e[sq_rebirth] == vide || flagAssassin)
               && !( CondFlag[contactgrid]
                     && nogridcontact(sq_rebirth)))
          {
            sqrenais[nbply]= sq_rebirth;
            if (rex_circe) {
              /* neutral K */
              if (prev_rb == sq_capture) {
                rb= sq_rebirth;
              }
              if (prev_rn == sq_capture) {
                rn= sq_rebirth;
              }

              if (castling_supported
                  && (abs(pi_reborn) == King)) {
                if (TSTFLAG(spec_pi_captured, White)
                    && sq_rebirth == square_e1
                    && (!CondFlag[dynasty] || nbpiece[roib]==1)) {
                  /* white king reborn on e1 */
                  SETFLAGMASK(castling_flag[nbply],ke1_cancastle);
                }
                else if (TSTFLAG(spec_pi_captured, Black)
                         && sq_rebirth == square_e8
                         && (!CondFlag[dynasty] || nbpiece[roin]==1)) {
                  /* black king reborn on e8 */
                  SETFLAGMASK(castling_flag[nbply],ke8_cancastle);
                }
              }
            }

            if (castling_supported
                && (abs(pi_reborn) == Rook)) {
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
            {
              /* captured white pawn on eighth rank: promotion ! */
              /* captured black pawn on first rank: promotion ! */
              piece pprom= cir_prom[nbply];
              if (pprom == vide) {
                cir_prom[nbply]= pprom= getprompiece[vide];
              }
              pi_reborn = pi_reborn < vide ? -pprom : pprom;
              if (cir_cham_prom[nbply]) {
                SETFLAG(spec_pi_captured, Chameleon);
              }
            }
            if (TSTFLAG(spec_pi_captured, Volage)
                && SquareCol(sq_rebirth) != SquareCol(sq_capture))
            {
              pi_reborn= -pi_reborn;
              CHANGECOLOR(spec_pi_captured);
              if (!CondFlag[hypervolage]) {
                CLRFLAG(spec_pi_captured, Volage);
              }
            }
            if (flagAssassin) {
              nbpiece[pdisp[nbply]=e[sq_rebirth]]--;
              pdispspec[nbply]=spec[sq_rebirth];
            }
            nbpiece[e[sq_rebirth]= pi_reborn]++;
            spec[sq_rebirth]= spec_pi_captured;
          }
        }
      }
    }

    if (bl_royal_sq != initsquare)
      rn= bl_royal_sq;

    if (wh_royal_sq != initsquare)
      rb= wh_royal_sq;

    if (CondFlag[republican])
    {
      if (traitnbply==White)
      {
        if (flag_writinglinesolution
            && repub_k[nbply]!=initsquare)
        {
          rn = repub_k[nbply];
          e[rn] = roin;
          nbpiece[roin]++;
        }
        else if (rn==initsquare && !is_republican_suspended)
        {
          is_republican_suspended = true;
          find_mate_square(White);
          repub_k[nbply] = super[nbply]<= haut ? super[nbply] : initsquare;
          if (RepublicanType==republican_type1)
          {
            /* In type 1, Republican chess is suspended (and hence
             * play is over) once a king is inserted. */
            if (repub_k[nbply]==initsquare)
              is_republican_suspended = false;
          }
          else
            /* In type 2, on the other hand, Republican chess is
             * continued, and the side just "mated" can attempt to
             * defend against the mate by inserting the opposite
             * king. */
            is_republican_suspended = false;
        }
        else
        {
          repub_k[nbply] = initsquare;
          super[nbply] = haut+1;
        }
      }
      else
      {
        if (flag_writinglinesolution
            && repub_k[nbply]!=initsquare)
        {
          rb = repub_k[nbply];
          e[rb] = roib;
          nbpiece[roib]++;
        }
        else if (rb==initsquare && !is_republican_suspended)
        {
          is_republican_suspended = true;
          find_mate_square(Black);
          repub_k[nbply] = super[nbply]<= haut ? super[nbply] : initsquare;
          if (RepublicanType==republican_type1)
          {
            if (repub_k[nbply]==initsquare)
              is_republican_suspended = false;
          }
          else
            is_republican_suspended = false;
        }
        else
        {
          repub_k[nbply] = initsquare;
          super[nbply] = haut+1;
        }
      }
    } /* republican */

    if (CondFlag[actrevolving])
      RotateMirror(rot270);

    if (CondFlag[arc])
    {
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

    if (traitnbply==White
        ? CondFlag[white_oscillatingKs]
        : CondFlag[black_oscillatingKs]) {
      boolean priorcheck= false;
      square temp= rb;
      piece temp1= e[rb];
      Flags temp2= spec[rb];

      oscillatedKs[nbply]= false;
      if (OscillatingKingsTypeB[traitnbply])
        priorcheck= echecc(traitnbply);
      if ((oscillatedKs[nbply]= (!OscillatingKingsTypeC[traitnbply]
                                 || echecc(advers(traitnbply)))))
      {
        e[rb]= e[rn];
        spec[rb]= spec[rn];

        e[rn]= temp1;
        spec[rn]= temp2;
        rb= rn;
        rn= temp;
        CLRFLAGMASK(castling_flag[nbply],ke1_cancastle|ke8_cancastle);
        if (rb==square_e1)
          SETFLAGMASK(castling_flag[nbply],ke1_cancastle);
        if (rn==square_e8)
          SETFLAGMASK(castling_flag[nbply],ke8_cancastle);
        if (OscillatingKingsTypeB[traitnbply] && priorcheck)
          return false;
      }
    }

    if (TSTFLAG(spec_pi_moving, ColourChange)) {
      if (abs(pi_hurdle= e[sq_hurdle]) > roib) {
        nbpiece[pi_hurdle]--;
        e[sq_hurdle]= -pi_hurdle;
        nbpiece[-pi_hurdle]++;
        CHANGECOLOR(spec[sq_hurdle]);
      }
    }

    if (CondFlag[dynasty]) {
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

    if (CondFlag[strictSAT] && SATCheck)
    {
      WhiteStrictSAT[nbply]= echecc_normal(White);
      BlackStrictSAT[nbply]= echecc_normal(Black);
    }

    if (CondFlag[masand]
        && echecc(advers(traitnbply))
        && observed(traitnbply == White ? rn : rb,
                    move_gen_top->arrival))
      change_observed(move_gen_top->arrival, flag_outputmultiplecolourchanges);
        
    if (!BGL_whiteinfinity
        && (BGL_global || traitnbply == White))
    {
      BGL_white -= BGL_move_diff_code[abs(move_gen_top->departure
                                          -move_gen_top->arrival)];
    }
    if (!BGL_blackinfinity && (BGL_global || traitnbply == Black))
    {
      BGL_black -= BGL_move_diff_code[abs(move_gen_top->departure
                                          -move_gen_top->arrival)];
    }
  } /* if (jouegenre) */


  return jouecoup_legality_test(prev_nbpiece,sq_rebirth);
} /* end of jouecoup */

void IncrementMoveNbr(void)
{
  if (MoveNbr>=RestartNbr)
  {
    sprintf(GlobalStr,"%3d  (", MoveNbr);
    StdString(GlobalStr);
    ecritcoup(no_goal);
    if (!flag_regression)
    {
      StdString("   ");
      PrintTime();
    }
#if defined(HASHRATE)
    StdString("   ");
    HashStats(0, NULL);
#endif
    StdString(")\n");
  }

  MoveNbr++;
}

void repcoup(void) {
  square sq_rebirth;
  piece pi_departing, pi_captured, pi_arriving, pi_hurdle;
  Flags spec_pi_moving;
  boolean next_prom = true;
  square nextsuper= initsquare;
  square sq_hurdle;
  boolean rochade=false;

  move_generation_elmt* move_gen_top = move_generation_stack+nbcou;

  square sq_departure= move_gen_top->departure;
  square sq_arrival= move_gen_top->arrival;
  square sq_capture= move_gen_top->capture;

  if (jouegenre) {
    if (sq_capture >= maxsquare + bas)
    {
       sq_capture= sq_arrival;
       rochade= true;
    }

    if (!BGL_whiteinfinity && (BGL_global || trait[nbply] == White))
    {
      BGL_white += BGL_move_diff_code[abs(sq_departure-sq_arrival)];
    }
    if (!BGL_blackinfinity && (BGL_global || trait[nbply] == Black))
    {
      BGL_black += BGL_move_diff_code[abs(sq_departure-sq_arrival)];
    }
        
    if (flag_magic)
    {
        ChangeMagic(nbply, false);
    }
    
    if (CondFlag[masand]
        && echecc(advers(trait[nbply]))
        && observed(trait[nbply] == White ? rn : rb,
                    sq_arrival))
      change_observed(sq_arrival, false);

    if (oscillatedKs[nbply])  /* for Osc Type C */
    {
      square temp= rb;
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

    if (CondFlag[arc]) {
      /* RotateMirror(rot90); */
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

    if (CondFlag[singlebox] && SingleBoxType==singlebox_type2) {
      sb2[nbply+1].where = initsquare;
      sb2[nbply+1].what = vide;

      if (sb2[nbply].where!=initsquare) {
        Side adv = advers(trait[nbply]);

        assert(sb2[nbply].what!=vide);
        --nbpiece[e[sb2[nbply].where]];
        e[sb2[nbply].where] = adv==White ? pb : pn;
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

  if (sq_arrival==nullsquare)
  {
    nbcou--;
    return;
  }

  if (jouegenre) {
    if (CondFlag[singlebox] && SingleBoxType==singlebox_type3
        && sb3[nbcou].what!=vide) {
      piece pawn = trait[nbply]==White ? pb : pn;
      e[sb3[nbcou].where] = pawn;
      if (sq_departure!=sb3[nbcou].where) {
        --nbpiece[sb3[nbcou].what];
        ++nbpiece[pawn];
      }
    }

    if (TSTFLAG(spec_pi_moving, ColourChange)) {
      sq_hurdle= chop[nbcou];
      if (abs(pi_hurdle= e[sq_hurdle]) > roib) {
        nbpiece[pi_hurdle]--;
        e[sq_hurdle]= -pi_hurdle;
        nbpiece[-pi_hurdle]++;
        CHANGECOLOR(spec[sq_hurdle]);
      }
    }
  }

  castling_flag[nbply]= castling_flag[nbply-1];

  switch (sq_capture) {
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
    if (CondFlag[einstein]) {
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
    /* reset everything */
    break;

  case queenside_castling:
    if (CondFlag[einstein]) {
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
    /* reset everything */
    break;

  } /* switch (sq_capture) */

  /* the supercirce rebirth square has to be evaluated here in the
  ** position after the capture. Otherwise it is more difficult to
  ** allow the rebirth on the original square of the capturing piece
  ** or in connection with locust or e.p. captures.
  */
  if ((CondFlag[supercirce] && pi_captured != vide)
      || isapril[abs(pi_captured)]
      || (CondFlag[antisuper] && pi_captured != vide))
  {
    nextsuper= super[nbply];
    while ((e[++nextsuper] != vide) && (nextsuper < haut))
      ;
    if (CondFlag[antisuper]
        && AntiCirCheylan
        && nextsuper==sq_capture)
      while ((e[++nextsuper] != vide) && (nextsuper < haut))
        ;
  }

  if (CondFlag[republican])
  {
    square sq = repub_k[nbply];
    if (sq!=initsquare)
    {
      e[sq] = vide;
      if (sq==rn)
      {
        rn = initsquare;
        nbpiece[roin]--;
      }
      if (sq==rb)
      {
        rb = initsquare;
        nbpiece[roib]--;
      }

      if (RepublicanType==republican_type1)
        /* Republican chess was suspended when the move was played. */
        is_republican_suspended = false;
    }
  }
  
  /* first delete all changes */
  if (repgenre) {
    if (senti[nbply]) {
      --nbpiece[e[sq_departure]];
      senti[nbply]= false;
    }
    if (CondFlag[imitators])
    {
      if (sq_capture == queenside_castling)
        joueim(+dir_left);
      else if (rochade)
        joueim((sq_departure + rochade_sq[nbcou] - 3*sq_arrival) / 2);
      else if (sq_capture != kingside_castling) /* joueim(0) (do nothing) if OO */
        joueim(sq_departure - sq_arrival);      /* verschoben TLi */
    }

    if (TSTFLAG(PieSpExFlags, Neutral)) {
      /* the following is faster !  TLi
       * initneutre((pi_departing > vide) ? White : Black);
       */

      if (TSTFLAG(spec_pi_moving, Neutral) &&
          (pi_departing < vide ? Black : White) != neutcoul)
        pi_departing= -pi_departing;
      if (TSTFLAG(pprispec[nbply], Neutral) &&
          (pi_captured < vide ? Black : White) != neutcoul)
        pi_captured= -pi_captured;
    }
    if ((sq_rebirth= sqrenais[nbply]) != initsquare) {
      sqrenais[nbply]= initsquare;
      if (sq_rebirth != sq_arrival) {
        nbpiece[e[sq_rebirth]]--;
        e[sq_rebirth]= vide;
        spec[sq_rebirth]= 0;
      }
      if (flagAssassin && pdisp[nbply]) {
        if (e[sq_rebirth])
          nbpiece[e[sq_rebirth]]--;
        nbpiece[e[sq_rebirth]= pdisp[nbply]]++;
        spec[sq_rebirth]= pdispspec[nbply];
      }
      if (anytraitor)
        spec[sq_rebirth]= pdispspec[nbply];
    }

    if ((sq_rebirth= crenkam[nbply]) != initsquare) {
      /* Kamikaze and AntiCirce */
      crenkam[nbply]= initsquare;
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

  if (rochade)
  {
     square sq_castle= (sq_departure + sq_arrival) / 2;
     e[rochade_sq[nbcou]] = e[sq_castle];
     spec[rochade_sq[nbcou]] = spec[sq_castle];
     e[sq_castle] = vide;
     if (rn == sq_castle)
       rn= rochade_sq[nbcou];
     if (rb == sq_castle)
       rb= rochade_sq[nbcou];
     CLEARFL(spec[sq_castle]);
  }

  if (PatienceB) {
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

  rb= RB_[nbply];
  rn= RN_[nbply];

  if (abs(pi_departing) == andergb) {
    square sq= sq_arrival - direction(sq_departure, sq_arrival);

    if (!TSTFLAG(spec[sq], Neutral) && (sq != rb) && (sq != rn)) {
      change(sq);
      CHANGECOLOR(spec[sq]);
    }
  } /* andergb */

  /* at last modify promotion-counters and decrement nbcou */
  /* ortho- und pwc-Umwandlung getrennt */
  if (CondFlag[republican])
  {
    next_prom = super[nbply]>haut;
    if (next_prom)
      super[nbply] = superbas;
  }

  if (next_prom) {
    if ((pi_arriving= norm_prom[nbply]) != vide) {
      if (CondFlag[singlebox]
          && (SingleBoxType==singlebox_type1
              || SingleBoxType==singlebox_type2))
      {
        pi_arriving = next_singlebox_prom(pi_arriving,trait[nbply]);
      }
      else {
        pi_arriving= getprompiece[pi_arriving];

        if (pi_captured != vide && anyanticirce) {
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
              break;
            }
            pi_arriving= getprompiece[pi_arriving];
          }
        }
      }
      norm_prom[nbply]= pi_arriving;

      if ((pi_arriving == vide)
          && TSTFLAG(PieSpExFlags, Chameleon)
          && !norm_cham_prom[nbply])
      {
        pi_arriving= getprompiece[vide];
        if (pi_captured != vide && anyanticirce)
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
      if ((pi_arriving == vide) && !CondFlag[noiprom])
        Iprom[nbply]= true;
    }
    else if (!CondFlag[noiprom] && Iprom[nbply]) {
      ply icount;
      for (icount= nbply; icount <= maxply; inum[icount++]--);
      Iprom[nbply]= false;
    }

    if (pi_arriving == vide) {
      norm_cham_prom[nbply]= false;
      if (anycirprom
          && ((pi_arriving= cir_prom[nbply]) != vide)) {
        pi_arriving= cir_prom[nbply]= getprompiece[pi_arriving];
        if (pi_arriving == vide
            && TSTFLAG(PieSpExFlags, Chameleon)
            && !cir_cham_prom[nbply])
        {
          cir_prom[nbply]= pi_arriving= getprompiece[vide];
          cir_cham_prom[nbply]= true;
        }
      }
      if (pi_arriving == vide
          && !(!CondFlag[noiprom] && Iprom[nbply]))
      {
        if ((CondFlag[supercirce] && pi_captured != vide)
            || isapril[abs(pi_captured)]
            || (CondFlag[antisuper] && pi_captured != vide))
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

/* Generate (piece by piece) candidate moves to check if camp is
 * immobile. Do *not* generate moves by the camp's king; it has
 * already been taken care of. */
boolean immobile_encore(Side camp, square** immobilesquare) {
  square i;
  piece p;

  if (encore())
    return true;

  if (TSTFLAG(PieSpExFlags,Neutral))
    initneutre(advers(camp));

  while ((i= *(*immobilesquare)++)) {
    if ((p= e[i]) != vide) {
      if (TSTFLAG(spec[i], Neutral))
        p= -p;
      if (camp == White) {
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
} /* immobileencore */

/* Is camp immobile? */
boolean immobile(Side camp)
{
  square *immobilesquare= boardnum;  /* local to allow recursion */
  boolean const whbl_exact= camp==White ? wh_exact : bl_exact;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d\n",camp);

  if (!whbl_exact && !flag_testlegality)
  {
    nextply();
    current_killer_state= null_killer_state;
    trait[nbply]= camp;
    if (TSTFLAG(PieSpExFlags,Neutral))
      initneutre(advers(camp));
    if (camp == White)
    {
      if (rb != initsquare)
        gen_wh_piece(rb, abs(e[rb]));
    }
    else
    {
      if (rn != initsquare)
        gen_bl_piece(rn, -abs(e[rn]));
    }

    if (CondFlag[MAFF] || CondFlag[OWU])
    {
      int k_fl= 0, w_unit= 0;
      while (encore()) {
        if (jouecoup()) {
          if (camp==Black ? pprise[nbply]>=roib : pprise[nbply]<=roib)
            w_unit++;        /* assuming OWU is OBU for checks to wK !! */
          if (!echecc(camp))
            k_fl++;
        }
        repcoup();
      }
      if ((CondFlag[OWU] && (k_fl!=0 || w_unit!=1))
          || (CondFlag[MAFF] && (k_fl!=1)) )
      {
        finply();
        TraceFunctionExit(__func__);
        TraceFunctionResult("%d\n",false);
        return false;
      }
    }

    while (immobile_encore(camp,&immobilesquare))
    {
      TraceCurrentMove();
      if (jouecoup())
        if (!echecc(camp))
        {
          repcoup();
          finply();
          TraceFunctionExit(__func__);
          TraceFunctionResult("%d\n",false);
          return false;
        }

      repcoup();
    }
    finply();
  }
  else
  {
    Side ad= advers(camp);

    /* exact-maxis, ohneschach */
    move_generation_mode= move_generation_optimized_by_killer_move;
    if (!CondFlag[ohneschach]) {
      genmove(camp);
      while (encore())
      {
        TraceCurrentMove();
        if (jouecoup())
        {
          if (!echecc(camp))
          {
            repcoup();
            finply();
            TraceFunctionExit(__func__);
            TraceFunctionResult("%d\n",false);
            return false;
          }
        }

        repcoup();
      }

      finply();
    } else {
      genmove(camp);
      while (encore())
      {
        CondFlag[ohneschach]= false;
        TraceCurrentMove();
        jouecoup();
        CondFlag[ohneschach]= true;
        if (!echecc(ad) && pos_legal())
        {
          repcoup();
          finply();
          TraceFunctionExit(__func__);
          TraceFunctionResult("%d\n",false);
          return false;
        }
        repcoup();
      }
      finply();
      move_generation_mode= move_generation_optimized_by_killer_move;
      genmove(camp);
      while (encore())
      {
        CondFlag[ohneschach]= false;
        TraceCurrentMove();
        jouecoup();
        CondFlag[ohneschach]= true;
        if (echecc(ad) && pos_legal())
        {
          repcoup();
          finply();
          TraceFunctionExit(__func__);
          TraceFunctionResult("%d\n",false);
          return false;
        }
        repcoup();
      }
      finply();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",true);
  return true;
} /* immobile */

void find_mate_square(Side camp)
{
  square sq;

  if (camp == White) {
    while ((sq= ++super[nbply]) <= haut) {
      if (e[sq] == vide) {
        rn= sq;
        e[rn]= roin;
        nbpiece[roin]++;
        if (leaf_is_goal_reached(camp,1))
          return;
        nbpiece[roin]--;
        e[rn]= vide;
      }
    }
    rn= initsquare;
  } else {
    while ((sq= ++super[nbply]) <= haut) {
      if (e[sq] == vide) {
        rb= sq;
        e[rb]= roib;
        nbpiece[roib]++;
        if (leaf_is_goal_reached(camp,1))
          return;
        nbpiece[roib]--;
        e[rb]= vide;
      }
    }
    rb= initsquare;
  }
}
