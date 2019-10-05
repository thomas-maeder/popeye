#include "pieces/attributes/total_invisible/uninterceptable_check.h"
#include "pieces/attributes/total_invisible/taboo.h"
#include "pieces/attributes/total_invisible/consumption.h"
#include "optimisations/orthodox_square_observation.h"
#include "debugging/trace.h"

#include <limits.h>

static boolean is_rider_check_uninterceptable_on_vector(Side side_checking, square king_pos,
                                                        vec_index_type k, piece_walk_type rider_walk)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_checking);
  TraceSquare(king_pos);
  TraceValue("%u",k);
  TraceWalk(rider_walk);
  TraceFunctionParamListEnd();

  {
    square s = king_pos+vec[k];
    while (is_square_empty(s)
           && (was_taboo(s) || (is_taboo(s,White) && is_taboo(s,Black))))
      s += vec[k];

    {
      piece_walk_type const walk = get_walk_of_piece_on_square(s);
      result = ((walk==rider_walk || walk==Queen)
                && TSTFLAG(being_solved.spec[s],side_checking));
    }
    TraceSquare(s);
    TraceValue("%u",is_square_empty(s));
    TraceValue("%u",nr_taboos_accumulated_until_ply[White][s]);
    TraceValue("%u",nr_taboos_accumulated_until_ply[Black][s]);
    TraceEOL();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

vec_index_type is_rider_check_uninterceptable(Side side_checking, square king_pos,
                                              vec_index_type kanf, vec_index_type kend, piece_walk_type rider_walk)
{
  vec_index_type result = 0;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_checking);
  TraceSquare(king_pos);
  TraceValue("%u",kanf);
  TraceValue("%u",kend);
  TraceWalk(rider_walk);
  TraceFunctionParamListEnd();

  {
    vec_index_type k;
    for (k = kanf; !result && k<=kend; k++)
      if (is_rider_check_uninterceptable_on_vector(side_checking,king_pos,k,rider_walk))
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

static boolean can_interceptor_be_allocated(void)
{
  dynamic_consumption_type const save_consumption = current_consumption;
  boolean result = allocate_flesh_out_unplaced(White);
  current_consumption = save_consumption;
  result = result || allocate_flesh_out_unplaced(Black);
  current_consumption = save_consumption;
  return result;
}

vec_index_type is_square_uninterceptably_attacked(Side side_under_attack, square sq_attacked)
{
  Side const side_checking = advers(side_under_attack);
  vec_index_type result = 0;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_under_attack);
  TraceSquare(sq_attacked);
  TraceFunctionParamListEnd();

  if (can_interceptor_be_allocated())
  {
    // TODO shouldn't we compare sq_attacked to initsquare?
    if (being_solved.king_square[side_under_attack]!=initsquare)
    {
      if (!result && being_solved.number_of_pieces[side_checking][King]>0)
        result = king_check_ortho(side_checking,sq_attacked);

      if (!result && being_solved.number_of_pieces[side_checking][Pawn]>0)
        result = pawn_check_ortho(side_checking,sq_attacked);

      if (!result && being_solved.number_of_pieces[side_checking][Knight]>0)
        result = knight_check_ortho(side_checking,sq_attacked);

      if (!result && being_solved.number_of_pieces[side_checking][Rook]+being_solved.number_of_pieces[side_checking][Queen]>0)
        result = is_rider_check_uninterceptable(side_checking,sq_attacked, vec_rook_start,vec_rook_end, Rook);

      if (!result && being_solved.number_of_pieces[side_checking][Bishop]+being_solved.number_of_pieces[side_checking][Queen]>0)
        result = is_rider_check_uninterceptable(side_checking,sq_attacked, vec_bishop_start,vec_bishop_end, Bishop);
    }
  }
  else
  {
    if (is_square_observed_ortho(side_checking,sq_attacked))
      result = UINT_MAX;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

vec_index_type is_square_attacked_by_uninterceptable(Side side_under_attack, square sq_attacked)
{
  vec_index_type result = 0;
  Side const side_checking = advers(side_under_attack);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_under_attack);
  TraceSquare(sq_attacked);
  TraceFunctionParamListEnd();

  if (being_solved.king_square[side_under_attack]!=initsquare)
  {
    if (!result && being_solved.number_of_pieces[side_checking][King]>0)
      result = king_check_ortho(side_checking,sq_attacked);

    if (!result && being_solved.number_of_pieces[side_checking][Pawn]>0)
      result = pawn_check_ortho(side_checking,sq_attacked);

    if (!result && being_solved.number_of_pieces[side_checking][Knight]>0)
      result = knight_check_ortho(side_checking,sq_attacked);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
