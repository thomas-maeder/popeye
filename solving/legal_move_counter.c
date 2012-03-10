#include "solving/legal_move_counter.h"
#include "pypipe.h"
#include "pydata.h"
#include "trace.h"

#include <assert.h>

/* current value of the count */
unsigned int legal_move_counter_count[maxply];

/* stop the move iteration once legal_move_counter_count exceeds this number */
unsigned int legal_move_counter_interesting[maxply];

/* Allocate a STLegalMoveCounter slice.
 * @return index of allocated slice
 */
slice_index alloc_legal_move_counter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STLegalMoveCounter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STAnyMoveCounter slice.
 * @return index of allocated slice
 */
slice_index alloc_any_move_counter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STAnyMoveCounter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type legal_move_counter_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  ++legal_move_counter_count[nbply];

  if (legal_move_counter_count[nbply]<=legal_move_counter_interesting[nbply])
    result = n+2;
  else
    /* stop the iteration */
    result = n;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type legal_move_counter_solve(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  ++legal_move_counter_count[nbply];

  if (legal_move_counter_count[nbply]<=legal_move_counter_interesting[nbply])
    result = has_no_solution;
  else
    /* stop the iteration */
    result = has_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
