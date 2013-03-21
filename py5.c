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
#include "solving/solve.h"
#include "solving/castling.h"
#include "stipulation/goals/doublemate/reached_tester.h"
#include "stipulation/temporary_hacks.h"
#include "stipulation/move.h"
#include "solving/single_piece_move_generator.h"
#include "solving/observation.h"
#include "conditions/disparate.h"
#include "conditions/eiffel.h"
#include "conditions/extinction.h"
#include "conditions/madrasi.h"
#include "conditions/mummer.h"
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
#include "conditions/beamten.h"
#include "conditions/patrol.h"
#include "conditions/central.h"
#include "conditions/phantom.h"
#include "conditions/marscirce/marscirce.h"
#include "conditions/marscirce/anti.h"
#include "conditions/marscirce/plus.h"
#include "conditions/annan.h"
#include "pieces/walks.h"
#include "pieces/attributes/paralysing/paralysing.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "optimisations/hash.h"
#include "debugging/trace.h"


piece champiece(piece pi_arriving)
{
  PieNam const walk_arriving = abs(pi_arriving);
  PieNam walk_chameleonised = walk_arriving;

  if (walk_arriving==standard_walks[Queen])
    walk_chameleonised = standard_walks[Knight];
  else if (walk_arriving==standard_walks[Knight])
    walk_chameleonised = standard_walks[Bishop];
  else if (walk_arriving==standard_walks[Bishop])
    walk_chameleonised = standard_walks[Rook];
  else if (walk_arriving==standard_walks[Rook])
    walk_chameleonised = standard_walks[Queen];

  return  pi_arriving<vide ? -walk_chameleonised : walk_chameleonised;
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
  square result;

  TraceFunctionEntry(__func__);
  TracePiece(p_captured);
  TraceSquare(sq_capture);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceEnumerator(Side,capturer,"");
  TraceFunctionParamListEnd();

  if (capturer==Black)
  {
    if (is_pawn(abs(p_captured)))
      result = col + (nr_of_slack_rows_below_board+1)*onerow;
    else
      result = col + nr_of_slack_rows_below_board*onerow;
  }
  else
  {
    if (is_pawn(abs(p_captured)))
      result = col + (nr_of_slack_rows_below_board+nr_rows_on_board-2)*onerow;
    else
      result = col + (nr_of_slack_rows_below_board+nr_rows_on_board-1)*onerow;
  }

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
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
  return (square_h8+square_a1) - sq_capture;
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
  return GetPositionInDiagram(p_captured_spec);
}

square rennormal(piece p_captured, Flags p_captured_spec,
                 square sq_capture, square sq_departure, square sq_arrival,
                 Side capturer)
{
  square  Result;
  unsigned int col = sq_capture % onerow;
  unsigned int const ran = sq_capture / onerow;
  PieNam pnam_captured = abs(p_captured);

  TraceFunctionEntry(__func__);
  TracePiece(p_captured);
  TraceValue("%d",p_captured);
  TraceSquare(sq_capture);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceEnumerator(Side,capturer,"");
  TraceFunctionParamListEnd();

  if (CondFlag[circemalefiquevertical])
  {
    col = onerow-1 - col;
    if (pnam_captured==Queen)
      pnam_captured = King;
    else if (pnam_captured==King)
      pnam_captured = Queen;
  }

  {
    Side const cou = (ran&1) != (col&1) ? White : Black;

    if (capturer == Black)
    {
      if (is_pawn(pnam_captured))
        Result = col + (nr_of_slack_rows_below_board+1)*onerow;
      else if (CondFlag[frischauf] && TSTFLAG(p_captured_spec,FrischAuf))
        Result = (col
                  + (onerow
                     *(CondFlag[glasgow]
                       ? nr_of_slack_rows_below_board+nr_rows_on_board-2
                       : nr_of_slack_rows_below_board+nr_rows_on_board-1)));
      else if (pnam_captured==standard_walks[Knight])
        Result = cou == White ? square_b1 : square_g1;
      else if (pnam_captured==standard_walks[Rook])
        Result = cou == White ? square_h1 : square_a1;
      else if (pnam_captured==standard_walks[Queen])
        Result = square_d1;
      else if (pnam_captured==standard_walks[Bishop])
        Result = cou == White ? square_f1 : square_c1;
      else if (pnam_captured==standard_walks[King])
        Result = square_e1;
      else
        Result = (col
                  + (onerow
                     *(CondFlag[glasgow]
                       ? nr_of_slack_rows_below_board+nr_rows_on_board-2
                       : nr_of_slack_rows_below_board+nr_rows_on_board-1)));
    }
    else
    {
      if (is_pawn(pnam_captured))
        Result = col + (nr_of_slack_rows_below_board+nr_rows_on_board-2)*onerow;
      else if (CondFlag[frischauf] && TSTFLAG(p_captured_spec,FrischAuf))
        Result = (col
                  + (onerow
                     *(CondFlag[glasgow]
                       ? nr_of_slack_rows_below_board+1
                       : nr_of_slack_rows_below_board)));
      else if (pnam_captured==standard_walks[King])
        Result = square_e8;
      else if (pnam_captured==standard_walks[Knight])
        Result = cou == White ? square_g8 : square_b8;
      else if (pnam_captured==standard_walks[Rook])
        Result = cou == White ? square_a8 : square_h8;
      else if (pnam_captured==standard_walks[Queen])
        Result = square_d8;
      else if (pnam_captured==standard_walks[Bishop])
        Result = cou == White ? square_c8 : square_f8;
      else
        Result = (col
                  + (onerow
                     *(CondFlag[glasgow]
                       ? nr_of_slack_rows_below_board+1
                       : nr_of_slack_rows_below_board)));
    }
  }

  TraceFunctionExit(__func__);
  TraceSquare(Result);
  TraceFunctionResultEnd();
  return(Result);
}

square rendiametral(piece p_captured, Flags p_captured_spec,
                    square sq_capture,
                    square sq_departure, square sq_arrival,
                    Side capturer) {
  return (square_h8+square_a1
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

#if defined(DOS)
# pragma warn +par
#endif

boolean is_short(PieNam p)
{
  switch (p)
  {
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

boolean is_pawn(PieNam p)
{
  switch (p)
  {
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

boolean is_forwardpawn(PieNam p)
{
  switch (p)
  {
    case  Pawn:
    case  BerolinaPawn:
    case  SuperBerolinaPawn:
    case  SuperPawn:
      return  true;

    default:
      return  false;
  }
}

boolean is_reversepawn(PieNam p)
{
  switch (p)
  {
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
    castling_intermediate_move_generator_init_next(from,to);
    result = solve(slices[temporary_hack_castling_intermediate_move_legality_tester[side]].next2,length_unspecified)==next_move_has_solution;
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

  if (calc_reflective_king[side] && !calctransmute)
  {
    /* K im Schach zieht auch */
    calctransmute = true;
    if (!normaltranspieces[side] && echecc(side))
    {
      PieNam *ptrans;
      for (ptrans = transmpieces[side]; *ptrans!=Empty; ++ptrans)
      {
        flag = true;
        current_trans_gen = -*ptrans;
        gen_bl_piece(sq_departure,-*ptrans);
        current_trans_gen = vide;
      }
    }
    else if (normaltranspieces[side])
    {
      PieNam const *ptrans;
      for (ptrans = transmpieces[side]; *ptrans!=Empty; ++ptrans)
        if (nbpiece[*ptrans]>0
            && (*checkfunctions[*ptrans])(sq_departure,*ptrans,&validate_observation))
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
    if (calc_transmuting_king[side] && flag)
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
    int i;
    square square_a = square_a1;
    for (i = nr_rows_on_board; i>0; --i, square_a += onerow)
    {
      int j;
      square pos_partner = square_a;
      for (j = nr_files_on_board; j>0; --j, pos_partner += dir_right)
        if (pos_partner!=sq_departure
            && TSTFLAG(spec[pos_partner],Black)
            && !is_pawn(abs(e[pos_partner]))) /* not sure if "castling" with Ps forbidden */
          empile(sq_departure,pos_partner,platzwechsel_rochade);
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
  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TracePiece(p);
  TraceFunctionParamListEnd();

  if (!(CondFlag[madras] && !madrasi_can_piece_move(sq_departure))
      && !(CondFlag[eiffel] && !eiffel_can_piece_move(sq_departure))
      && !(CondFlag[disparate] && !disparate_can_piece_move(sq_departure))
      && !(TSTFLAG(PieSpExFlags,Paralysing) && is_piece_paralysed_on(sq_departure))
      && !(CondFlag[ultrapatrouille] && !patrol_is_supported(sq_departure))
      && !(CondFlag[central] && !central_can_piece_move_from(sq_departure))
      && !(TSTFLAG(spec[sq_departure],Beamtet) && !beamten_is_observed(sq_departure)))
  {
    if (CondFlag[phantom])
      phantom_chess_generate_moves(Black,p,sq_departure);
    else if (CondFlag[plus])
      plus_generate_moves(Black,p,sq_departure);
    else if (anymars)
      marscirce_generate_moves(Black,p,sq_departure);
    else if (anyantimars)
      antimars_generate_moves(Black,p,sq_departure);
    else
      gen_piece_aux(Black,sq_departure,p);

    if (CondFlag[messigny] && !(king_square[Black]==sq_departure && rex_mess_ex))
    {
      square const *bnp;
      for (bnp = boardnum; *bnp; ++bnp)
        if (e[*bnp]==-p)
          empile(sq_departure,*bnp,messigny_exchange);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
} /* orig_gen_bl_piece */

void singleboxtype3_gen_bl_piece(square z, piece p)
{
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
        move_generation_stack[prev_nbcou].singlebox_type3_promotion_where = sq;
        move_generation_stack[prev_nbcou].singlebox_type3_promotion_what = -pprom;
      }
    }
  }

  if (latent_prom==0)
  {
    orig_gen_bl_piece(z,p);
    for (++save_nbcou; save_nbcou<=current_move[nbply]; ++save_nbcou)
    {
      move_generation_stack[save_nbcou].singlebox_type3_promotion_where = initsquare;
      move_generation_stack[save_nbcou].singlebox_type3_promotion_what = vide;
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

  trait[nbply]= camp;
  we_generate_exact = false;

  /* exact and consequent maximummers */
  if (camp == White)
  {
    /* let's first generate consequent moves */
    if (mummer_strictness[White]>mummer_strictness_regular)
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
        nextply();
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
    if (mummer_strictness[Black]>mummer_strictness_regular)
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
        nextply();
        gen_bl_ply();
      }
      we_generate_exact = false;
    }
    else
      gen_bl_ply();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
} /* genmove(camp) */
