#include "pieces/attributes/total_invisible/uninterceptable_check.h"
#include "pieces/attributes/total_invisible/taboo.h"
#include "pieces/attributes/total_invisible/consumption.h"
#include "pieces/attributes/total_invisible/revelations.h"
#include "solving/machinery/solve.h"
#include "solving/has_solution_type.h"
#include "solving/pipe.h"
#include "optimisations/orthodox_square_observation.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

#include <limits.h>

ply uninterceptable_check_delivered_in_ply = ply_nil;
square uninterceptable_check_delivered_from = initsquare;

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
           && (is_taboo(s,side_checking) || was_taboo(s,side_checking) || will_be_taboo(s,side_checking)))
      s += vec[k];

    {
      piece_walk_type const walk = get_walk_of_piece_on_square(s);
      result = ((walk==rider_walk || walk==Queen)
                && TSTFLAG(being_solved.spec[s],side_checking));
    }
    TraceSquare(s);
    TraceValue("%u",is_square_empty(s));
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

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void total_invisible_uninterceptable_selfcheck_guard_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  top_ply_of_regular_play = nbply;

  // TODO separate slice type for update nr_taboos_accumulated_until_ply?
  if (is_square_uninterceptably_attacked(trait[nbply],being_solved.king_square[trait[nbply]]))
    solve_result = previous_move_is_illegal;
  else if (nbply>ply_retro_move)
  {
    remember_taboos_for_current_move();

    top_before_revelations[nbply] = move_effect_journal_base[nbply+1];

    make_revelations();

    if (revelation_status_is_uninitialised)
      pipe_solve_delegate(si);
    else
    {
      // TODO move to separate slice implemented in the revelations module

      unsigned int nr_revealed_unplaced_invisibles[nr_sides] = { 0, 0 };

      {
        unsigned int i;
        for (i = 0; i!=nr_potential_revelations; ++i)
        {
          square const s = revelation_status[i].first_on;
          TraceValue("%u",i);
          TraceSquare(s);
          TraceWalk(revelation_status[i].walk);
          TraceValue("%x",revelation_status[i].spec);
          TraceEOL();
          if (revelation_status[i].walk!=Empty && is_square_empty(s))
          {
            Side const side = TSTFLAG(revelation_status[i].spec,White) ? White : Black;
            ++nr_revealed_unplaced_invisibles[side];
          }
        }
      }

      // TODO there must be a simpler, understandable way to do this
      if (current_consumption.claimed[White] && nr_revealed_unplaced_invisibles[White]>0)
        --nr_revealed_unplaced_invisibles[White];
      if (current_consumption.claimed[Black] && nr_revealed_unplaced_invisibles[Black]>0)
        --nr_revealed_unplaced_invisibles[Black];
      if ((current_consumption.fleshed_out[White]
           +current_consumption.fleshed_out[Black]
           +current_consumption.placed[White]
           +current_consumption.placed[Black]
           +nr_revealed_unplaced_invisibles[White]
           +nr_revealed_unplaced_invisibles[Black])
          <=total_invisible_number)
        evaluate_revelations(si,nr_potential_revelations);
    }

    forget_taboos_for_current_move();
  }
  else
    pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
