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
 **
 ** 2008/03/13 SE   New condition: Castling Chess (invented: N.A.Bakke?)
 **
 ** 2009/01/03 SE   New condition: Disparate Chess (invented: R.Bedoni)
 **
 **************************** End of List ******************************/

#if defined(macintosh) /* is always defined on macintosh's  SB */
# define SEGM2
# include "platform/unix/mac.h"
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>  /* H.D. 10.02.93 prototype fuer exit */
#include <string.h>

#if defined(DOS)
# if defined(__GNUC__)
#  include <pc.h>
# else
#  include <bios.h>
# endif /* __GNUC__ */
#endif /* DOS */

#include "py.h"
#include "stipulation/stipulation.h"
#include "pyproc.h"
#include "pydata.h"
#include "pymsg.h"
#include "stipulation/has_solution_type.h"
#include "solving/battle_play/attack_play.h"
#include "stipulation/goals/doublemate/reached_tester.h"
#include "stipulation/temporary_hacks.h"
#include "solving/single_piece_move_generator.h"
#include "solving/castling_intermediate_move_generator.h"
#include "conditions/ohneschach/immobility_tester.h"
#include "conditions/disparate.h"
#include "conditions/eiffel.h"
#include "conditions/exclusive.h"
#include "conditions/extinction.h"
#include "conditions/madrasi.h"
#include "conditions/republican.h"
#include "conditions/patience.h"
#include "conditions/sat.h"
#include "conditions/oscillating_kings.h"
#include "conditions/circe/cage.h"
#include "pieces/attributes/paralysing/paralysing.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "optimisations/hash.h"
#include "debugging/trace.h"
#include "debugging/measure.h"

#define setneutre(i)            do {if (neutral_side != get_side(i)) change_side(i);} while(0)

static piece linechampiece(piece p, square sq)
{
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
  if (CondFlag[leofamily])
  {
    switch (p)
    {
      case leob:
        return maob;
      case leon:
        return maon;
      case maob:
        return vaob;
      case maon:
        return vaon;
      case vaob:
        return paob;
      case vaon:
        return paon;
      case paob:
        return leob;
      case paon:
        return leon;
      default:
        break;
    }
  }
  else if (CondFlag[cavaliermajeur])
  {
    switch (p)
    {
      case db:
        return nb;
      case dn:
        return nn;
      case nb:
        return fb;
      case nn:
        return fn;
      case fb:
        return tb;
      case fn:
        return tn;
      case tb:
        return db;
      case tn:
        return dn;
      default:
        break;
    }
  }
  else
  {
    switch (p)
    {
      case db:
        return cb;
      case dn:
        return cn;
      case cb:
        return fb;
      case cn:
        return fn;
      case fb:
        return tb;
      case fn:
        return tn;
      case tb:
        return db;
      case tn:
        return dn;
      default:
        break;
    }
  }

  return p;
}

piece norskpiece(piece p)
{
  /* function realisiert Figurenwechsel bei NorskSjakk */
  if (CondFlag[leofamily]) {
    switch (p)
    {
      case leob:
        return maob;
      case leon:
        return maon;
      case maob:
        return leob;
      case maon:
        return leon;
      case vaob:
        return paob;
      case vaon:
        return paon;
      case paob:
        return vaob;
      case paon:
        return vaon;
      default:
        break;
    }
  }
  else if (CondFlag[cavaliermajeur])
  {
    switch (p)
    {
      case db:
        return nb;
      case dn:
        return nn;
      case nb:
        return db;
      case nn:
        return dn;
      case fb:
        return tb;
      case fn:
        return tn;
      case tb:
        return fb;
      case tn:
        return fn;
      default:
        break;
    }
  }
  else
  {
    switch (p)
    {
      case db:
        return cb;
      case dn:
        return cn;
      case cb:
        return db;
      case cn:
        return dn;
      case fb:
        return tb;
      case fn:
        return tn;
      case tb:
        return fb;
      case tn:
        return fn;
      default:
        break;
    }
  }

  return p;
} /* norskpiece */

piece dec_einstein(piece p)
{
  /* function realisiert Figurenwechsel bei Einsteinschach abwaerts */
  switch (p)
  {
  case db:
    return tb;
    case dn:
      return tn;
    case tb:
      return fb;
    case tn:
      return fn;
    case fb:
      return cb;
    case fn:
      return cn;
    case cb:
      return pb;
    case cn:
      return pn;
    default:
      break;
  }

  return p;
}

piece inc_einstein(piece p)
{
  /* function realisiert Figurenwechsel bei Einsteinschach aufwaerts */
  switch (p) {
    case pb:
      return cb;
    case pn:
      return cn;
    case cb:
      return fb;
    case cn:
      return fn;
    case fb:
      return tb;
    case fn:
      return tn;
    case tb:
      return db;
    case tn:
      return dn;
    default:
      break;
  }

  return p;
}

#if defined(DOS)
# pragma warn -par
#endif

square renplus(ply ply_id,
               piece p_captured, Flags p_captured_spec,
               square sq_capture, square sq_departure, square sq_arrival,
               Side capturer) {
  /* Echecs plus */
  if (sq_capture==square_d4
      || sq_capture==square_e4
      || sq_capture==square_d5
      || sq_capture==square_e5)
  {
    square k;
    switch (mars_circe_rebirth_state)
    {
      case 0:
        k= square_d4;
        break;

      case 1:
        k= square_e4;
        break;

      case 2:
        k= square_d5;
        break;

      case 3:
        k= square_e5;
        break;

      default:
        k= initsquare;
        break;
    }

    mars_circe_rebirth_state = (mars_circe_rebirth_state+1)%4;

    return k;
  }
  else
    return sq_capture;
}

square renrank(ply ply_id,
               piece p_captured, Flags p_captured_spec,
               square sq_capture, square sq_departure, square sq_arrival,
               Side capturer) {
  square sq= ((sq_capture/onerow)%2==1
              ? rennormal(ply_id,p_captured,p_captured_spec,
                          sq_capture,sq_departure,sq_arrival,capturer)
              : renspiegel(ply_id,p_captured,p_captured_spec,
                           sq_capture,sq_departure,sq_arrival,capturer));
  return onerow*(sq_capture/onerow) + sq%onerow;
}

square renfile(ply ply_id,
               piece p_captured, Flags p_captured_spec,
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

square renspiegelfile(ply ply_id,
                      piece p_captured, Flags p_captured_spec,
                      square sq_capture,
                      square sq_departure, square sq_arrival,
                      Side capturer)
{
  return renfile(ply_id,p_captured,p_captured_spec,
                 sq_capture,sq_departure,sq_arrival,advers(capturer));
} /* renspiegelfile */

square renpwc(ply ply_id,
              piece p_captured, Flags p_captured_spec,
              square sq_capture, square sq_departure, square sq_arrival,
              Side capturer)
{
  return sq_departure;
} /* renpwc */

square renequipollents(ply ply_id,
                       piece p_captured, Flags p_captured_spec,
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

square renequipollents_anti(ply ply_id,
                            piece p_captured, Flags p_captured_spec,
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

square rensymmetrie(ply ply_id,
                    piece p_captured, Flags p_captured_spec,
                    square sq_capture,
                    square sq_departure, square sq_arrival,
                    Side capturer)
{
  return (square_h8+square_a1) - sq_capture;
} /* rensymmetrie */

square renantipoden(ply ply_id,
                    piece p_captured, Flags p_captured_spec,
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

square rendiagramm(ply ply_id,
                   piece p_captured, Flags p_captured_spec,
                   square sq_capture, square sq_departure, square sq_arrival,
                   Side capturer)
{
  return GetPositionInDiagram(p_captured_spec);
}

square rennormal(ply ply_id,
                 piece p_captured, Flags p_captured_spec,
                 square sq_capture, square sq_departure, square sq_arrival,
                 Side capturer)
{
  square  Result;
  int col, ran;
  Side  cou;
  PieNam pnam_captured = abs(p_captured);

  col = sq_capture % onerow;
  ran = sq_capture / onerow;

  if (CondFlag[circemalefiquevertical]) {
    col= onerow-1 - col;
    if (pnam_captured==Queen)
      pnam_captured = King;
    else if (pnam_captured==King)
      pnam_captured = Queen;
  }

  if ((ran&1) != (col&1))
    cou = White;
  else
    cou = Black;

  if (CondFlag[cavaliermajeur])
    if (pnam_captured==NightRider)
      pnam_captured = Knight;

  /* Below is the reason for the define problems. What a "hack" ! */
  if (CondFlag[leofamily]
      && pnam_captured>=Leo && Vao>=pnam_captured)
    pnam_captured -= 4;

  if (capturer == Black)
  {
    if (is_pawn(pnam_captured))
      Result= col + (nr_of_slack_rows_below_board+1)*onerow;
    else {
      if (CondFlag[frischauf] && TSTFLAG(p_captured_spec,FrischAuf)) {
        Result= (col
                 + (onerow
                    *(CondFlag[glasgow]
                      ? nr_of_slack_rows_below_board+nr_rows_on_board-2
                      : nr_of_slack_rows_below_board+nr_rows_on_board-1)));
      }
      else
        switch(pnam_captured) {
        case King:
          Result= square_e1;
          break;
        case Knight:
          Result= cou == White ? square_b1 : square_g1;
          break;
        case Rook:
          Result= cou == White ? square_h1 : square_a1;
          break;
        case Queen:
          Result= square_d1;
          break;
        case Bishop:
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
    if (is_pawn(pnam_captured))
      Result= col + (nr_of_slack_rows_below_board+nr_rows_on_board-2)*onerow;
    else {
      if (CondFlag[frischauf] && TSTFLAG(p_captured_spec,FrischAuf)) {
        Result= (col
                 + (onerow
                    *(CondFlag[glasgow]
                      ? nr_of_slack_rows_below_board+1
                      : nr_of_slack_rows_below_board)));
      }
      else
        switch(pnam_captured) {
        case Bishop:
          Result= cou == White ? square_c8 : square_f8;
          break;
        case Queen:
          Result= square_d8;
          break;
        case Rook:
          Result= cou == White ? square_a8 : square_h8;
          break;
        case Knight:
          Result= cou == White ? square_g8 : square_b8;
          break;
        case King:
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

square rendiametral(ply ply_id,
                    piece p_captured, Flags p_captured_spec,
                    square sq_capture,
                    square sq_departure, square sq_arrival,
                    Side capturer) {
  return (square_h8+square_a1
          - rennormal(ply_id,p_captured,p_captured_spec,
                      sq_capture,sq_departure,sq_arrival,capturer));
}

square renspiegel(ply ply_id,
                  piece p_captured, Flags p_captured_spec,
                  square sq_capture,
                  square sq_departure, square sq_arrival,
                  Side capturer)
{
  return rennormal(ply_id,p_captured,p_captured_spec,
                   sq_capture,sq_departure,sq_arrival,advers(capturer));
}

square rensuper(ply ply_id,
                piece p_captured, Flags p_captured_spec,
                square sq_capture,
                square sq_departure, square sq_arrival,
                Side capturer)
{
  return current_super_circe_rebirth_square[ply_id];
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
    return  true;
  default:
    return  false;
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
    return  true;
  default:
    return  false;
  }
}

boolean is_forwardpawn(piece p)
{
  switch (abs(p)) {
  case  Pawn:
  case  BerolinaPawn:
  case  SuperBerolinaPawn:
  case  SuperPawn:
    return  true;
  default:
    return  false;
  }
}

boolean is_reversepawn(piece p)
{
  switch (abs(p)) {
  case  ReversePawn:
    return  true;
  default:
    return  false;
  }
}

/* Determine whether a sequence of squares are empty
 * @param from start of sequence
 * @param to end of sequence
 * @param direction delta to (repeatedly) apply to reach to from from
 * @return true if the squares between (and not including) from and to are empty
 */
static boolean are_squares_empty(square from, square to, int direction)
{
  square s;
  for (s = from+direction; s!=to; s += direction)
    if (e[s]!=vide)
      return false;

  return true;
}

boolean castling_is_intermediate_king_move_legal(Side side, square from, square to)
{
  boolean result = false;

  if (complex_castling_through_flag)
  {
    /* temporarily deactivate maximummer etc. */
    boolean const save_flagmummer = flagmummer[side];
    flagmummer[side] = false;
    castling_intermediate_move_generator_init_next(from,to);
    result = attack(slices[temporary_hack_castling_intermediate_move_legality_tester[side]].next2,length_unspecified)==has_solution;
    flagmummer[side] = save_flagmummer;
  }
  else
  {
    piece const sides_king = side==White ? roib : roin;
    e[from]= vide;
    e[to]= sides_king;
    if (king_square[side]!=initsquare)
      king_square[side] = to;

    result = !echecc(side);

    e[from]= sides_king;
    e[to]= vide;
    if (king_square[side]!=initsquare)
      king_square[side] = from;
  }

  return result;
}

void generate_castling(Side side)
{
  /* It works only for castling_supported == TRUE
     have a look at funtion verifieposition() in py6.c
  */

  square const square_a = side==White ? square_a1 : square_a8;
  square const square_e = square_a+file_e;
  piece const sides_king = side==White ? roib : roin;

  if (dont_generate_castling)
    return;

  if (TSTCASTLINGFLAGMASK(nbply,side,castlings)>k_cancastle
      && e[square_e]==sides_king
      /* then the king on e8 and at least one rook can castle !! */
      && !echecc(side))
  {
    square const square_c = square_a+file_c;
    square const square_d = square_a+file_d;
    square const square_f = square_a+file_f;
    square const square_g = square_a+file_g;
    square const square_h = square_a+file_h;
    piece const sides_rook = side==White ? tb : tn;

    /* 0-0 */
    if (TSTCASTLINGFLAGMASK(nbply,side,k_castling)==k_castling
        && e[square_h]==sides_rook
        && are_squares_empty(square_e,square_h,dir_right)
        && castling_is_intermediate_king_move_legal(side,square_e,square_f))
      empile(square_e,square_g,kingside_castling);

    /* 0-0-0 */
    if (TSTCASTLINGFLAGMASK(nbply,side,q_castling)==q_castling
        && e[square_a]==sides_rook
        && are_squares_empty(square_e,square_a,dir_left)
        && castling_is_intermediate_king_move_legal(side,square_e,square_d))
      empile(square_e,square_c,queenside_castling);
  }
}

void genrn(square sq_departure)
{
  Side const side = Black;
  boolean flag = false;  /* K im Schach ? */
  numecoup const save_nbcou = current_move[nbply];

  if (calc_refl_king[side] && !calctransmute)
  {
    /* K im Schach zieht auch */
    calctransmute = true;
    if (!normaltranspieces[side] && echecc(side))
    {
      piece *ptrans;
      for (ptrans = transmpieces[side]; *ptrans!=vide; ++ptrans)
      {
        flag = true;
        current_trans_gen = -*ptrans;
        gen_bl_piece(sq_departure,-*ptrans);
        current_trans_gen = vide;
      }
    }
    else if (normaltranspieces[side])
    {
      piece const *ptrans;
      for (ptrans = transmpieces[side]; *ptrans!=vide; ++ptrans)
        if (nbpiece[*ptrans]>0
            && (*checkfunctions[*ptrans])(sq_departure,*ptrans,eval_black))
        {
          flag = true;
          current_trans_gen = -*ptrans;
          gen_bl_piece(sq_departure,-*ptrans);
          current_trans_gen = vide;
        }
    }
    calctransmute = false;

    if (flag && nbpiece[orphanb]>0)
    {
      piece const king = e[king_square[side]];
      e[king_square[side]] = dummyn;
      if (!echecc(side))
        /* side's king checked only by an orphan empowered by the king */
        flag = false;
      e[king_square[side]] = king;
    }

    /* K im Schach zieht nur */
    if (calc_trans_king[side] && flag)
      return;
  }

  if (CondFlag[sting])
    gerhop(sq_departure,vec_queen_start,vec_queen_end,side);

  {
    numvec k;
    for (k = vec_queen_end; k>=vec_queen_start; --k)
    {
      square const sq_arrival = sq_departure+vec[k];
      if (e[sq_arrival]==vide || e[sq_arrival]>=roib)
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }

  if (flag)
    remove_duplicate_moves(save_nbcou);

  /* Now we test castling */
  if (castling_supported)
    generate_castling(side);

  if (CondFlag[castlingchess] && !echecc(side))
  {
    numvec k;
    for (k = vec_queen_end; k>= vec_queen_start; --k)
    {
      square const sq_passed = sq_departure+vec[k];
      square const sq_arrival = sq_passed+vec[k];
      square sq_castler;
      piece p;

      finligne(sq_departure,vec[k],p,sq_castler);
      if (sq_castler!=sq_passed && sq_castler!=sq_arrival && abs(p)>=roib
          && castling_is_intermediate_king_move_legal(side,sq_departure,sq_passed))
        empile(sq_departure,sq_arrival,maxsquare+sq_castler);
    }
  }

  if (CondFlag[platzwechselrochade] && blpwr[nbply])
  {
    int i,j;
    piece p;
    square z= square_a1;
    for (i= nr_rows_on_board; i > 0; i--, z+= onerow-nr_files_on_board)
    for (j= nr_files_on_board; j > 0; j--, z++) {
      if ((p = e[z]) != vide) {
      if (TSTFLAG(spec[z], Neutral))
        p = -p;
      if (p < vide && !is_pawn(p))  /* not sure if "castling" with Ps forbidden */
        empile(sq_departure,z,platzwechsel_rochade);
      }
    }
  }
}

void gen_bl_ply(void)
{
  square i, j, z;
  piece p;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  /* Don't try to "optimize" by hand. The double-loop is tested as the  */
  /* fastest way to compute (due to compiler-optimizations !) */
  z= square_h8;
  for (i= nr_rows_on_board; i > 0; i--, z-= onerow-nr_files_on_board)
    for (j= nr_files_on_board; j > 0; j--, z--) {
      if ((p = e[z]) != vide) {
        if (TSTFLAG(spec[z], Neutral))
          p = -p;
        if (p < vide)
          gen_bl_piece(z, p);
      }
    }
  if (CondFlag[schwarzschacher])
    empile(nullsquare, nullsquare, nullsquare);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
} /* gen_bl_ply */

void gen_bl_piece_aux(square z, piece p)
{
  TraceFunctionEntry(__func__);
  TraceSquare(z);
  TracePiece(p);
  TraceFunctionParamListEnd();

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

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void orig_gen_bl_piece(square sq_departure, piece p)
{
  piece pi_departing;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TracePiece(p);
  TraceFunctionParamListEnd();

  if (CondFlag[madras] && !madrasi_can_piece_move(sq_departure))
  {
    TraceFunctionExit(__func__);
    TraceFunctionResultEnd();
    return;
  }
  else if (CondFlag[eiffel] && !eiffel_can_piece_move(sq_departure))
  {
    TraceFunctionExit(__func__);
    TraceFunctionResultEnd();
    return;
  }
  else if (CondFlag[disparate] && !disparate_can_piece_move(sq_departure))
  {
    TraceFunctionExit(__func__);
    TraceFunctionResultEnd();
    return;
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
      numecoup const anf1 = current_move[nbply];
      /* generate standard moves first */
      flagactive= false;
      flagpassive= false;
      flagcapture= false;

      gen_bl_piece_aux(sq_departure,p);

      /* Kings normally don't move from their rebirth-square */
      if (p == e[king_square[Black]] && !rex_phan) {
        return;
      }
      /* generate moves from rebirth square */
      flagactive= true;
      spec_departing=spec[sq_departure];
      sq_rebirth= (*marsrenai)(nbply,
                               p,
                               spec_departing,
                               sq_departure,
                               initsquare,
                               initsquare,
                               White);
      /* if rebirth square is where the piece stands,
         we've already generated all the relevant moves.
      */
      if (sq_rebirth==sq_departure)
        return;
      if (e[sq_rebirth] == vide)
      {
        numecoup const anf2 = current_move[nbply];
        numecoup l1;
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
        for (l1 = anf1+1; l1<=anf2; l1++)
        {
          numecoup l2 = anf2+1;
          while (l2 <= current_move[nbply])
            if (move_generation_stack[l1].arrival
                ==move_generation_stack[l2].arrival)
            {
              move_generation_stack[l2] = move_generation_stack[current_move[nbply]];
              --current_move[nbply];
              break;  /* remember: ONE duplicate ! */
            }
            else
              l2++;
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
      mars_circe_rebirth_state = 0;
      do {   /* Echecs Plus */
        spec_departing= spec[sq_departure];
        sq_rebirth= (*marsrenai)(nbply,
                                 p,
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
      } while (mars_circe_rebirth_state);
      flagcapture= false;
    }
  }
  else
    gen_bl_piece_aux(sq_departure,p);

  if (CondFlag[messigny] && !(king_square[Black]==sq_departure && rex_mess_ex))
  {
    square const *bnp;
    for (bnp= boardnum; *bnp; bnp++)
      if (e[*bnp]==-p)
        empile(sq_departure,*bnp,messigny_exchange);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
} /* orig_gen_bl_piece */

void singleboxtype3_gen_bl_piece(square z, piece p) {
  numecoup save_nbcou = current_move[nbply];
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
      numecoup prev_nbcou = current_move[nbply];
      ++latent_prom;
      e[sq] = -pprom;
      orig_gen_bl_piece(z, sq==z ? -pprom : p);
      e[sq] = pn;

      for (++prev_nbcou; prev_nbcou<=current_move[nbply]; ++prev_nbcou)
      {
        sb3[prev_nbcou].where = sq;
        sb3[prev_nbcou].what = -pprom;
      }
    }
  }

  if (latent_prom==0)
  {
    orig_gen_bl_piece(z,p);
    for (++save_nbcou; save_nbcou<=current_move[nbply]; ++save_nbcou)
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

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,camp,"");
  TraceFunctionParamListEnd();

  if (nbply==1 && flag_magic)
    PushMagicViews();
  nextply(nbply);
  trait[nbply]= camp;
  we_generate_exact = false;
  init_move_generation_optimizer();

  if (CondFlag[exclusive])
    exclusive_init_genmove(camp);

  /* exact and consequent maximummers */
  if (camp == White)
  {
    /* let's first generate consequent moves */
    if (wh_exact)
    {
      we_generate_exact = true;
      there_are_consmoves = false;
      gen_wh_ply();
      if (CondFlag[whforsqu] && CondFlag[whconforsqu]
          && !there_are_consmoves) {
        /* There are no consequent moves.
        ** Now let's look for ``normal'' forced moves,
        ** but first reset current_move[nbply] etc.
        */
        we_generate_exact = false;
        finply();
        nextply(nbply);
        init_move_generation_optimizer();
        gen_wh_ply();
      }
      we_generate_exact = false;
    }
    else
      gen_wh_ply();
  }
  else
  {
    /* let's first generate consequent moves */
    if (bl_exact)
    {
      we_generate_exact = true;
      there_are_consmoves = false;
      gen_bl_ply();
      if (CondFlag[blforsqu] && CondFlag[blconforsqu]
          && !there_are_consmoves) {
        /* There are no consequent moves.
        ** Now let's look for ``normal'' forced moves,
        ** but first reset current_move[nbply] etc.
        */
        we_generate_exact = false;
        finply();
        nextply(nbply);
        init_move_generation_optimizer();
        gen_bl_ply();
      }
      we_generate_exact = false;
    }
    else
      gen_bl_ply();
  }
  finish_move_generation_optimizer();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
} /* genmove(camp) */

piece pdisp[maxply+1];
Flags pdispspec[maxply+1];

static int direction(square from, square to)
{
  int dir= to-from;
  int hori= to%onerow-from%onerow;
  int vert= to/onerow-from/onerow;
  int i=7;
  while ((hori%i) || (vert%i))
    i--;

  return dir/i;
}

static square blpc;

static boolean eval_spec(square sq_departure,
                         square sq_arrival,
                         square sq_capture)
{
  return sq_departure==blpc;
}

static boolean att_once(square sq_departure, Side trait_ply)
{
  int i,j, count=0;
  square square_a = square_a1;

  if (trait_ply == White) {

    square const rb_=king_square[White];
    king_square[White] = sq_departure;

    for (i = nr_rows_on_board; i>0; --i, square_a += dir_up)
    {
      square z = square_a;
      for (j= nr_files_on_board; j>0; --j, z += dir_right)
        if (e[z]<-obs)
        {
          blpc = z;
          if (rbechec(eval_spec))
          {
            ++count;
            if (count > 1)
              return false;
          }
        }
      }

      king_square[White] = rb_;

  } else {

    square const rn_=king_square[Black];
    king_square[Black] = sq_departure;

    for (i = nr_rows_on_board; i>0; --i, square_a += dir_up)
    {
      square z = square_a;
      for (j= nr_files_on_board; j>0; --j, z += dir_right)
        if (e[z]<-obs)
        {
          blpc = z;
          if (rnechec(eval_spec))
          {
            ++count;
            if (count > 1)
              return false;
          }
        }
      }

      king_square[Black] = rn_;
  }
  return count==1;
}

square next_latent_pawn(square s, Side c) {
  piece pawn;
  int  i, delta;

  pawn=  c==White ? pb : pn;
  delta= c==White ?+dir_left :+dir_right;

  if (s==initsquare) {
    i = 0;
    s = c==White ? square_h8 : square_a1;
  }
  else {
    i = c==White ? square_h8-s+1 : s-square_a1+1;
    s += delta;
  }

  for (; i<8; ++i, s+=delta) {
    if (e[s]==pawn) {
      return s;
    }
  }

  return initsquare;
}

piece next_singlebox_prom(piece p, Side c)
{
  piece pprom;
  piece result = vide;

  TraceFunctionEntry(__func__);
  TracePiece(p);
  TraceFunctionParam("%u",c);
  TraceFunctionParamListEnd();

  for (pprom = getprompiece[p];
       pprom!=vide;
       pprom = getprompiece[pprom])
    if (pprom!=pb
        && nbpiece[c==White ? pprom : -pprom] < nr_piece(game_array)[pprom])
    {
      result = pprom;
      break;
    }

  TraceFunctionExit(__func__);
  TracePiece(p);
  TraceFunctionResultEnd();
  return result;
}

#if defined(DEBUG)
static  int nbrtimes = 0;
#endif

static ghost_index_type find_ghost(square sq_arrival)
{
  ghost_index_type current = nr_ghosts;
  ghost_index_type result = ghost_not_found;
  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();

  while (current>0)
  {
    --current;
    if (ghosts[current].ghost_square==sq_arrival)
    {
      result = current;
      break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",result);
  TraceFunctionResultEnd();
  return result;
}

static void remember_ghost(square sq_arrival)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();

  assert(nr_ghosts<ghost_capacity);
  ghosts[nr_ghosts].ghost_square = sq_arrival;
  ghosts[nr_ghosts].ghost_piece = e[sq_arrival];
  ghosts[nr_ghosts].ghost_flags = spec[sq_arrival];
  ghosts[nr_ghosts].hidden = false;
  ++nr_ghosts;
  TraceValue("->%u\n",nr_ghosts);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void preempt_ghost(square sq_arrival)
{
  ghost_index_type const ghost_pos = find_ghost(sq_arrival);

  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",ghost_pos);
  if (ghost_pos!=ghost_not_found)
    ghosts[ghost_pos].hidden = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void unpreempt_ghost(square sq_arrival)
{
  ghost_index_type const ghost_pos = find_ghost(sq_arrival);

  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",ghost_pos);
  if (ghost_pos!=ghost_not_found)
    ghosts[ghost_pos].hidden = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forget_ghost_at_pos(ghost_index_type ghost_pos)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ghost_pos);
  TraceFunctionParamListEnd();

  assert(ghost_pos!=ghost_not_found);
  assert(nr_ghosts>0);
  --nr_ghosts;

  TraceValue("->%u ",nr_ghosts);
  TraceSquare(ghosts[ghost_pos].ghost_square);
  TracePiece(ghosts[ghost_pos].ghost_piece);
  TraceText("\n");
  memmove(ghosts+ghost_pos, ghosts+ghost_pos+1,
          (nr_ghosts-ghost_pos) * sizeof ghosts[0]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forget_ghost(square sq_arrival)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();

  forget_ghost_at_pos(find_ghost(sq_arrival));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void summon_ghost(square sq_departure)
{
  ghost_index_type const ghost_pos = find_ghost(sq_departure);
  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",ghost_pos);

  if (ghost_pos!=ghost_not_found && !ghosts[ghost_pos].hidden)
  {
    piece const piece_summoned = ghosts[ghost_pos].ghost_piece;
    Flags spec_summoned = ghosts[ghost_pos].ghost_flags;

    if (CondFlag[ghostchess])
      SETFLAG(spec_summoned,Uncapturable);

    e[sq_departure] = piece_summoned;
    spec[sq_departure] = spec_summoned;
    ++nbpiece[piece_summoned];

    if (TSTFLAG(spec_summoned,Neutral))
      setneutre(sq_departure);

    forget_ghost_at_pos(ghost_pos);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void ban_ghost(square sq_departure)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  CLRFLAG(spec[sq_departure],Uncapturable);
  remember_ghost(sq_departure);
  --nbpiece[e[sq_departure]];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean post_move_iterations_locked[maxply];

void lock_post_move_iterations(void)
{
  post_move_iterations_locked[nbply] = true;
}

void unlock_post_move_iterations(void)
{
  post_move_iterations_locked[nbply] = false;
}

boolean are_post_move_iterations_locked(void)
{
  return post_move_iterations_locked[nbply];
}

DEFINE_COUNTER(jouecoup)

void jouecoup(void)
{
  numecoup const coup_id = current_move[nbply];
  move_generation_elmt const * const move_gen_top = move_generation_stack+coup_id;

  square sq_rebirth = initsquare;

  piece pi_captured;
  Flags spec_pi_captured;

  boolean rochade = false;
  boolean platzwechsel = false;

  Side const trait_ply = trait[nbply];

  square const prev_rb = king_square[White];
  square const prev_rn = king_square[Black];

  square sq_capture = move_gen_top->capture;

  square const sq_arrival = move_gen_top->arrival;
  square const sq_departure = move_gen_top->departure;

  Flags spec_pi_moving = spec[sq_departure];
  piece pi_arriving = e[sq_departure];

  piece pi_departing = pi_arriving;

#if defined(DEBUG)
  nbrtimes++;
#endif

  INCREMENT_COUNTER(jouecoup);

  assert(sq_arrival!=nullsquare);

  RB_[nbply] = king_square[White];
  RN_[nbply] = king_square[Black];

  pjoue[nbply] = pi_arriving;
  jouespec[nbply] = spec_pi_moving;
  sqdep[nbply] = sq_departure;

  if (jouegenre)
  {
    rochade_sq[coup_id] = initsquare;
    if (sq_capture > platzwechsel_rochade)
    {
      rochade_sq[coup_id] = sq_capture-maxsquare;
      rochade_pc[coup_id] = e[rochade_sq[coup_id]];
      rochade_sp[coup_id] = spec[rochade_sq[coup_id]];
      sq_capture = sq_arrival;
      rochade = true;
    }
    else if (sq_capture == platzwechsel_rochade)
    {
      rochade_sq[coup_id] = sq_arrival;
      rochade_pc[coup_id] = e[rochade_sq[coup_id]];
      rochade_sp[coup_id] = spec[rochade_sq[coup_id]];
      rochade_sq[coup_id] = -sq_arrival; /* hack for output */
      sq_capture = sq_arrival;
      platzwechsel = true;
      if (trait_ply == White)
        whpwr[nbply]= false;
      else
        blpwr[nbply]=false;
    }

    if (CondFlag[amu])
      att_1[nbply]= att_once(sq_departure,trait_ply);

    if (CondFlag[imitators])
    {
      if (sq_capture==queenside_castling)
        joueim(+dir_right);
      else if (rochade)
        joueim((3*sq_arrival-sq_departure-rochade_sq[coup_id]) / 2);
      else if (sq_capture!=kingside_castling) /* joueim(0) (do nothing) if OO */
        joueim(sq_arrival-sq_departure);
    }
  }

  spec_pi_captured = pprispec[nbply] = spec[sq_capture];
  pi_captured = pprise[nbply] = e[sq_capture];

  if (anyantimars && sq_departure==sq_capture || move_gen_top->capture >= platzwechsel_rochade)
  {
    spec_pi_captured = 0;
    pprispec[nbply]= 0;
    pi_captured = vide;
    pprise[nbply] = vide;
  }

  pdisp[nbply] = vide;
  pdispspec[nbply] = 0;

  if (jouegenre)
  {
    if (CondFlag[blsupertrans_king]
        && trait_ply==Black
        && ctrans[coup_id]!=vide)
    {
      king_square[Black]=initsquare;
      pi_arriving=ctrans[coup_id];
    }
    if (CondFlag[whsupertrans_king]
        && trait_ply==White
        && ctrans[coup_id]!=vide)
    {
      king_square[White]=initsquare;
      pi_arriving=ctrans[coup_id];
    }

    if (CondFlag[singlebox] && SingleBoxType==singlebox_type3
        && sb3[coup_id].what!=vide) {
      --nbpiece[e[sb3[coup_id].where]];
      e[sb3[coup_id].where] = sb3[coup_id].what;
      ++nbpiece[sb3[coup_id].what];
      if (sq_departure==sb3[coup_id].where) {
        pi_departing = pi_arriving = sb3[coup_id].what;
      }
    }

    if (CondFlag[ghostchess] && pi_captured!=vide)
      remember_ghost(sq_arrival);
    if (CondFlag[hauntedchess] && pi_captured!=vide)
    {
      preempt_ghost(sq_arrival);
      remember_ghost(sq_arrival);
    }
  }

  switch (sq_capture)
  {
    case kingside_castling:
      if (CondFlag[einstein])
      {
        if (trait_ply==White)
        {
          nbpiece[tb]--;
          if (CondFlag[reveinstein])
          {
            e[square_f1]= db;
            nbpiece[db]++;
          }
          else
          {
            e[square_f1]= fb;
            nbpiece[fb]++;
          }
        }
        else
        {
          nbpiece[tn]--;
          if (CondFlag[reveinstein])
          {
            e[square_f8]= dn;
            nbpiece[dn]++;
          }
          else
          {
            e[square_f8]= fn;
            nbpiece[fn]++;
          }
        }
      }
      else
        e[sq_departure+dir_right]= e[sq_departure+3*dir_right];

      spec[sq_departure+dir_right]= spec[sq_departure+3*dir_right];
      e[sq_departure+3*dir_right]= CondFlag[haanerchess] ? obs : vide;
      CLEARFL(spec[sq_departure+3*dir_right]);
      CLRCASTLINGFLAGMASK(nbply,trait_ply,k_castling);
      CLRCASTLINGFLAGMASK(nbply,advers(trait_ply),
                          castling_mutual_exclusive[trait_ply][kingside_castling-min_castling]);
      break;

    case queenside_castling:
      if (CondFlag[einstein])
      {
        if (trait_ply==White)
        {
          nbpiece[tb]--;
          if (CondFlag[reveinstein])
          {
            e[square_d1]= db;
            nbpiece[db]++;
          }
          else
          {
            e[square_d1]= fb;
            nbpiece[fb]++;
          }
        }
        else
        {
          nbpiece[tn]--;
          if (CondFlag[reveinstein])
          {
            e[square_d8]= dn;
            nbpiece[dn]++;
          }
          else
          {
            e[square_d8]= fn;
            nbpiece[fn]++;
          }
        }
      }
      else
        e[sq_departure+dir_left]= e[sq_departure+4*dir_left];

      spec[sq_departure+dir_left]= spec[sq_departure+4*dir_left];
      e[sq_departure+4*dir_left]= CondFlag[haanerchess] ? obs : vide;
      CLEARFL(spec[sq_departure+4*dir_left]);
      CLRCASTLINGFLAGMASK(nbply,trait_ply,q_castling);
      CLRCASTLINGFLAGMASK(nbply,advers(trait_ply),
                          castling_mutual_exclusive[trait_ply][queenside_castling-min_castling]);
      break;
  } /* switch (sq_capture) */

  if (platzwechsel)
  {
    piece p=e[sq_arrival];
    Flags sp=spec[sq_arrival];
    e[sq_arrival]=e[sq_departure];
    spec[sq_arrival]=spec[sq_departure];
    if (king_square[Black] == sq_departure)
      king_square[Black]= sq_arrival;
    if (king_square[White] == sq_departure)
      king_square[White]= sq_arrival;
    e[sq_departure]=p;
    spec[sq_departure]=sp;
  }
  else if (rochade)
  {
    square sq_castle= (sq_departure + sq_arrival) / 2;
    e[sq_castle] = e[rochade_sq[coup_id]];
    spec[sq_castle] = spec[rochade_sq[coup_id]];
    e[rochade_sq[coup_id]] = CondFlag[haanerchess] ? obs : vide;
    CLEARFL(spec[rochade_sq[coup_id]]);
    if (king_square[Black] == rochade_sq[coup_id])
      king_square[Black]= sq_castle;
    if (king_square[White] == rochade_sq[coup_id])
      king_square[White]= sq_castle;
    e[sq_departure]= CondFlag[haanerchess] ? obs : vide;
    spec[sq_departure]= 0;
  }
  else
  {
    e[sq_departure]= CondFlag[haanerchess] ? obs : vide;
    spec[sq_departure]= 0;
  }

  if (PatienceB) {
    ply nply;
    e[sq_departure]= obs;
    for (nply= nbply - 1 ; nply > 1 ; nply--) {
      if (trait[nply] == trait_ply) {
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

    if (!TSTFLAG(spec[sq], Neutral) && (sq != king_square[White]) && (sq != king_square[Black])) {
      change_side(sq);
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
        square ii = anyantimars ? cmren[coup_id] : sq_departure;
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
  }

  prompieces[nbply] = GetPromotingPieces(sq_departure,
                                      pi_departing,
                                      trait[nbply],
                                      spec_pi_moving,
                                      sq_arrival,
                                      pi_captured);

  if (prompieces[nbply] != NULL)
    {
      pi_arriving = current_promotion_of_moving[nbply];
      if (pi_arriving==vide)
      {
        if (!CondFlag[noiprom] && Iprom[nbply])
        {
          ply icount;
          if (inum[nbply] == maxinum)
            FtlMsg(ManyImitators);
          for (icount = nbply; icount<=maxply; ++icount)
            ++inum[icount];
          isquare[inum[nbply]-1] = sq_arrival;
        }
        else
        {
          pi_arriving= (prompieces[nbply])[vide];

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
                                       advers(trait_ply));
              if (sq_rebirth == sq_departure)
                break;
              if (LegalAntiCirceMove(sq_rebirth, sq_capture, sq_departure))
                break;
              pi_arriving= (prompieces[ply_id])[pi_arriving];
            } while (1);
#endif /*BETTER_READABLE*/

            while (((sq_rebirth= (*antirenai)(nbply,
                                              pi_arriving,
                                              spec_pi_moving,
                                              sq_capture,
                                              sq_departure,
                                              sq_arrival,
                                              advers(trait_ply)))
                    != sq_departure)
                   && !LegalAntiCirceMove(sq_rebirth,
                                          sq_capture,
                                          sq_departure))
            {
              pi_arriving= (prompieces[nbply])[pi_arriving];
              if (!pi_arriving && CondFlag[antisuper])
              {
                current_super_circe_rebirth_square[nbply]++;
                pi_arriving= (prompieces[nbply])[vide];
              }
            }
          }

          current_promotion_of_moving[nbply]= pi_arriving;
        }
      }
      else
      {
        if (CondFlag[frischauf])
          SETFLAG(spec_pi_moving, FrischAuf);
      }

      if (!(!CondFlag[noiprom] && Iprom[nbply])) {
        if (TSTFLAG(spec_pi_moving, Chameleon)
            && is_pawn(pi_departing))
          is_moving_chameleon_promoted[nbply]= true;

        if (pi_departing<vide)
          pi_arriving = -pi_arriving;

        /* so also promoted neutral pawns have the correct color and
         * an additional call to setneutre is not required.
         */
        if (is_moving_chameleon_promoted[nbply])
          SETFLAG(spec_pi_moving, Chameleon);
      }
      else
        pi_arriving= vide; /* imitator-promotion */
    } /* promotion */

  if (sq_arrival!=sq_capture)
  {
    e[sq_capture] = vide;
    spec[sq_capture] = 0;
  }

  if (pi_captured!=vide)
  {
    nbpiece[pi_captured]--;

    if (sq_capture==king_square[White])
      king_square[White] = initsquare;
    if (sq_capture==king_square[Black])
      king_square[Black] = initsquare;
  }

  if (change_moving_piece)
  {
    if (CondFlag[degradierung]
        && !is_pawn(pi_departing)
        && sq_departure != prev_rn
        && sq_departure != prev_rb
        && (trait_ply == Black
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
         && (((trait_ply == Black)
              && (sq_departure != prev_rn))
             || ((trait_ply == White)
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
          && (!TSTFLAG(spec_pi_moving, White) || neutral_side == Black))
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
      if (trait_ply == Black) {
        SETFLAG(spec_pi_moving, White);
        pi_arriving= abs(pi_arriving);
      }
      else {
        SETFLAG(spec_pi_moving, Black);
        pi_arriving= -abs(pi_arriving);
      }
    } /* CondFlag[antiandernach] ... */

    if (CondFlag[champursue]
        && sq_arrival == move_generation_stack[current_move[nbply-1]].departure
        && sq_departure != prev_rn
        && sq_departure != prev_rb)
    {
      /* the following also copes correctly with neutral */
      CLRFLAG(spec_pi_moving, Black);
      CLRFLAG(spec_pi_moving, White);
      CLRFLAG(spec_pi_moving, Neutral);
      if (trait_ply == Black) {
        SETFLAG(spec_pi_moving, White);
        pi_arriving= abs(pi_arriving);
      }
      else {
        SETFLAG(spec_pi_moving, Black);
        pi_arriving= -abs(pi_arriving);
      }
    } /* CondFlag[antiandernach] ... */

    if ((CondFlag[traitor]
         && trait_ply == Black
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

  if (sq_departure==prev_rb)
  {
    if (king_square[White]!=initsquare)
      king_square[White] = sq_arrival;
    CLRCASTLINGFLAGMASK(nbply,White,k_cancastle);
  }
  if (sq_departure==prev_rn)
  {
    if (king_square[Black]!=initsquare)
      king_square[Black] = sq_arrival;
    CLRCASTLINGFLAGMASK(nbply,Black,k_cancastle);
  }

  /* Needed for castling */
  if (castling_supported)
  {
    /* pieces vacating a1, h1, a8, h8 */
    if (sq_departure == square_h1)
      CLRCASTLINGFLAGMASK(nbply,White,rh_cancastle);
    else if (sq_departure == square_a1)
      CLRCASTLINGFLAGMASK(nbply,White,ra_cancastle);
    else if (sq_departure == square_h8)
      CLRCASTLINGFLAGMASK(nbply,Black,rh_cancastle);
    else if (sq_departure == square_a8)
      CLRCASTLINGFLAGMASK(nbply,Black,ra_cancastle);

    /* pieces arriving at a1, h1, a8, h8 and possibly capturing a rook */
    if (sq_arrival == square_h1)
      CLRCASTLINGFLAGMASK(nbply,White,rh_cancastle);
    else if (sq_arrival == square_a1)
      CLRCASTLINGFLAGMASK(nbply,White,ra_cancastle);
    else if (sq_arrival == square_h8)
      CLRCASTLINGFLAGMASK(nbply,Black,rh_cancastle);
    else if (sq_arrival == square_a8)
      CLRCASTLINGFLAGMASK(nbply,Black,ra_cancastle);

    if (CondFlag[losingchess])
    {
      /* r[bn] (and therefore prev_r[bn]) are ==initsquare if kings
       * are not royal
       */
      if (sq_arrival==square_e1)
        CLRCASTLINGFLAGMASK(nbply,White,k_cancastle);
      else if (sq_arrival==square_e8)
        CLRCASTLINGFLAGMASK(nbply,Black,k_cancastle);
    }
  }     /* castling_supported */

  if ((CondFlag[andernach] && pi_captured!=vide)
      || (CondFlag[antiandernach] && pi_captured==vide)
      || (CondFlag[champursue] && sq_arrival == move_generation_stack[current_move[nbply-1]].departure)
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
            SETCASTLINGFLAGMASK(nbply,White,rh_cancastle);
          else if (sq_arrival == square_a1)
            SETCASTLINGFLAGMASK(nbply,White,ra_cancastle);
        }
        if (TSTFLAG(spec_pi_moving, Black)) {
          /* new black/neutral rook */
          if (sq_arrival == square_h8)
            SETCASTLINGFLAGMASK(nbply,Black,rh_cancastle);
          else if (sq_arrival == square_a8)
            SETCASTLINGFLAGMASK(nbply,Black,ra_cancastle);
        }
      }
    } /* castling_supported */
  } /* andernach || antiandernach ... */

  if (TSTFLAG(spec_pi_moving, HalfNeutral))
  {
    if (TSTFLAG(spec_pi_moving, Neutral))
    {
      CLRFLAG(spec_pi_moving,advers(trait_ply));
      CLRFLAG(spec_pi_moving, Neutral);
      pi_arriving= trait_ply==Black ? -abs(pi_arriving) : abs(pi_arriving);

      if (king_square[Black] == sq_arrival && trait_ply == White)
        king_square[Black]= initsquare;

      if (king_square[White] == sq_arrival && trait_ply == Black)
        king_square[White]= initsquare;
    }
    else if (trait_ply==Black) {
      if (TSTFLAG(spec_pi_moving, Black)) {
        SETFLAG(spec_pi_moving, Neutral);
        SETFLAG(spec_pi_moving, White);
        pi_arriving= abs(pi_arriving);
        if (king_square[Black] == sq_arrival)
          king_square[White] = sq_arrival;
      }
    }
    else if (trait_ply==White) {
      if (TSTFLAG(spec_pi_moving, White)) {
        SETFLAG(spec_pi_moving, Neutral);
        SETFLAG(spec_pi_moving, Black);
        pi_arriving= -abs(pi_arriving);
        if (king_square[White] == sq_arrival)
          king_square[Black] = sq_arrival;
      }
    }
  }

  if (!platzwechsel)
  {
    e[sq_arrival] = pi_arriving;
    spec[sq_arrival] = spec_pi_moving;
    jouearr[nbply] = pi_arriving;

    if (pi_departing!=pi_arriving)
    {
      nbpiece[pi_departing]--;
      nbpiece[pi_arriving]++;
    }
  }

  if (jouegenre)
  {
    if (pi_captured != vide)
    {
      if (kobulking[Black] && trait_ply == White && king_square[Black] != initsquare)
      {
        PieSpec sp;
        piece kobul = is_pawn(pi_captured) ? roin : pi_captured;
        nbpiece[e[king_square[Black]]]--;
        e[king_square[Black]] = kobul;
        if (TSTFLAG(spec[king_square[Black]], Neutral))
          setneutre(king_square[Black]);
        for (sp = Kamikaze; sp < PieSpCount; sp++)
          if (sp != Royal)
            TSTFLAG(spec_pi_captured, sp) ?
              SETFLAG(spec[king_square[Black]], sp) :
              CLRFLAG(spec[king_square[Black]], sp);
        nbpiece[kobul]++;
      }
      if (kobulking[White] && trait_ply == Black && king_square[White] != initsquare)
      {
        PieSpec sp;
        piece kobul = is_pawn(pi_captured) ? roib : pi_captured;
        nbpiece[e[king_square[White]]]--;
        e[king_square[White]] = kobul;
        if (TSTFLAG(spec[king_square[Black]], Neutral))
          setneutre(king_square[Black]);
        for (sp = Kamikaze; sp < PieSpCount; sp++)
          if (sp != Royal)
            TSTFLAG(spec_pi_captured, sp) ?
              SETFLAG(spec[king_square[White]], sp) :
              CLRFLAG(spec[king_square[White]], sp);
        nbpiece[kobul]++;
      }
    }

    if (CondFlag[ghostchess] || CondFlag[hauntedchess])
      summon_ghost(sq_departure);

    if (TSTFLAG(spec_pi_moving, HalfNeutral)
        && TSTFLAG(spec_pi_moving, Neutral))
      setneutre(sq_arrival);

    /* Duellantenschach */
    if (CondFlag[duellist]) {
      if (trait_ply == Black) {
        whduell[nbply]= whduell[nbply - 1];
        blduell[nbply]= sq_arrival;
      }
      else {
        blduell[nbply]= blduell[nbply - 1];
        whduell[nbply]= sq_arrival;
      }
    }

    if (CondFlag[singlebox] && SingleBoxType==singlebox_type2)
    {
      Side adv = advers(trait_ply);

      if (sb2[nbply].where==initsquare)
      {
        assert(sb2[nbply].what==vide);
        sb2[nbply].where = next_latent_pawn(initsquare,adv);
        if (sb2[nbply].where!=initsquare)
        {
          sb2[nbply].what = next_singlebox_prom(vide,adv);
          if (sb2[nbply].what==vide)
            sb2[nbply].where = initsquare;
        }
      }

      if (sb2[nbply].where!=initsquare)
      {
        assert(e[sb2[nbply].where] == (adv==White ? pb : pn));
        assert(sb2[nbply].what!=vide);
        --nbpiece[e[sb2[nbply].where]];
        e[sb2[nbply].where] = (adv==White
                                ? sb2[nbply].what
                                : -sb2[nbply].what);
        ++nbpiece[e[sb2[nbply].where]];
      }
    }

    /* AntiCirce */
    if (pi_captured != vide && anyanticirce) {
      sq_rebirth= (*antirenai)(nbply,
                               pi_arriving,
                               spec_pi_moving,
                               sq_capture,
                               sq_departure,
                               sq_arrival,
                               advers(trait_ply));
      if (CondFlag[antisuper])
      {
        while (!LegalAntiCirceMove(sq_rebirth, sq_capture, sq_departure))
          sq_rebirth++;
        current_super_circe_rebirth_square[nbply]= sq_rebirth;
      }
      e[sq_arrival]= vide;
      spec[sq_arrival]= 0;
      current_anticirce_rebirth_square[nbply]= sq_rebirth;
      prompieces[nbply]= GetPromotingPieces(sq_departure,
                                   pi_departing,
                                   trait_ply,
                                   spec_pi_moving,
                                   sq_rebirth,
                                   pi_captured);

      if (prompieces[nbply])
      {
        /* white pawn on eighth rank or
           black pawn on first rank - promotion ! */
        nbpiece[pi_arriving]--;
        pi_arriving= current_promotion_of_moving[nbply];
        if (pi_arriving == vide)
          current_promotion_of_moving[nbply]= pi_arriving= prompieces[nbply][vide];
        if (pi_departing < vide)
          pi_arriving= -pi_arriving;
        nbpiece[pi_arriving]++;
      }
      e[sq_rebirth]= pi_arriving;
      spec[sq_rebirth]= spec_pi_moving;
      if (sq_departure == prev_rb)
        king_square[White]= sq_rebirth;
      if (sq_departure == prev_rn)
        king_square[Black]= sq_rebirth;

      if (castling_supported) {
        PieNam const abspja= abs(pi_arriving);
        if (abspja==King) {
          if (TSTFLAG(spec_pi_moving, White)
              && sq_rebirth == square_e1
              && (!CondFlag[dynasty] || nbpiece[roib]==1))
            /* white king new on e1 */
            SETCASTLINGFLAGMASK(nbply,White,k_cancastle);
          else if (TSTFLAG(spec_pi_moving, Black)
                   && sq_rebirth == square_e8
                   && (!CondFlag[dynasty] || nbpiece[roin]==1))
            /* black king new on e8 */
            SETCASTLINGFLAGMASK(nbply,Black,k_cancastle);
        }
        else if (abspja==Rook) {
          if (TSTFLAG(spec_pi_moving, White)) {
            /* new white/neutral rook */
            if (sq_rebirth == square_h1)
              SETCASTLINGFLAGMASK(nbply,White,rh_cancastle);
            else if (sq_rebirth == square_a1)
              SETCASTLINGFLAGMASK(nbply,White,ra_cancastle);
          }
          if (TSTFLAG(spec_pi_moving, Black)) {
            /* new black/neutral rook */
            if (sq_rebirth == square_h8)
              SETCASTLINGFLAGMASK(nbply,Black,rh_cancastle);
            else if (sq_rebirth == square_a8)
              SETCASTLINGFLAGMASK(nbply,Black,ra_cancastle);
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

    if (CondFlag[sentinelles])
    {
      if (sq_departure>=square_a2 && sq_departure<=square_h7
          && !is_pawn(pi_departing))
      {
        if (SentPionNeutral) {
          if (TSTFLAG(spec_pi_moving, Neutral)) {
            nbpiece[e[sq_departure]= sentinelb]++;
            SETFLAG(spec[sq_departure], Neutral);
            setneutre(sq_departure);
          }
          else if ((trait_ply==Black) != SentPionAdverse) {
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
        else if ((trait_ply==Black) != SentPionAdverse) {
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
  } /* if (jouegenre) */

  unlock_post_move_iterations();;
} /* end of jouecoup */

void repcoup(void)
{
  square sq_rebirth;
  piece pi_departing, pi_captured;
  Flags spec_pi_moving;
  square nextsuper= superbas;
  boolean rochade=false;
  boolean platzwechsel = false;

  move_generation_elmt* move_gen_top = move_generation_stack+current_move[nbply];

  square sq_departure= move_gen_top->departure;
  square sq_arrival= move_gen_top->arrival;
  square sq_capture= move_gen_top->capture;

  assert(sq_arrival!=nullsquare);

  if (jouegenre)
  {
    if (sq_capture > platzwechsel_rochade)
    {
       sq_capture= sq_arrival;
       rochade= true;
    }
    else if (sq_capture == platzwechsel_rochade)
    {
      sq_capture= sq_arrival;
      platzwechsel= true;
    }


    if (flag_magic)
      ChangeMagic(nbply, false);

    if (CondFlag[singlebox] && SingleBoxType==singlebox_type2)
    {
      sb2[nbply+1].where = initsquare;
      sb2[nbply+1].what = vide;

      if (sb2[nbply].where!=initsquare)
      {
        Side adv = advers(trait[nbply]);

        assert(sb2[nbply].what!=vide);
        --nbpiece[e[sb2[nbply].where]];
        e[sb2[nbply].where] = adv==White ? pb : pn;
        ++nbpiece[e[sb2[nbply].where]];

        sb2[nbply].what = next_singlebox_prom(sb2[nbply].what,adv);
        if (sb2[nbply].what==vide)
        {
          sb2[nbply].where = next_latent_pawn(sb2[nbply].where,adv);
          if (sb2[nbply].where!=initsquare)
          {
            sb2[nbply].what = next_singlebox_prom(vide,adv);
            assert(sb2[nbply].what!=vide);
          }
        }

        if (sb2[nbply].where!=initsquare)
          lock_post_move_iterations();
      }
    }
  } /* jouegenre */

  pi_captured= pprise[nbply];
  pi_departing= pjoue[nbply];
  spec_pi_moving= jouespec[nbply];

  if (jouegenre)
  {
    if (CondFlag[ghostchess] && pi_captured!=vide)
      forget_ghost(sq_arrival);
    if (CondFlag[hauntedchess] && pi_captured!=vide)
    {
      forget_ghost(sq_arrival);
      unpreempt_ghost(sq_arrival);
    }

    if (CondFlag[singlebox] && SingleBoxType==singlebox_type3
        && sb3[current_move[nbply]].what!=vide) {
      piece pawn = trait[nbply]==White ? pb : pn;
      e[sb3[current_move[nbply]].where] = pawn;
      if (sq_departure!=sb3[current_move[nbply]].where) {
        --nbpiece[sb3[current_move[nbply]].what];
        ++nbpiece[pawn];
      }
    }
  }

  castling_flag[nbply]= castling_flag[nbply-1];
  whpwr[nbply]= whpwr[nbply-1];
  blpwr[nbply]= blpwr[nbply-1];

  switch (sq_capture)
  {
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
  if ((CondFlag[supercirce] && pi_captured!=vide)
      || isapril[abs(pi_captured)]
      || (CondFlag[antisuper] && pi_captured!=vide))
  {
    nextsuper = current_super_circe_rebirth_square[nbply]+1;

    while (e[nextsuper]!=vide && nextsuper<square_h8)
      ++nextsuper;

    if (CondFlag[antisuper]
        && AntiCirCheylan
        && nextsuper==sq_capture)
    {
      ++nextsuper;
      while (e[nextsuper]!=vide && nextsuper<square_h8)
        ++nextsuper;
    }
  }

  if (CondFlag[circecage] && pi_captured!=vide)
  {
    piece circecage_next_norm_prom = current_promotion_of_moving[nbply];
    piece circecage_next_cage_prom = current_promotion_of_reborn[nbply];
    square const currcage = current_super_circe_rebirth_square[nbply];
    square nextcage = currcage;

    if (circecage_next_cage_prom!=vide)
      circecage_advance_cage_prom(nbply,nextcage,&circecage_next_cage_prom);

    if (circecage_next_cage_prom==vide)
    {
      /* prevent current prisoner from disturbing the search for the
       * next cage
       */
      piece const currprisoner = e[currcage];
      e[currcage] = vide;

      circecage_advance_cage(nbply,
                             pi_captured,
                             &nextcage,
                             &circecage_next_cage_prom);

      if (nextcage>square_h8)
      {
        nextcage = superbas;
        if (circecage_next_norm_prom!=vide)
          circecage_advance_norm_prom(nbply,
                                      sq_arrival,pi_captured,
                                      &nextcage,
                                      &circecage_next_cage_prom,
                                      &circecage_next_norm_prom);
      }

      e[currcage] = currprisoner;
    }

    current_super_circe_rebirth_square[nbply] = nextcage;
    current_promotion_of_reborn[nbply] = circecage_next_cage_prom;
    current_promotion_of_moving[nbply] = circecage_next_norm_prom;

    if (nextcage!=superbas)
      lock_post_move_iterations();
  }

  /* first delete all changes */
  if (repgenre)
  {
    if (trait[nbply] == White && kobulking[Black] && king_square[Black] != initsquare)
    {
      nbpiece[e[king_square[Black]]]--;
      e[king_square[Black]] = blkobul[nbply];
      spec[king_square[Black]] = blkobulspec[nbply];
      nbpiece[blkobul[nbply]]++;
    }
    if (trait[nbply] == Black && kobulking[White] && king_square[White] != initsquare)
    {
      nbpiece[e[king_square[White]]]--;
      e[king_square[White]] = whkobul[nbply];
      spec[king_square[White]] = whkobulspec[nbply];
      nbpiece[whkobul[nbply]]++;
    }

    if ((CondFlag[ghostchess] || CondFlag[hauntedchess])
        && e[sq_departure]!=vide)
      ban_ghost(sq_departure);

    if (senti[nbply]) {
      --nbpiece[e[sq_departure]];
      senti[nbply]= false;
    }
    if (CondFlag[imitators])
    {
      if (sq_capture == queenside_castling)
        joueim(+dir_left);
      else if (rochade)
        joueim((sq_departure + rochade_sq[current_move[nbply]] - 3*sq_arrival) / 2);
      else if (sq_capture != kingside_castling) /* joueim(0) (do nothing) if OO */
        joueim(sq_departure - sq_arrival);      /* verschoben TLi */
    }

    if (TSTFLAG(PieSpExFlags, Neutral)) {
      /* the following is faster !  TLi
       * initialise_neutrals((pi_departing > vide) ? White : Black);
       */

      if (TSTFLAG(spec_pi_moving, Neutral) &&
          (pi_departing < vide ? Black : White) != neutral_side)
        pi_departing= -pi_departing;
      if (TSTFLAG(pprispec[nbply], Neutral) &&
          (pi_captured < vide ? Black : White) != neutral_side)
        pi_captured= -pi_captured;
    }
    sq_rebirth = current_circe_rebirth_square[nbply];
    if (sq_rebirth!=initsquare)
    {
      current_circe_rebirth_square[nbply] = initsquare;
      if (sq_rebirth != sq_arrival)
      {
        nbpiece[e[sq_rebirth]]--;
        e[sq_rebirth]= vide;
        spec[sq_rebirth]= 0;
      }
      if (CondFlag[circeassassin] && pdisp[nbply]) {
        if (e[sq_rebirth])
          nbpiece[e[sq_rebirth]]--;
        nbpiece[e[sq_rebirth]= pdisp[nbply]]++;
        spec[sq_rebirth]= pdispspec[nbply];
      }
      if (anytraitor)
        spec[sq_rebirth]= pdispspec[nbply];
    }

    if ((sq_rebirth= current_anticirce_rebirth_square[nbply]) != initsquare) {
      /* Kamikaze and AntiCirce */
      current_anticirce_rebirth_square[nbply]= initsquare;
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
  if (platzwechsel)
  {
    e[sq_arrival]=e[sq_departure];
    spec[sq_arrival]=spec[sq_departure];
  }

  e[sq_departure]= pi_departing;
  spec[sq_departure] = spec_pi_moving;
  nbpiece[pi_departing]++;

  if (rochade)
  {
     square sq_castle= (sq_departure + sq_arrival) / 2;
     e[rochade_sq[current_move[nbply]]] = e[sq_castle];
     spec[rochade_sq[current_move[nbply]]] = spec[sq_castle];
     e[sq_castle] = vide;
     if (king_square[Black] == sq_castle)
       king_square[Black]= rochade_sq[current_move[nbply]];
     if (king_square[White] == sq_castle)
       king_square[White]= rochade_sq[current_move[nbply]];
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

  if (!platzwechsel && (!anyantimars || sq_capture != sq_departure))
  {
    e[sq_capture]= pi_captured;
    spec[sq_capture]= pprispec[nbply];
  }

  if (pi_captured != vide)
    nbpiece[pi_captured]++;

  king_square[White]= RB_[nbply];
  king_square[Black]= RN_[nbply];

  if (abs(pi_departing) == andergb) {
    square sq= sq_arrival - direction(sq_departure, sq_arrival);

    if (!TSTFLAG(spec[sq], Neutral) && (sq != king_square[White]) && (sq != king_square[Black])) {
      change_side(sq);
      CHANGECOLOR(spec[sq]);
    }
  } /* andergb */

  /* at last modify promotion-counters and decrement current_move[nbply] */
  /* ortho- und pwc-Umwandlung getrennt */

  if (!are_post_move_iterations_locked())
  {
    piece* prompieceset = prompieces[nbply] ? prompieces[nbply] : getprompiece;

    piece prom_kind_moving = current_promotion_of_moving[nbply];
    if (prom_kind_moving!=vide)
    {
      prom_kind_moving = prompieceset[prom_kind_moving];
      if (!(CondFlag[singlebox] && SingleBoxType==singlebox_type2))
      {
        if (pi_captured!=vide)
        {
          if (anyanticirce)
            while (prom_kind_moving!=vide)
            {
              sq_rebirth = (*antirenai)(nbply,
                                        prom_kind_moving,
                                        spec_pi_moving,
                                        sq_capture,
                                        sq_departure,
                                        sq_arrival,
                                        advers(trait[nbply]));
              if (sq_rebirth==sq_departure
                  || LegalAntiCirceMove(sq_rebirth,sq_capture,sq_departure))
                break;
              else
                prom_kind_moving = prompieceset[prom_kind_moving];
            }
        }
      }

      current_promotion_of_moving[nbply] = prom_kind_moving;

      if (prom_kind_moving==vide
          && TSTFLAG(PieSpExFlags,Chameleon)
          && !is_moving_chameleon_promoted[nbply])
      {
        prom_kind_moving= prompieceset[vide];
        if (pi_captured != vide && anyanticirce)
          while (prom_kind_moving != vide
                 && ((sq_rebirth= (*antirenai)(nbply,
                                               prom_kind_moving,
                                               spec_pi_moving,
                                               sq_capture,
                                               sq_departure,
                                               sq_arrival,
                                               advers(trait[nbply])))
                     != sq_departure)
                 && e[sq_rebirth] != vide)
            prom_kind_moving= prompieceset[prom_kind_moving];

        current_promotion_of_moving[nbply]= prom_kind_moving;
        is_moving_chameleon_promoted[nbply]= true;
      }
      if (prom_kind_moving==vide && !CondFlag[noiprom])
        Iprom[nbply]= true;
    }
    else if (!CondFlag[noiprom] && Iprom[nbply])
    {
      ply icount;
      for (icount= nbply; icount<=maxply; icount++)
        --inum[icount];
      Iprom[nbply]= false;
    }

    if (prom_kind_moving == vide)
    {
      piece prom_kind_reborn = vide;
      is_moving_chameleon_promoted[nbply]= false;
      if (anycirprom)
      {
        prom_kind_reborn = current_promotion_of_reborn[nbply];
        if (prom_kind_reborn!=vide)
        {
          prom_kind_reborn = getprompiece[prom_kind_reborn];
          if (prom_kind_reborn==vide
              && TSTFLAG(PieSpExFlags, Chameleon)
              && !is_reborn_chameleon_promoted[nbply])
          {
            prom_kind_reborn = getprompiece[vide];
            is_reborn_chameleon_promoted[nbply]= true;
          }
        }
      }

      current_promotion_of_reborn[nbply] = prom_kind_reborn;

      if (prom_kind_reborn==vide
          && !(!CondFlag[noiprom] && Iprom[nbply]))
      {
        if ((CondFlag[supercirce] && pi_captured != vide)
            || isapril[abs(pi_captured)]
            || (CondFlag[antisuper] && pi_captured != vide))
        {
          current_super_circe_rebirth_square[nbply] = nextsuper;
          if (current_super_circe_rebirth_square[nbply]>square_h8
              || (CondFlag[antisuper]
                  && !LegalAntiCirceMove(nextsuper,sq_capture,sq_departure)))
          {
            current_super_circe_rebirth_square[nbply]= superbas;
            current_move[nbply]--;
          }
        }
        else
          current_move[nbply]--;
      }
    }
  } /* post_move_iterations_locked*/
} /* end of repcoup */
