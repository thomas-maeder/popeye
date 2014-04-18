#include "pieces/attributes/paralysing/mate_filter.h"
#include "stipulation/pipe.h"
#include "solving/has_solution_type.h"
#include "solving/pipe.h"
#include "pieces/attributes/paralysing/paralysing.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* This module provides slice type STPiecesParalysingMateFilter
 */

/* Allocate a STPiecesParalysingMateFilter slice.
 * @param starter_or_adversary is the starter mated or its adversary?
 * @return index of allocated slice
 */
slice_index
alloc_paralysing_mate_filter_slice(goal_applies_to_starter_or_adversary starter_or_adversary)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceValue("%u",starter_or_adversary);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STPiecesParalysingMateFilter);
  slices[result].u.goal_filter.applies_to_who = starter_or_adversary;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STPiecesParalysingMateFilterTester slice.
 * @param starter_or_adversary is the starter mated or its adversary?
 * @return index of allocated slice
 */
slice_index
alloc_paralysing_mate_filter_tester_slice(goal_applies_to_starter_or_adversary starter_or_adversary)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceValue("%u",starter_or_adversary);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STPiecesParalysingMateFilterTester);
  slices[result].u.goal_filter.applies_to_who = starter_or_adversary;

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
void paralysing_mate_filter_tester_solve(slice_index si)
{
  Side const mated = (slices[si].u.goal_filter.applies_to_who
                      ==goal_applies_to_starter
                      ? slices[si].starter
                      : advers(slices[si].starter));

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_solve_delegate(si);

  if (move_has_solved() && suffocated_by_paralysis(mated))
    solve_result = MOVE_HAS_NOT_SOLVED_LENGTH();

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
void paralysing_mate_filter_solve(slice_index si)
{
  Side const mated = (slices[si].u.goal_filter.applies_to_who
                      ==goal_applies_to_starter
                      ? slices[si].starter
                      : advers(slices[si].starter));

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_this_move_doesnt_solve_if(si,suffocated_by_paralysis(mated));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
