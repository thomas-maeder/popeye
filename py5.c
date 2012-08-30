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
#include "solving/castling.h"
#include "stipulation/goals/doublemate/reached_tester.h"
#include "stipulation/temporary_hacks.h"
#include "stipulation/move_player.h"
#include "solving/single_piece_move_generator.h"
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
#include "conditions/circe/super.h"
#include "conditions/anticirce/super.h"
#include "conditions/haunted_chess.h"
#include "conditions/singlebox/type1.h"
#include "conditions/singlebox/type2.h"
#include "conditions/singlebox/type3.h"
#include "pieces/side_change.h"
#include "pieces/attributes/paralysing/paralysing.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "optimisations/hash.h"
#include "debugging/trace.h"


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
          break;
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
          break;
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
    e[to] = e[from];
    spec[to] = spec[from];

    e[from] = vide;
    CLEARFL(spec[from]);

    if (king_square[side]!=initsquare)
      king_square[side] = to;

    result = !echecc(side);

    e[from] = e[to];
    spec[from] = spec[to];

    e[to] = vide;
    CLEARFL(spec[to]);

    if (king_square[side]!=initsquare)
      king_square[side] = from;
  }

  return result;
}

void generate_castling(Side side)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  if (!dont_generate_castling)
  {
    if (TSTCASTLINGFLAGMASK(nbply,side,castlings)>k_cancastle
        && !echecc(side))
    {
      square const square_a = side==White ? square_a1 : square_a8;
      square const square_c = square_a+file_c;
      square const square_d = square_a+file_d;
      square const square_e = square_a+file_e;
      square const square_f = square_a+file_f;
      square const square_g = square_a+file_g;
      square const square_h = square_a+file_h;

      /* 0-0 */
      if (TSTCASTLINGFLAGMASK(nbply,side,k_castling)==k_castling
          && are_squares_empty(square_e,square_h,dir_right)
          && castling_is_intermediate_king_move_legal(side,square_e,square_f))
        empile(square_e,square_g,kingside_castling);

      /* 0-0-0 */
      if (TSTCASTLINGFLAGMASK(nbply,side,q_castling)==q_castling
          && are_squares_empty(square_e,square_a,dir_left)
          && castling_is_intermediate_king_move_legal(side,square_e,square_d))
        empile(square_e,square_c,queenside_castling);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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

  if (CondFlag[platzwechselrochade] && platzwechsel_rochade_allowed[Black][nbply])
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

  if (TSTFLAG(PieSpExFlags,Paralyse))
  {
    if (paralysiert(sq_departure))
    {
      TraceFunctionExit(__func__);
      TraceFunctionResultEnd();
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
      if (p == e[king_square[Black]] && !rex_phan)
      {
        TraceFunctionExit(__func__);
        TraceFunctionResultEnd();
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
      {
        TraceFunctionExit(__func__);
        TraceFunctionResultEnd();
        return;
      }
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
    PieNam pprom;
    for (pprom = next_singlebox_prom(Empty,Black);
         pprom!=Empty;
         pprom = next_singlebox_prom(pprom,Black))
    {
      numecoup prev_nbcou = current_move[nbply];
      ++latent_prom;
      e[sq] = -pprom;
      orig_gen_bl_piece(z, sq==z ? (piece)-pprom : p);
      e[sq] = pn;

      for (++prev_nbcou; prev_nbcou<=current_move[nbply]; ++prev_nbcou)
      {
        singlebox_type3_promotions[prev_nbcou].where = sq;
        singlebox_type3_promotions[prev_nbcou].what = -pprom;
      }
    }
  }

  if (latent_prom==0)
  {
    orig_gen_bl_piece(z,p);
    for (++save_nbcou; save_nbcou<=current_move[nbply]; ++save_nbcou)
    {
      singlebox_type3_promotions[save_nbcou].where = initsquare;
      singlebox_type3_promotions[save_nbcou].what = vide;
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
