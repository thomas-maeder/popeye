#include "optimisations/goals/enpassant/filter.h"
#include "stipulation/stipulation.h"
#include "pydata.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/goals/prerequisite_guards.h"
#include "solving/en_passant.h"
#include "conditions/einstein/en_passant.h"
#include "debugging/trace.h"

#include <assert.h>

/* Allocate a STEnPassantFilter slice.
 * @return index of allocated slice
 */
slice_index alloc_enpassant_filter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STEnPassantFilter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type enpassant_filter_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (ep[nbply]!=initsquare || einstein_ep[nbply]!=initsquare)
    SETFLAG(goal_preprequisites_met[nbply],goal_ep);
  result = solve(next,n);
  CLRFLAG(goal_preprequisites_met[nbply],goal_ep);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
