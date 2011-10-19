#include "optimisations/intelligent/count_nr_of_moves.h"
#include "pydata.h"
#include "pyint.h"
#include "pyproof.h"
#include "optimisations/intelligent/moves_left.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static unsigned int count_nr_of_moves_from_to_king_no_castling(square from, square to)
{
  unsigned int const diffcol = abs(from%onerow - to%onerow);
  unsigned int const diffrow = abs(from/onerow - to/onerow);

  return diffcol>diffrow ? diffcol : diffrow;
}

static unsigned int intelligent_count_nr_of_moves_from_to_king(Side side,
                                                               square from_square,
                                                               square to_square)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceSquare(from_square);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  result = count_nr_of_moves_from_to_king_no_castling(from_square,to_square);

  if (testcastling)
  {
    if (side==White)
    {
      if (from_square==square_e1)
      {
        if (TSTCASTLINGFLAGMASK(nbply,White,ra_cancastle&castling_flag[castlings_flags_no_castling]))
        {
          unsigned int const withcast = count_nr_of_moves_from_to_king_no_castling(square_c1,to_square);
          if (withcast<result)
            result = withcast;
        }
        if (TSTCASTLINGFLAGMASK(nbply,White,rh_cancastle&castling_flag[castlings_flags_no_castling]))
        {
          unsigned int const withcast = count_nr_of_moves_from_to_king_no_castling(square_g1,to_square);
          if (withcast<result)
            result = withcast;
        }
      }
    }
    else {
      if (from_square==square_e8)
      {
        if (TSTCASTLINGFLAGMASK(nbply,Black,ra_cancastle&castling_flag[castlings_flags_no_castling]))
        {
          unsigned int const withcast = count_nr_of_moves_from_to_king_no_castling(square_c8,to_square);
          if (withcast<result)
            result = withcast;
        }
        if (TSTCASTLINGFLAGMASK(nbply,Black,rh_cancastle&castling_flag[castlings_flags_no_castling]))
        {
          unsigned int const withcast = count_nr_of_moves_from_to_king_no_castling(square_g8,to_square);
          if (withcast<result)
            result = withcast;
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int intelligent_count_nr_of_moves_from_to_black_pawn_no_promotion(square from_square,
                                                                                  square to_square)
{
  unsigned int result;
  int const diffcol = abs(from_square%onerow - to_square%onerow);
  int const diffrow = from_square/onerow - to_square/onerow;

  if (diffrow<diffcol)
    /* if diffrow<=0 then this test is true, since diffcol is always
     * non-negative
     */
    result = maxply+1;

  else if (from_square>=square_a7 && diffrow-2 >= diffcol)
    /* double step */
    result = diffrow-1;

  else
    result = diffrow;

  return result;
}

static unsigned int intelligent_count_nr_of_moves_from_to_white_pawn_no_promotion(square from_square,
                                                                                  square to_square)
{
  unsigned int result;
  int const diffcol = abs(from_square%onerow - to_square%onerow);
  int const diffrow = from_square/onerow - to_square/onerow;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  /* white pawn */
  if (-diffrow<diffcol)
    result = maxply+1;

  else  if (from_square<=square_h2 && -diffrow-2 >= diffcol)
    /* double step */
    result = -diffrow-1;

  else
    result = -diffrow;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int intelligent_count_nr_of_moves_from_to_pawn_promotion(square from_square,
                                                                         piece to_piece,
                                                                         square to_square)
{
  unsigned int result = maxply+1;
  square const start = to_piece<vide ? square_a1 : square_a8;
  piece const pawn = to_piece<vide ? pn : pb;
  square prom_square;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TracePiece(to_piece);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  assert(from_square>=square_a2);
  assert(from_square<=square_h7);

  for (prom_square = start; prom_square<start+nr_files_on_board; ++prom_square)
  {
    unsigned int const to_prom = (pawn>obs
                                  ? intelligent_count_nr_of_moves_from_to_white_pawn_no_promotion(from_square,prom_square)
                                  : intelligent_count_nr_of_moves_from_to_black_pawn_no_promotion(from_square,prom_square));
    unsigned int const from_prom = intelligent_count_nr_of_moves_from_to_no_check(to_piece,
                                                                                  prom_square,
                                                                                  to_piece,
                                                                                  to_square);
    unsigned int const total = to_prom+from_prom;
    if (total<result)
      result = total;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int count_nr_of_moves_from_to_from_to_different(piece from_piece,
                                                                square from_square,
                                                                piece to_piece,
                                                                square to_square)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TracePiece(from_piece);
  TraceSquare(from_square);
  TracePiece(to_piece);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  switch (abs(from_piece))
  {
    case Knight:
      result = ProofKnightMoves[abs(from_square-to_square)];
      break;

    case Rook:
      result = CheckDirRook[from_square-to_square]==0 ? 2 : 1;
      break;

    case Queen:
      result = ((CheckDirRook[from_square-to_square]==0
                 && CheckDirBishop[from_square-to_square]==0)
                ? 2
                : 1);
      break;

    case Bishop:
      if (SquareCol(from_square)==SquareCol(to_square))
        result = CheckDirBishop[from_square-to_square]==0 ? 2 : 1;
      else
        result = maxply+1;
      break;

    case King:
      result = intelligent_count_nr_of_moves_from_to_king(from_piece>obs ? White : Black,
                                                          from_square,
                                                          to_square);
      break;

    case Pawn:
      if (from_piece==to_piece)
        result = (from_piece>obs
                  ? intelligent_count_nr_of_moves_from_to_white_pawn_no_promotion(from_square,to_square)
                  : intelligent_count_nr_of_moves_from_to_black_pawn_no_promotion(from_square,to_square));
      else
        result = intelligent_count_nr_of_moves_from_to_pawn_promotion(from_square,
                                                                      to_piece,
                                                                      to_square);
      break;

    default:
      assert(0);
      result = UINT_MAX;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

unsigned int intelligent_count_nr_of_moves_from_to_no_check(piece from_piece,
                                                            square from_square,
                                                            piece to_piece,
                                                            square to_square)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TracePiece(from_piece);
  TraceSquare(from_square);
  TracePiece(to_piece);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  if (from_square==to_square && from_piece==to_piece)
    result = 0;
  else
    result = count_nr_of_moves_from_to_from_to_different(from_piece,from_square,
                                                         to_piece,to_square);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean intelligent_reserve_black_pawn_moves_from_to_no_promotion(square from_square,
                                                                  square to_square)
{
  boolean result;
  unsigned int nr_of_moves;
  unsigned int const diffcol = abs(from_square%onerow - to_square%onerow);

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  nr_of_moves = intelligent_count_nr_of_moves_from_to_black_pawn_no_promotion(from_square,
                                                                              to_square);

  if (nr_of_moves<=Nr_remaining_moves[Black]
      && diffcol<=Nr_unused_masses[White])
  {
    Nr_remaining_moves[Black] -= nr_of_moves;
    Nr_unused_masses[White] -= diffcol;
    TraceValue("%u",Nr_remaining_moves[Black]);
    TraceValue("%u\n",Nr_unused_masses[White]);
    result = true;
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean intelligent_reserve_white_pawn_moves_from_to_no_promotion(square from_square,
                                                                  square to_square)
{
  boolean result;
  unsigned int nr_of_moves;
  unsigned int const diffcol = abs(from_square%onerow - to_square%onerow);

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  nr_of_moves = intelligent_count_nr_of_moves_from_to_white_pawn_no_promotion(from_square,
                                                                              to_square);

  if (nr_of_moves<=Nr_remaining_moves[White]
      && diffcol<=Nr_unused_masses[Black])
  {
    Nr_remaining_moves[White] -= nr_of_moves;
    Nr_unused_masses[Black] -= diffcol;
    TraceValue("%u",Nr_remaining_moves[White]);
    TraceValue("%u\n",Nr_unused_masses[Black]);
    result = true;
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int count_nr_black_moves_to_square_with_promoted_pawn(square pawn_comes_from,
                                                                      square to_be_blocked)
{
  unsigned int result = maxply+1;

  TraceFunctionEntry(__func__);
  TraceSquare(pawn_comes_from);
  TraceSquare(to_be_blocked);
  TraceFunctionParamListEnd();

  {
    /* A rough check whether it is worth thinking about promotions */
    unsigned int moves = (pawn_comes_from>=square_a7
                          ? 5
                          : pawn_comes_from/onerow - nr_of_slack_rows_below_board);
    assert(moves<=5);

    if (to_be_blocked>=square_a2)
      /* square is not on 8th rank -- 1 move necessary to get there */
      ++moves;

    if (Nr_remaining_moves[Black]>=moves)
    {
      piece pp;
      for (pp = -getprompiece[vide]; pp!=vide; pp = -getprompiece[-pp])
      {
        unsigned int const time = intelligent_count_nr_of_moves_from_to_pawn_promotion(pawn_comes_from,
                                                                                       pp,
                                                                                       to_be_blocked);
        if (time<result)
          result = time;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

unsigned int intelligent_estimate_min_nr_black_moves_to_square(square to_be_blocked)
{
  unsigned int result = maxply+1;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceSquare(to_be_blocked);
  TraceFunctionParamListEnd();

  for (i = 1; i<MaxPiece[Black]; ++i)
    if (black[i].usage==piece_is_unused)
    {
      piece const blocker_type = black[i].type;
      square const blocker_comes_from = black[i].diagram_square;

      if (blocker_type==pn)
      {
        if (to_be_blocked>=square_a2)
        {
          unsigned int const time = intelligent_count_nr_of_moves_from_to_black_pawn_no_promotion(blocker_comes_from,
                                                                                                  to_be_blocked);
          if (time<result)
            result = time;
        }

        {
          unsigned int const time_prom = count_nr_black_moves_to_square_with_promoted_pawn(blocker_comes_from,
                                                                                           to_be_blocked);
          if (time_prom<result)
            result = time_prom;
        }
      }
      else
      {
        unsigned int const time = intelligent_count_nr_of_moves_from_to_no_check(blocker_type,
                                                                                 blocker_comes_from,
                                                                                 blocker_type,
                                                                                 to_be_blocked);
        if (time<result)
          result = time;
      }
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

unsigned int intelligent_count_nr_of_moves_from_to_checking(piece from_piece,
                                                            square from_square,
                                                            piece to_piece,
                                                            square to_square)
{
  if (from_square==to_square && from_piece==to_piece)
  {
    if (from_piece==pb)
      return maxply+1;

    else if (from_piece==cb)
      return 2;

    /* it's a rider */
    else if (move_diff_code[abs(king_square[Black]-to_square)]<3)
      return 2;

    else
      return 0;
  }
  else
    return count_nr_of_moves_from_to_from_to_different(from_piece,from_square,
                                                       to_piece,to_square);
}

/* Tests if a specific checking white sequence of moves by the same officer is
 * still possible.
 * @param from_square from
 * @param to_square to
 * @return true iff the move sequence is still possible
 * @note modifies Nr_remaining_moves[side] if the move sequence is possible
 */
boolean intelligent_reserve_white_officer_moves_from_to_checking(piece piece,
                                                                 square from_square,
                                                                 square to_square)
{
  boolean result;
  unsigned int nr_of_moves;

  TraceFunctionEntry(__func__);
  TracePiece(piece);
  TraceSquare(from_square);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  if (from_square==to_square)
  {
    if (piece==cb)
      nr_of_moves = 2;

    /* it's a rider */
    else if (move_diff_code[abs(king_square[Black]-to_square)]<3)
      nr_of_moves = 2;

    else
      nr_of_moves = 0;
  }
  else
    nr_of_moves = count_nr_of_moves_from_to_from_to_different(piece,from_square,
                                                              piece,to_square);

  if (nr_of_moves<=Nr_remaining_moves[White])
  {
    Nr_remaining_moves[White] -= nr_of_moves;
    TraceValue("%u\n",Nr_remaining_moves[White]);
    result = true;
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Tests if a specific checking white sequence of moves by the same pawn is
 * still possible.
 * @param from_square from
 * @param to_square to
 * @return true iff the move sequence is still possible
 * @note modifies Nr_remaining_moves[side] and Nr_unused_masses[Black] if the
 *       move sequence is possible
 */
boolean intelligent_reserve_white_pawn_moves_from_to_checking(square from_square,
                                                              square to_square)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  if (from_square!=to_square)
  {
    unsigned int const diffcol = abs(from_square%onerow - to_square%onerow);
    if (diffcol<=Nr_unused_masses[Black])
    {
      unsigned int const nr_of_moves = intelligent_count_nr_of_moves_from_to_white_pawn_no_promotion(from_square,
                                                                                                     to_square);
      if (nr_of_moves<=Nr_remaining_moves[White])
      {
        Nr_unused_masses[Black] -= diffcol;
        Nr_remaining_moves[White] -= nr_of_moves;
        TraceValue("%u",Nr_unused_masses[Black]);
        TraceValue("%u\n",Nr_remaining_moves[White]);
        result = true;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Tests if a specific checking sequence of moves by the same pawn including its
 * promotion is still possible.
 * @param from_square from
 * @param promotee_type type of piece that the pawn promotes to
 * @param to_square to
 * @return true iff the move sequence is still possible
 * @note modifies Nr_remaining_moves[side] and Nr_unused_masses[opponent] if
 * the move sequence is possible
 */
boolean intelligent_reserve_promoting_pawn_moves_from_to(square from_square,
                                                         piece promotee_type,
                                                         square to_square)
{
  boolean result;
  Side const side = promotee_type>obs ? White : Black;
  Side const opponent = advers(side);
  unsigned int nr_of_moves;
  unsigned int diffcol = 0;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  if (abs(promotee_type)==Bishop)
  {
    square const start_prom_row = side==White ? square_a8 : square_a1;
    unsigned int const from_file = from_square%nr_files_on_board;
    square const promotion_square_on_same_file = start_prom_row+from_file;
    if (SquareCol(to_square)!=SquareCol(promotion_square_on_same_file))
      diffcol = 1;
  }

  nr_of_moves = intelligent_count_nr_of_moves_from_to_pawn_promotion(from_square,
                                                                     promotee_type,
                                                                     to_square);
  if (nr_of_moves<=Nr_remaining_moves[side]
      && diffcol<=Nr_unused_masses[opponent])
  {
    Nr_remaining_moves[side] -= nr_of_moves;
    Nr_unused_masses[opponent] -= diffcol;
    TraceEnumerator(Side,side,"");
    TraceValue("%u",Nr_remaining_moves[side]);
    TraceValue("%u\n",Nr_unused_masses[opponent]);
    result = true;
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Tests if a specific king move sequence is still possible.
 * @param side whose king to move
 * @param from_square from
 * @param to_square to
 * @return true iff the move sequence is still possible
 * @note modifies Nr_remaining_moves[side] if the move sequence is possible
 */
boolean intelligent_reserve_king_moves_from_to(Side side,
                                               square from_square,
                                               square to_square)
{
  boolean result;
  unsigned int nr_of_moves;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceSquare(from_square);
  TraceSquare(to_square);
  TraceFunctionParamListEnd();

  nr_of_moves = intelligent_count_nr_of_moves_from_to_king(side,from_square,to_square);

  if (nr_of_moves<=Nr_remaining_moves[side])
  {
    Nr_remaining_moves[side] -= nr_of_moves;
    TraceEnumerator(Side,side,"");
    TraceValue("%u\n",Nr_remaining_moves[side]);
    result = true;
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

unsigned int intelligent_count_moves_to_white_promotion(square from_square)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceSquare(from_square);
  TraceFunctionParamListEnd();

  if (MovesLeft[White]==5
      && from_square<=square_h2
      && (e[from_square+dir_up]>vide || e[from_square+2*dir_up]>vide))
    /* pawn can't reach the promotion square */
    result = maxply+1;
  else
  {
    unsigned int const rank = from_square/onerow - nr_of_slack_rows_below_board;
    result = 7-rank;

    if (result==6)
    {
      --result; /* double step! */

      if (MovesLeft[White]<=6)
      {
        /* immediate double step is required if this pawn is to promote */
        if (e[from_square+dir_up]==pn
            && (e[from_square+dir_left]<=roib
                && e[from_square+dir_right]<=roib))
          /* Black can't immediately get rid of block on 3th row
           * -> no immediate double step possible */
          ++result;

        else if (e[from_square+2*dir_up]==pn
                 && (e[from_square+dir_up+dir_left]<=roib
                     && e[from_square+dir_up+dir_right]<=roib
                     && ep[1]!=from_square+dir_up+dir_left
                     && ep[1]!=from_square+dir_up+dir_right))
          /* Black can't immediately get rid of block on 4th row
           * -> no immediate double step possible */
          ++result;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
