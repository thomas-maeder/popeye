#include "optimisations/orthodox_square_observation.h"
#include "pieces/walks/pawns/en_passant.h"
#include "debugging/trace.h"
#include "pydata.h"

static boolean leapers_check_ortho(Side side_checking,
                                   square sq_king,
                                   vec_index_type kanf, vec_index_type kend,
                                   PieNam p)
{
  vec_index_type k;
  for (k = kanf; k<=kend; k++)
  {
    square const sq_departure = sq_king+vec[k];
    if (get_walk_of_piece_on_square(sq_departure)==p
        && TSTFLAG(spec[sq_departure],side_checking))
      return true;
  }

  return false;
}

static boolean king_check_ortho(Side side_checking, square sq_king)
{
  return leapers_check_ortho(side_checking,sq_king, vec_queen_start,vec_queen_end, King);
}

static boolean knight_check_ortho(Side side_checking, square sq_king)
{
  return leapers_check_ortho(side_checking,sq_king, vec_knight_start,vec_knight_end, Knight);
}

static boolean pawn_test_check_ortho(Side side_checking, square sq_departure)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  result = (get_walk_of_piece_on_square(sq_departure)==Pawn
            && TSTFLAG(spec[sq_departure],side_checking));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean en_passant_test_check_ortho(Side side_checking,
                                           square sq_target,
                                           numvec dir_capture)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_target);
  TraceFunctionParamListEnd();

  if (sq_target==en_passant_find_capturee())
  {
    unsigned int i;

    for (i = 0; i!=en_passant_max_nr_multistep_over; ++i)
    {
      square const sq_crossed = en_passant_multistep_over[i][parent_ply[nbply]];
      if (sq_crossed!=initsquare
          && pawn_test_check_ortho(side_checking,sq_crossed-dir_capture))
      {
        result = true;
        break;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean pawn_check_ortho(Side side_checking, square sq_king)
{
  SquareFlags const capturable = side_checking==White ? CapturableByWhPawnSq : CapturableByBlPawnSq;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceSquare(sq_king);
  TraceFunctionParamListEnd();

  if (TSTFLAG(sq_spec[sq_king],capturable))
  {
    numvec const dir_forward = side_checking==White ? dir_up : dir_down;
    numvec const dir_forward_right = dir_forward+dir_right;
    numvec const dir_forward_left = dir_forward+dir_left;

    if (pawn_test_check_ortho(side_checking,sq_king-dir_forward_right))
      result = true;
    else if (pawn_test_check_ortho(side_checking,sq_king-dir_forward_left))
      result = true;
    else if (en_passant_test_check_ortho(side_checking,sq_king,dir_forward_right))
      result = true;
    else if (en_passant_test_check_ortho(side_checking,sq_king,dir_forward_left))
      result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean is_square_observed_ortho(Side side_checking,
                                 square sq_target)
{
  if (number_of_pieces[side_checking][King]>0
      && king_check_ortho(side_checking,sq_target))
    return true;

  if (number_of_pieces[side_checking][Pawn]>0
      && pawn_check_ortho(side_checking,sq_target))
    return true;

  if (number_of_pieces[side_checking][Knight]>0
      && knight_check_ortho(side_checking,sq_target))
    return true;

  if (number_of_pieces[side_checking][Queen]>0
      || number_of_pieces[side_checking][Rook]>0)
  {
    vec_index_type k;
    for (k= vec_rook_end; k>=vec_rook_start; k--)
    {
      square const sq_departure = find_end_of_line(sq_target,vec[k]);
      PieNam const p = get_walk_of_piece_on_square(sq_departure);
      if ((p==Rook || p==Queen) && TSTFLAG(spec[sq_departure],side_checking))
        return true;
    }
  }

  if (number_of_pieces[side_checking][Queen]>0
      || number_of_pieces[side_checking][Bishop]>0)
  {
    vec_index_type k;
    for (k= vec_bishop_start; k<=vec_bishop_end; k++)
    {
      square const sq_departure = find_end_of_line(sq_target,vec[k]);
      PieNam const p = get_walk_of_piece_on_square(sq_departure);
      if ((p==Bishop || p==Queen) && TSTFLAG(spec[sq_departure],side_checking))
        return true;
    }
  }

  return false;
}

boolean is_square_uninterceptably_observed_ortho(Side side_checking, square sq_target)
{
  if (number_of_pieces[side_checking][Pawn]>0
      && pawn_check_ortho(side_checking,sq_target))
    return true;

  if (number_of_pieces[side_checking][Knight]>0
      && knight_check_ortho(side_checking,sq_target))
    return true;

  if (number_of_pieces[side_checking][Rook]>0
      && leapers_check_ortho(side_checking,sq_target, vec_rook_start,vec_rook_end, Rook))
    return true;

  if (number_of_pieces[side_checking][Bishop]>0
      && leapers_check_ortho(side_checking,sq_target, vec_bishop_start,vec_bishop_end, Bishop))
    return true;

  if (number_of_pieces[side_checking][Queen]>0
      && leapers_check_ortho(side_checking,sq_target, vec_queen_start,vec_queen_end, Queen))
    return true;

  return false;
}
