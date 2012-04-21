#include "stipulation/testing_pipe.h"
#include "pystip.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "solving/solving.h"
#include "debugging/trace.h"

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
  slices[result].next2 = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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

  link_to_branch(state->spun_off[si],state->spun_off[slices[si].next1]);
  slices[si].next2 = state->spun_off[si];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
