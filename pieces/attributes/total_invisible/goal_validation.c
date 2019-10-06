#include "pieces/attributes/total_invisible/goal_validation.h"
#include "pieces/attributes/total_invisible/taboo.h"
#include "pieces/attributes/total_invisible/consumption.h"
#include "pieces/attributes/total_invisible/uninterceptable_check.h"
#include "pieces/attributes/total_invisible/attack_mating_piece.h"
#include "pieces/attributes/total_invisible.h"
#include "position/position.h"
#include "solving/machinery/solve.h"
#include "solving/has_solution_type.h"
#include "solving/pipe.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

mate_validation_type mate_validation_result;
mate_validation_type combined_validation_result;

static unsigned int find_nr_interceptors_needed(Side side_checking,
                                                square potential_flight,
                                                unsigned int nr_interceptors_available,
                                                vec_index_type start, vec_index_type end,
                                                piece_walk_type walk_rider)
{
  unsigned int result = 0;
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_checking);
  TraceSquare(potential_flight);
  TraceValue("%u",nr_placeable_invisibles_for_side);
  TraceValue("%u",start);
  TraceValue("%u",end);
  TraceWalk(walk_rider);
  TraceFunctionParamListEnd();

  for (k = start; k<=end && result<=nr_interceptors_available; ++k)
  {
    square const end = find_end_of_line(potential_flight,vec[k]);
    piece_walk_type const walk = get_walk_of_piece_on_square(end);
    Flags const flags = being_solved.spec[end];
    if ((walk==Queen || walk==walk_rider) && TSTFLAG(flags,side_checking))
    {
      square s;
      for (s = potential_flight+vec[k]; s!=end; s += vec[k])
        if (nr_taboos_accumulated_until_ply[White][s]==0 || nr_taboos_accumulated_until_ply[Black][s]==0)
        {
          ++result;
          break;
        }

      if (s==end)
      {
        /* line can't be intercepted - all guard are not interceptable */
        result = nr_interceptors_available+1;
        break;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean are_all_guards_interceptable(Side side_in_check, square potential_flight)
{
  Side const side_checking = advers(side_in_check);
  unsigned int nr_available;
  unsigned int nr_interceptors_needed;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_in_check);
  TraceSquare(potential_flight);
  TraceFunctionParamListEnd();

  nr_available = nr_placeable_invisibles_for_side(side_in_check);
  TraceConsumption();
  TraceValue("%u",nr_available);
  TraceEOL();

  nr_interceptors_needed = find_nr_interceptors_needed(side_checking,
                                                       potential_flight,
                                                       nr_available,
                                                       vec_rook_start,vec_rook_end,
                                                       Rook);

  if (nr_interceptors_needed<=nr_available)
    nr_interceptors_needed += find_nr_interceptors_needed(side_checking,
                                                          potential_flight,
                                                          nr_available-nr_interceptors_needed,
                                                          vec_bishop_start,vec_bishop_end,
                                                          Bishop);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",(nr_interceptors_needed<=nr_available));
  TraceFunctionResultEnd();
  return nr_interceptors_needed<=nr_available;
}

static boolean make_flight(Side side_in_check, square s)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_in_check);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  if ((is_square_empty(s)
      || TSTFLAG(being_solved.spec[s],advers(side_in_check)))
      && !is_square_uninterceptably_attacked(side_in_check,s)
      && are_all_guards_interceptable(side_in_check,s))
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean make_a_flight(void)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    Side const side_in_check = advers(trait[nbply]);
    square const king_pos = being_solved.king_square[side_in_check];
    int dir_vert;
    int dir_horiz;

    piece_walk_type const walk = get_walk_of_piece_on_square(king_pos);
    Flags const flags = being_solved.spec[king_pos];

    TraceWalk(walk);
    TraceValue("%x",flags);
    TraceEOL();

    empty_square(king_pos);

    for (dir_vert = dir_down; dir_vert<=dir_up && !result; dir_vert += dir_up)
      for (dir_horiz = dir_left; dir_horiz<=dir_right; dir_horiz += dir_right)
      {
        square const flight = king_pos+dir_vert+dir_horiz;
        if (flight!=king_pos && make_flight(side_in_check,flight))
        {
          result = true;
          break;
        }
      }

    occupy_square(king_pos,walk,flags);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void attack_checks(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    Side const side_delivering_check = trait[nbply];
    Side const side_in_check = advers(side_delivering_check);
    square const king_pos = being_solved.king_square[side_in_check];
    vec_index_type const k = is_square_uninterceptably_attacked(side_in_check,king_pos);
    if (k==0)
    {
      if (are_all_guards_interceptable(side_in_check,king_pos))
      {
        TraceText("interceptable check\n");
        /* the king square can be made a "flight" */
        mate_validation_result = no_mate;
        solve_result = previous_move_has_not_solved;
      }
      else
      {
        TraceText("mate can be refuted by interceptors or free TIs (if any)\n");
        mate_validation_result = mate_defendable_by_interceptors;
      }
    }
    else if (k==UINT_MAX)
    {
      TraceText("mate can be refuted by interceptors - no free TI avaliable\n");
      mate_validation_result = mate_defendable_by_interceptors;
    }
    else if (nr_placeable_invisibles_for_side(side_in_check)>0)
    {
      square const sq_attacker = find_end_of_line(king_pos,vec[k]);
      TraceSquare(king_pos);TraceValue("%u",k);TraceValue("%d",vec[k]);TraceSquare(sq_attacker);TraceEOL();
      assert(TSTFLAG(being_solved.spec[sq_attacker],side_delivering_check));
      CLRFLAG(being_solved.spec[sq_attacker],side_delivering_check);
      if (is_square_uninterceptably_attacked(side_in_check,king_pos))
      {
        TraceText("mate can not be defended\n");
        mate_validation_result = mate_with_2_uninterceptable_doublechecks;
      }
      else
      {
        TraceText("mate can be refuted by free TIs\n");
        mate_validation_result = mate_attackable;
        sq_mating_piece_to_be_attacked = sq_attacker;
      }
      SETFLAG(being_solved.spec[sq_attacker],side_delivering_check);
    }
    else
    {
      TraceText("mate can be refuted by interceptors - no free TI avaliable\n");
      mate_validation_result = mate_defendable_by_interceptors;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
void total_invisible_goal_guard_solve(slice_index si)
{
  play_phase_type const save_play_phase = play_phase;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* make sure that we don't generate pawn captures total invisible */
  assert(play_phase==play_replay_validating || play_phase==play_replay_testing);

  play_phase = play_testing_goal;
  pipe_solve_delegate(si);
  play_phase = save_play_phase;

  if (play_phase==play_replay_validating)
  {
    if (solve_result==previous_move_has_not_solved)
      mate_validation_result = no_mate;
    else if (make_a_flight())
    {
      solve_result = previous_move_has_not_solved;
      mate_validation_result = no_mate;
    }
    else
      attack_checks();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
