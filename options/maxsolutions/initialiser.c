#include "options/maxsolutions/initialiser.h"
#include "options/maxsolutions/maxsolutions.h"
#include "pypipe.h"
#include "trace.h"

/* Allocate a STMaxSolutionsInitialiser slice.
 * @return allocated slice
 */
slice_index alloc_maxsolutions_initialiser_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STMaxSolutionsInitialiser);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Spin off set play
 * @param si slice index
 * @param st state of traversal
 */
void maxsolutions_initialiser_apply_setplay(slice_index si,
                                            stip_structure_traversal *st)
{
  spin_off_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  TraceValue("%u\n",state->spun_off[slices[si].u.pipe.next]);

  if (state->spun_off[slices[si].u.pipe.next]!=no_slice)
  {
    state->spun_off[si] = copy_slice(si);
    pipe_link(state->spun_off[si],state->spun_off[slices[si].u.pipe.next]);
  }

  TraceValue("%u\n",state->spun_off[si]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type maxsolutions_initialiser_solve(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  reset_nr_found_solutions_per_phase();

  result = slice_solve(slices[si].u.pipe.next);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
