#include "stipulation/goals/immobile/reached_tester.h"
#include "stipulation/proxy.h"
#include "stipulation/boolean/and.h"
#include "stipulation/goals/immobile/reached_tester_king.h"
#include "stipulation/goals/immobile/reached_tester_non_king.h"
#include "trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether a side is immobile
 */

static void substitute_king_first(slice_index si, stip_structure_traversal *st)
{
  stip_traverse_structure_children(si,st);

  {
    slice_index const proxy1 = alloc_proxy_slice();
    slice_index const proxy2 = alloc_proxy_slice();
    slice_index const next = slices[si].u.pipe.next;

    pipe_link(si,alloc_and_slice(proxy1,proxy2));
    pipe_link(proxy2,make_immobility_tester_non_king(stip_deep_copy(next)));
    pipe_link(proxy1,make_immobility_tester_king(next));

    pipe_remove(si);
  }
}

/* Replace immobility tester slices' type
 * @param si where to start (entry slice into stipulation)
 */
void immobility_testers_substitute_king_first(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STImmobilityTester,
                                           &substitute_king_first);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
