#include "options/interruption.h"
#include "stipulation/branch.h"
#include "stipulation/pipe.h"
#include "solving/pipe.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

/* Reset our state before delegating, then be ready to report our state
 * @param si identifies the STProblemSolvingInterrupted slice
 */
void problem_solving_interrupted_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  SLICE_U(si).flag_handler.value = false;

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Remember that solving has been interrupted
 * @param si identifies the STProblemSolvingInterrupted slice
 */
void problem_solving_remember_interruption(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(SLICE_TYPE(si)==STProblemSolvingInterrupted);
  SLICE_U(si).flag_handler.value = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Report whether solving has been interrupted
 * @param si identifies the STProblemSolvingInterrupted slice
 * @return true iff solving has been interrupted
 */
boolean problem_solving_is_interrupted(slice_index si)
{
  boolean result;

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
slice_index alloc_phase_solving_interrupted(slice_index base)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
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
void phase_solving_interrupted_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  SLICE_U(si).flag_handler.value = false;

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Remember that solving has been interrupted
 * @param si identifies the STProblemSolvingInterrupted slice
 */
void phase_solving_remember_interruption(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(SLICE_TYPE(si)==STPhaseSolvingInterrupted);
  SLICE_U(si).flag_handler.value = true;
  SLICE_U(SLICE_NEXT2(si)).flag_handler.value = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Report whether solving has been interrupted
 * @param si identifies the STProblemSolvingInterrupted slice
 * @return true iff solving has been interrupted
 */
boolean phase_solving_is_interrupted(slice_index si)
{
  boolean result;

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
