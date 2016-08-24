#include "stipulation/testing_pipe.h"
#include "stipulation/stipulation.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Allocate a new testing pipe and make an existing pipe its successor
 * @param type which slice type
 * @return newly allocated slice
 */
slice_index alloc_testing_pipe(slice_type type)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type);
  TraceFunctionParamListEnd();

  result = alloc_pipe(type);
  SLICE_NEXT2(result) = no_slice;

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
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* don't fall back on stip_spin_off_testers_pipe - testing pipes are not
   * needed in "testing mode", so just allocate a proxy placeholder */

  SLICE_TESTER(si) = alloc_proxy_slice();

  stip_traverse_structure_children(si,st);

  link_to_branch(SLICE_TESTER(si),SLICE_TESTER(SLICE_NEXT1(si)));
  SLICE_NEXT2(si) = SLICE_TESTER(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
