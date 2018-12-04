#include "solving/goals/kiss.h"
#include "position/position.h"
#include "solving/move_effect_journal.h"
#include "position/effects/utils.h"
#include "solving/move_generator.h"
#include "solving/pipe.h"
#include "stipulation/goals/kiss/reached_tester.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* This module provides functionality dealing with slices that detect
 * whether an chess81 goal has just been reached
 */

static boolean is_naked(square candidate, square pos_kisser)
{
  int x;
  int y;

  for (x = dir_left; x<=dir_right; ++x)
    for (y = dir_down; y<=dir_up; y += onerow)
    {
      square const s1 = candidate+x+y;
      if (s1!=candidate && s1!=pos_kisser
          && !is_square_blocked(s1)
          && get_walk_of_piece_on_square(s1)!=Empty)
        return false;
    }

  return true;
}

static boolean is_kiss(void)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
  PieceIdType const moving_id = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
  square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                            moving_id,
                                                                            sq_arrival);
  int x;
  int y;

  for (x = dir_left; x<=dir_right; ++x)
    for (y = dir_down; y<=dir_up; y += onerow)
    {
      square const s = pos+x+y;
      if (s!=pos
          && !is_square_blocked(s)
          && get_walk_of_piece_on_square(s)!=Empty
          && GetPieceId(being_solved.spec[s])==id_to_be_kissed
          && is_naked(s,pos))
        return true;
    }

  return false;
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
void goal_kiss_reached_tester_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_this_move_doesnt_solve_if(si,!is_kiss());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
