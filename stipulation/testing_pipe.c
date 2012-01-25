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
  slices[result].u.testing_pipe.tester = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Traverse a subtree
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_testing_pipe(slice_index testing_pipe,
                                          stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",testing_pipe);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(testing_pipe,st);

  if (slices[testing_pipe].u.testing_pipe.tester!=no_slice)
    stip_traverse_structure(slices[testing_pipe].u.testing_pipe.tester,st);

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

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* don't fall back on stip_spin_off_testers_pipe - testing pipes are not
   * needed in "testing mode", so just allocate a proxy placeholder */

  state->spun_off[si] = alloc_proxy_slice();

  stip_traverse_structure_children(si,st);

  link_to_branch(state->spun_off[si],state->spun_off[slices[si].u.testing_pipe.next]);
  slices[si].u.testing_pipe.tester = state->spun_off[si];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Substitute links to proxy slices by the proxy's target
 * @param si slice where to resolve proxies
 * @param st points at the structure holding the state of the traversal
 */
void testing_pipe_resolve_proxies(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_resolve_proxies(si,st);

  if (slices[si].u.testing_pipe.tester!=no_slice)
    proxy_slice_resolve(&slices[si].u.testing_pipe.tester,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
