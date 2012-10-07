#include "stipulation/goals/doublemate/king_capture_avoider.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/help_play/branch.h"
#include "debugging/trace.h"

#include <assert.h>

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  boolean const * const behind_goal_with_potential_king_capture = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (*behind_goal_with_potential_king_capture)
  {
    slice_index const prototype = alloc_pipe(STKingCaptureAvoider);
    assert(st->context==stip_traversal_context_help);
    help_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_goal_with_potential_king_capture(slice_index si,
                                                      stip_structure_traversal *st)
{
  boolean * const behind_goal_with_potential_king_capture = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *behind_goal_with_potential_king_capture = true;
  stip_traverse_structure_children(si,st);
  *behind_goal_with_potential_king_capture = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_king_capture_avoiders(slice_index si)
{
  boolean behind_goal_with_potential_king_capture = false;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&behind_goal_with_potential_king_capture);
  stip_structure_traversal_override_single(&st,
                                           STMove,
                                           &instrument_move);
  stip_structure_traversal_override_single(&st,
                                           STGoalCounterMateReachedTester,
                                           &remember_goal_with_potential_king_capture);
  stip_structure_traversal_override_single(&st,
                                           STGoalDoubleMateReachedTester,
                                           &remember_goal_with_potential_king_capture);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type king_capture_avoider_solve(slice_index si,
                                             stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (king_square[slices[si].starter]==initsquare)
    result = slack_length-2;
  else
    result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
