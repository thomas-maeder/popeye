#include "conditions/ultraschachzwang/legality_tester.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"

#include <assert.h>

/* Allocate a STUltraschachzwangLegalityTester slice.
 * @return index of allocated slice
 */
slice_index alloc_ultraschachzwang_legality_tester_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STUltraschachzwangLegalityTester);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type ultraschachzwang_legality_tester_solve(slice_index si,
                                                         stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if ((CondFlag[blackultraschachzwang]
       && trait[nbply]==Black
       && !echecc(White))
      || (CondFlag[whiteultraschachzwang]
          && trait[nbply]==White
          && !echecc(Black)))
    result = slack_length-2;
  else
    result = solve(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
