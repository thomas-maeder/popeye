#include "optimisations/orthodox_square_observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "pieces/walks/pawns/en_passant.h"
#include "position/position.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

static vec_index_type leapers_check_ortho(Side side_checking,
                                          square sq_king,
                                          vec_index_type kanf, vec_index_type kend,
                                          piece_walk_type p)
{
  vec_index_type result = 0;
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_checking);
  TraceSquare(sq_king);
  TraceWalk(p);
  TraceFunctionParamListEnd();

  for (k = kanf; k<=kend; k++)
  {
    square const sq_departure = sq_king+vec[k];
    if (get_walk_of_piece_on_square(sq_departure)==p
        && TSTFLAG(being_solved.spec[sq_departure],side_checking))
    {
      result = k;
      break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

vec_index_type king_check_ortho(Side side_checking, square sq_king)
{
  return leapers_check_ortho(side_checking,sq_king, vec_queen_start,vec_queen_end, King);
}

vec_index_type knight_check_ortho(Side side_checking, square sq_king)
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
            && TSTFLAG(being_solved.spec[sq_departure],side_checking));

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
    ply const ply_parent = parent_ply[nbply];
    unsigned int i;

    for (i = en_passant_top[ply_parent-1]; i<en_passant_top[ply_parent]; ++i)
    {
      square const sq_crossed = en_passant_multistep_over[i];
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

vec_index_type pawn_check_ortho(Side side_checking, square sq_king)
{
  SquareFlags const capturable = side_checking==White ? CapturableByWhPawnSq : CapturableByBlPawnSq;
  vec_index_type result = 0;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_checking);
  TraceSquare(sq_king);
  TraceFunctionParamListEnd();

  if (TSTFLAG(sq_spec(sq_king),capturable))
  {
    numvec const dir_forward = side_checking==White ? dir_up : dir_down;
    numvec const dir_forward_right = dir_forward+dir_right;
    numvec const dir_forward_left = dir_forward+dir_left;

    if (pawn_test_check_ortho(side_checking,sq_king-dir_forward_right))
      result = side_checking==White ? 8 : 5;
    else if (pawn_test_check_ortho(side_checking,sq_king-dir_forward_left))
      result = side_checking==White ? 7 : 6;
    else if (en_passant_test_check_ortho(side_checking,sq_king,dir_forward_right))
      result = side_checking==White ? 8 : 5;
    else if (en_passant_test_check_ortho(side_checking,sq_king,dir_forward_left))
      result = side_checking==White ? 7 : 6;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean is_square_observed_ortho(Side side_checking,
                                 square sq_target)
{
  if (being_solved.number_of_pieces[side_checking][King]>0
      && king_check_ortho(side_checking,sq_target))
    return true;

  if (being_solved.number_of_pieces[side_checking][Pawn]>0
      && pawn_check_ortho(side_checking,sq_target))
    return true;

  if (being_solved.number_of_pieces[side_checking][Knight]>0
      && knight_check_ortho(side_checking,sq_target))
    return true;

  if (being_solved.number_of_pieces[side_checking][Queen]>0
      || being_solved.number_of_pieces[side_checking][Rook]>0)
  {
    vec_index_type k;
    for (k= vec_rook_end; k>=vec_rook_start; k--)
    {
      square const sq_departure = find_end_of_line(sq_target,vec[k]);
      piece_walk_type const p = get_walk_of_piece_on_square(sq_departure);
      if ((p==Rook || p==Queen) && TSTFLAG(being_solved.spec[sq_departure],side_checking))
      {
        TraceWalk(p);TraceSquare(sq_departure);TraceEOL();
        return true;
      }
    }
  }

  if (being_solved.number_of_pieces[side_checking][Queen]>0
      || being_solved.number_of_pieces[side_checking][Bishop]>0)
  {
    vec_index_type k;
    for (k= vec_bishop_start; k<=vec_bishop_end; k++)
    {
      square const sq_departure = find_end_of_line(sq_target,vec[k]);
      piece_walk_type const p = get_walk_of_piece_on_square(sq_departure);
      if ((p==Bishop || p==Queen) && TSTFLAG(being_solved.spec[sq_departure],side_checking))
      {
        TraceWalk(p);TraceSquare(sq_departure);TraceEOL();
        return true;
      }
    }
  }

  {
    piece_walk_type const *pcheck;

    for (pcheck = checkpieces; *pcheck; ++pcheck)
      if (being_solved.number_of_pieces[side_checking][*pcheck]>0)
      {
        observing_walk[nbply] = *pcheck;
        if ((*checkfunctions[*pcheck])(EVALUATE(check)))
          return true;
      }
  }

  return false;
}

vec_index_type is_square_uninterceptably_observed_ortho(Side side_checking, square sq_target)
{
  vec_index_type result = 0;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_checking);
  TraceSquare(sq_target);
  TraceFunctionParamListEnd();


  if (being_solved.number_of_pieces[side_checking][King]>0)
    result = king_check_ortho(side_checking,sq_target);

  if (result==0 && being_solved.number_of_pieces[side_checking][Pawn]>0)
    result = pawn_check_ortho(side_checking,sq_target);

  if (result==0 && being_solved.number_of_pieces[side_checking][Knight]>0)
    result = knight_check_ortho(side_checking,sq_target);

  if (result==0 && being_solved.number_of_pieces[side_checking][Rook]>0)
    result = leapers_check_ortho(side_checking,sq_target, vec_rook_start,vec_rook_end, Rook);

  if (result==0 && being_solved.number_of_pieces[side_checking][Bishop]>0)
    result = leapers_check_ortho(side_checking,sq_target, vec_bishop_start,vec_bishop_end, Bishop);

  if (result==0 && being_solved.number_of_pieces[side_checking][Queen]>0)
    result = leapers_check_ortho(side_checking,sq_target, vec_queen_start,vec_queen_end, Queen);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

unsigned int count_interceptable_orthodox_checks(Side side_checking, square sq_target)
{
  unsigned int result = 0;

  /* for simplicity's sake, we also count checks from adjacent squares even if
   * these aren't interceptable; we trust in the caller to first invoke
   * is_square_uninterceptably_observed_ortho() so that this doesn't matter.
   */

  if (being_solved.number_of_pieces[side_checking][Queen]>0
      || being_solved.number_of_pieces[side_checking][Rook]>0)
  {
    vec_index_type k;
    for (k= vec_rook_end; k>=vec_rook_start; k--)
    {
      square const sq_departure = find_end_of_line(sq_target,vec[k]);
      piece_walk_type const p = get_walk_of_piece_on_square(sq_departure);
      if ((p==Rook || p==Queen) && TSTFLAG(being_solved.spec[sq_departure],side_checking))
        ++result;
    }
  }

  if (being_solved.number_of_pieces[side_checking][Queen]>0
      || being_solved.number_of_pieces[side_checking][Bishop]>0)
  {
    vec_index_type k;
    for (k= vec_bishop_start; k<=vec_bishop_end; k++)
    {
      square const sq_departure = find_end_of_line(sq_target,vec[k]);
      piece_walk_type const p = get_walk_of_piece_on_square(sq_departure);
      if ((p==Bishop || p==Queen) && TSTFLAG(being_solved.spec[sq_departure],side_checking))
        ++result;
    }
  }

  return result;
}
