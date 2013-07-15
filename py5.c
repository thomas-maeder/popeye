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
#include "pieces/walks/walks.h"
#include "pieces/attributes/paralysing/paralysing.h"
#include "pieces/walks/pawns/pawn.h"
#include "position/pieceid.h"
#include "optimisations/hash.h"
#include "debugging/trace.h"

square renrank(PieNam p_captured, Flags p_captured_spec,
               square sq_capture, square sq_departure, square sq_arrival,
               Side capturer) {
  square sq= ((sq_capture/onerow)%2==1
              ? rennormal(p_captured,p_captured_spec,
                          sq_capture,sq_departure,sq_arrival,capturer)
              : renspiegel(p_captured,p_captured_spec,
                           sq_capture,sq_departure,sq_arrival,capturer));
  return onerow*(sq_capture/onerow) + sq%onerow;
}

square renfile(PieNam p_captured, Flags p_captured_spec,
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
    if (is_pawn(p_captured))
      result = col + (nr_of_slack_rows_below_board+1)*onerow;
    else
      result = col + nr_of_slack_rows_below_board*onerow;
  }
  else
  {
    if (is_pawn(p_captured))
      result = col + (nr_of_slack_rows_below_board+nr_rows_on_board-2)*onerow;
    else
      result = col + (nr_of_slack_rows_below_board+nr_rows_on_board-1)*onerow;
  }

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

square renspiegelfile(PieNam p_captured, Flags p_captured_spec,
                      square sq_capture,
                      square sq_departure, square sq_arrival,
                      Side capturer)
{
  return renfile(p_captured,p_captured_spec,
                 sq_capture,sq_departure,sq_arrival,advers(capturer));
} /* renspiegelfile */

square renpwc(PieNam p_captured, Flags p_captured_spec,
              square sq_capture, square sq_departure, square sq_arrival,
              Side capturer)
{
  return sq_departure;
} /* renpwc */

square renequipollents(PieNam p_captured, Flags p_captured_spec,
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

square renequipollents_anti(PieNam p_captured, Flags p_captured_spec,
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

square rensymmetrie(PieNam p_captured, Flags p_captured_spec,
                    square sq_capture,
                    square sq_departure, square sq_arrival,
                    Side capturer)
{
  return (square_h8+square_a1) - sq_capture;
} /* rensymmetrie */

square renantipoden(PieNam p_captured, Flags p_captured_spec,
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

square rendiagramm(PieNam p_captured, Flags p_captured_spec,
                   square sq_capture, square sq_departure, square sq_arrival,
                   Side capturer)
{
  return GetPositionInDiagram(p_captured_spec);
}

square rennormal(PieNam pnam_captured, Flags p_captured_spec,
                 square sq_capture, square sq_departure, square sq_arrival,
                 Side capturer)
{
  square  Result;
  unsigned int col = sq_capture % onerow;
  unsigned int const ran = sq_capture / onerow;

  TraceFunctionEntry(__func__);
  TracePiece(pnam_captured);
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

square rendiametral(PieNam p_captured, Flags p_captured_spec,
                    square sq_capture,
                    square sq_departure, square sq_arrival,
                    Side capturer) {
  return (square_h8+square_a1
          - rennormal(p_captured,p_captured_spec,
                      sq_capture,sq_departure,sq_arrival,capturer));
}

square renspiegel(PieNam p_captured, Flags p_captured_spec,
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
    case  MarinePawn:
      return  true;

    default:
      return  false;
  }
}

boolean is_king(PieNam p)
{
  switch (p)
  {
    case  King:
    case  Poseidon:
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
    case  MarinePawn:
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
    case  MarinePawn:
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
    if (!is_square_empty(s))
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
    occupy_square(to,get_walk_of_piece_on_square(from),spec[from]);
    empty_square(from);

    if (king_square[side]!=initsquare)
      king_square[side] = to;

    result = !echecc(side);

    occupy_square(from,get_walk_of_piece_on_square(to),spec[to]);
    empty_square(to);

    if (king_square[side]!=initsquare)
      king_square[side] = from;
  }

  return result;
}

void generate_castling(void)
{
  Side const side = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (TSTCASTLINGFLAGMASK(nbply,side,castlings)>k_cancastle)
  {
    castling_flag_type allowed_castlings = 0;

    square const square_a = side==White ? square_a1 : square_a8;
    square const square_c = square_a+file_c;
    square const square_d = square_a+file_d;
    square const square_e = square_a+file_e;
    square const square_f = square_a+file_f;
    square const square_g = square_a+file_g;
    square const square_h = square_a+file_h;

    /* 0-0 */
    if (TSTCASTLINGFLAGMASK(nbply,side,k_castling)==k_castling
        && are_squares_empty(square_e,square_h,dir_right))
       allowed_castlings |= rh_cancastle;

    /* 0-0-0 */
    if (TSTCASTLINGFLAGMASK(nbply,side,q_castling)==q_castling
        && are_squares_empty(square_e,square_a,dir_left))
      allowed_castlings |= ra_cancastle;

    if (allowed_castlings!=0 && !echecc(side))
    {
      if ((allowed_castlings&rh_cancastle)
          && castling_is_intermediate_king_move_legal(side,square_e,square_f))
        add_to_move_generation_stack(square_e,square_g,kingside_castling);

      if ((allowed_castlings&ra_cancastle)
          && castling_is_intermediate_king_move_legal(side,square_e,square_d))
        add_to_move_generation_stack(square_e,square_c,queenside_castling);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
