#include "solving/incomplete.h"
#include "stipulation/branch.h"
#include "stipulation/pipe.h"
#include "solving/pipe.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

/* Reset our state before delegating, then be ready to report our state
 * @param si identifies the STProblemSolvingInterrupted slice
 */
void problem_solving_incomplete_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  SLICE_U(si).value_handler.value = solving_complete;

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Report whether solving has been interrupted
 * @param si identifies the STProblemSolvingInterrupted slice
 * @return completeness of solution
 */
solving_completeness_type problem_solving_completeness(slice_index si)
{
  solving_completeness_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(SLICE_TYPE(si)==STProblemSolvingInterrupted);
  result = SLICE_U(si).flag_handler.value;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STPhaseSolvingInterrupted slice
 * @param base base for searching for the STProblemSolvingInterrupted slice
 *             that the result will propagate the information about
 *             interruptions to.
 * @return identiifer of the allocates slice
 */
slice_index alloc_phase_solving_incomplete(slice_index base)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",base);
  TraceFunctionParamListEnd();

  {
    slice_index const problem = branch_find_slice(STProblemSolvingInterrupted,
                                                  base,
                                                  stip_traversal_context_intro);
    result = alloc_pipe(STPhaseSolvingInterrupted);
    assert(problem!=no_slice);
    SLICE_NEXT2(result) = problem;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Reset our state before delegating, then be ready to report our state
 * @param si identifies the STProblemSolvingInterrupted slice
 */
void phase_solving_incomplete_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  SLICE_U(si).flag_handler.value = solving_complete;

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
/* Remember that solving has been interrupted
 * @param si identifies the STProblemSolvingInterrupted slice
 * @param c completeness of phase
 */
void phase_solving_remember_incompleteness(slice_index si,
                                           solving_completeness_type c)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%d",c);
  TraceFunctionParamListEnd();

  assert(SLICE_TYPE(si)==STPhaseSolvingInterrupted);

  if (SLICE_U(si).flag_handler.value<c)
    SLICE_U(si).flag_handler.value = c;

  if (SLICE_U(SLICE_NEXT2(si)).flag_handler.value<c)
    SLICE_U(SLICE_NEXT2(si)).flag_handler.value = c;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Report whether solving has been interrupted
 * @param si identifies the STProblemSolvingInterrupted slice
 * @return completeness of solution
 */
solving_completeness_type phase_solving_completeness(slice_index si)
{
  solving_completeness_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(SLICE_TYPE(si)==STPhaseSolvingInterrupted);
  result = SLICE_U(si).flag_handler.value;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
