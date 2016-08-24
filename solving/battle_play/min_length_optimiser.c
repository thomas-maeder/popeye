#include "solving/battle_play/min_length_optimiser.h"
#include "stipulation/stipulation.h"
#include "stipulation/branch.h"
#include "solving/avoid_unsolvable.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Allocate a STMinLengthOptimiser defender slice.
 * @return index of allocated slice
 */
slice_index alloc_min_length_optimiser_slice(stip_length_type length,
                                             stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STMinLengthOptimiser,length,min_length);

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
void min_length_optimiser_solve(slice_index si)
{
  slice_index const length = SLICE_U(si).branch.length;
  slice_index const min_length = SLICE_U(si).branch.min_length;
  slice_index const save_max_unsolvable = max_unsolvable;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",max_unsolvable);
  TraceEOL();
  if (max_unsolvable+length-min_length<solve_nr_remaining)
  {
    max_unsolvable = solve_nr_remaining-1-(length-min_length);
    TraceValue("->%u",max_unsolvable);
    TraceEOL();
  }

  pipe_solve_delegate(si);

  max_unsolvable = save_max_unsolvable;
  TraceValue("->%u",max_unsolvable);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
