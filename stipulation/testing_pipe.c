#include "stipulation/testing_pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "solving/solving.h"
#include "trace.h"

#include <assert.h>

/* Allocate a new testing pipe and make an existing pipe its successor
 * @param type which slice type
 * @return newly allocated slice
 */
slice_index alloc_testing_pipe(slice_type type)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParamListEnd();

  result = alloc_pipe(type);
  slices[result].u.fork.fork = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Traverse a subtree
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_children_testing_pipe(slice_index testing_pipe,
                                                   stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",testing_pipe);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(testing_pipe,st);

  if (slices[testing_pipe].u.fork.fork!=no_slice)
    stip_traverse_structure(slices[testing_pipe].u.fork.fork,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Callback to stip_spin_off_testers
 * Spin a tester slice off a testing pipe slice
 * @param si identifies the testing pipe slice
 * @param st address of structure representing traversal
 */
void stip_spin_off_testers_testing_pipe(slice_index si,
                                        stip_structure_traversal *st)
{
  spin_off_tester_state_type * const state = st->param;
  boolean const save_spinning_off = state->spinning_off;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* don't fall back on stip_spin_off_testers_pipe - testing pipes are not
   * needed in "testing mode", so just allocate a proxy placeholder */

  state->spun_off[si] = alloc_proxy_slice();

  state->spinning_off = true;
  stip_traverse_structure_children(si,st);
  state->spinning_off = save_spinning_off;

  link_to_branch(state->spun_off[si],state->spun_off[slices[si].u.fork.next]);
  slices[si].u.fork.fork = state->spun_off[si];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
