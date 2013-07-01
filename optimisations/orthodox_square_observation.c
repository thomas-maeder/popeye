#include "optimisations/orthodox_square_observation.h"
#include "solving/en_passant.h"
#include "debugging/trace.h"
#include "pydata.h"

static boolean leapcheck_ortho(square sq_king,
                               vec_index_type kanf, vec_index_type kend,
                               PieNam p)
{
  vec_index_type k;
  for (k= kanf; k<=kend; k++)
  {
    square const sq_departure= sq_king+vec[k];
    if (get_walk_of_piece_on_square(sq_departure)==p
        && TSTFLAG(spec[sq_departure],trait[nbply]))
      return true;
  }

  return false;
}

static boolean roicheck_ortho(square sq_king)
{
  return leapcheck_ortho(sq_king, vec_queen_start,vec_queen_end, King);
}

static boolean cavcheck_ortho(square sq_king)
{
  return leapcheck_ortho(sq_king, vec_knight_start,vec_knight_end, Knight);
}

static boolean pawn_test_check_ortho(square sq_departure)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TracePiece(p);
  TraceFunctionParamListEnd();

  result = (get_walk_of_piece_on_square(sq_departure)==Pawn
            && TSTFLAG(spec[sq_departure],trait[nbply]));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean en_passant_test_check_ortho(square sq_target, numvec dir_capture)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_target);
  TracePiece(p);
  TraceFunctionParamListEnd();

  if (sq_target==en_passant_find_capturee())
  {
    square const sq_crossed0 = en_passant_multistep_over[0][parent_ply[nbply]];
    if (sq_crossed0!=initsquare)
    {
      if (pawn_test_check_ortho(sq_crossed0-dir_capture))
        result = true;
      else
      {
        square const sq_crossed1 = en_passant_multistep_over[1][parent_ply[nbply]];
        if (sq_crossed1!=initsquare
            && pawn_test_check_ortho(sq_crossed1-dir_capture))
          result = true;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean pioncheck_ortho(square sq_king)
{
  SquareFlags const capturable = trait[nbply]==White ? CapturableByWhPawnSq : CapturableByBlPawnSq;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_king);
  TraceFunctionParamListEnd();

  if (TSTFLAG(sq_spec[sq_king],capturable))
  {
    numvec const dir_forward = trait[nbply]==White ? dir_up : dir_down;
    numvec const dir_forward_right = dir_forward+dir_right;
    numvec const dir_forward_left = dir_forward+dir_left;

    if (pawn_test_check_ortho(sq_king-dir_forward_right))
      result = true;
    else if (pawn_test_check_ortho(sq_king-dir_forward_left))
      result = true;
    else if (en_passant_test_check_ortho(sq_king,dir_forward_right))
      result = true;
    else if (en_passant_test_check_ortho(sq_king,dir_forward_left))
      result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean is_square_observed_ortho(square sq_target, evalfunction_t *dummy_for_compatibility)
{
  Side const side_checking = trait[nbply];

  if (number_of_pieces[side_checking][King]>0
      && roicheck_ortho(sq_target))
    return true;

  if (number_of_pieces[side_checking][Pawn]>0
      && pioncheck_ortho(sq_target))
    return true;

  if (number_of_pieces[side_checking][Knight]>0
      && cavcheck_ortho(sq_target))
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
