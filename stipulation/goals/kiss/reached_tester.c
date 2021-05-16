#include "stipulation/goals/kiss/reached_tester.h"
#include "position/position.h"
#include "position/pieceid.h"
#include "stipulation/pipe.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

PieceIdType id_to_be_kissed;

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
  Goal const goal = SLICE_U(si).goal_handler.goal;
  if (goal.type==goal_kiss)
    id_to_be_kissed = GetPieceId(being_solved.spec[goal.target]);

  stip_traverse_structure_children(si,st);
}

/* Remember the id of the piece to be kissed
 * @param si root of the solving machinery
 */
void goal_kiss_init_piece_id(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,STGoalReachedTester,&remember_id);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
