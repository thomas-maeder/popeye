#include "conditions/circe/circe.h"
#include "pypipe.h"
#include "pyquodli.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "conditions/circe/circuit_b_special.h"
#include "conditions/circe/steingewinn_filter.h"
#include "trace.h"

#include <assert.h>


static void prepend_goal_filters(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  switch (slices[si].u.goal_reached_tester.goal.type)
  {
    case goal_steingewinn:
      pipe_append(slices[si].prev,alloc_circe_steingewinn_filter_slice());
      break;

    case goal_circuitB:
    {
      slice_index const tested = branch_find_slice(STGoalReachedTested,si);
      slice_index const proxy_special = alloc_proxy_slice();
      slice_index const special = alloc_circe_circuit_special_slice();
      slice_index const proxy_regular = alloc_proxy_slice();

      assert(tested!=no_slice);
      pipe_link(slices[si].prev,
                alloc_quodlibet_slice(proxy_regular,proxy_special));
      pipe_link(proxy_special,special);
      pipe_link(special,tested);
      pipe_link(proxy_regular,si);
      break;
    }

    case goal_exchangeB:
    {
      slice_index const tested = branch_find_slice(STGoalReachedTested,si);
      slice_index const proxy_special = alloc_proxy_slice();
      slice_index const special = alloc_circe_exchange_special_slice();
      slice_index const proxy_regular = alloc_proxy_slice();

      assert(tested!=no_slice);
      pipe_link(slices[si].prev,
                alloc_quodlibet_slice(proxy_regular,proxy_special));
      pipe_link(proxy_special,special);
      pipe_link(special,tested);
      pipe_link(proxy_regular,si);
      break;
    }

    default:
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors goal_filter_inserters[] =
{
  { STGoalReachedTester, &prepend_goal_filters }
};

enum
{
  nr_goal_filter_inserters = (sizeof goal_filter_inserters
                              / sizeof goal_filter_inserters[0])
};

/* Instrument a stipulation with goal filter slices
 * @param si root of branch to be instrumented
 */
void stip_insert_circe_goal_filters(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    goal_filter_inserters,
                                    nr_goal_filter_inserters);
  stip_traverse_structure(si,&st);

  TraceStipulation(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
