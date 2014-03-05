#include "stipulation/goals/kiss/reached_tester.h"
#include "solving/move_effect_journal.h"
#include "solving/move_generator.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

static PieceIdType id_to_be_kissed;

/* This module provides functionality dealing with slices that detect
 * whether an chess81 goal has just been reached
 */

/* Allocate a system of slices that tests whether first row or last row has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_kiss_reached_tester_system(square s)
{
  slice_index result;
  slice_index kiss_tester;
  Goal const goal = { goal_kiss, s };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  kiss_tester = alloc_pipe(STGoalKissReachedTester);
  pipe_link(kiss_tester,alloc_true_slice());
  result = alloc_goal_reached_tester_slice(goal,kiss_tester);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void remember_id(slice_index si, stip_structure_traversal *st)
{
  Goal const goal = slices[si].u.goal_handler.goal;
  if (goal.type==goal_kiss)
    id_to_be_kissed = GetPieceId(spec[goal.target]);

  stip_traverse_structure_children(si,st);
}

/* Remember the id of the piece to be kissed
 * @param si root of the solving machinery
 */
void goal_kiss_init_piece_id(slice_index si)
{
  stip_structure_traversal st;
  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,STGoalReachedTester,&remember_id);
  stip_traverse_structure(si,&st);
}

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
          && GetPieceId(spec[s])==id_to_be_kissed
          && is_naked(s,pos))
        return true;
    }

  return false;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type goal_kiss_reached_tester_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (is_kiss())
    result = solve(slices[si].next1,n);
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
