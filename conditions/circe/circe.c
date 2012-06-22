#include "conditions/circe/circe.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/boolean/or.h"
#include "stipulation/boolean/true.h"
#include "conditions/circe/circuit_by_rebirth_special.h"
#include "conditions/circe/exchange_by_rebirth_special.h"
#include "conditions/circe/steingewinn_filter.h"
#include "debugging/trace.h"

#include <assert.h>

static void prepend_steingewinn_filter(slice_index si,
                                       stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);
  pipe_append(slices[si].prev,alloc_circe_steingewinn_filter_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void prepend_circuit_by_rebirth_filter(slice_index si,
                                              stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const proxy_special = alloc_proxy_slice();
    slice_index const special = alloc_circe_circuit_special_slice();
    slice_index const leaf_special = alloc_true_slice();

    slice_index const proxy_regular = alloc_proxy_slice();

    pipe_link(slices[si].prev,
              alloc_or_slice(proxy_regular,proxy_special));

    pipe_link(proxy_special,special);
    pipe_link(special,leaf_special);

    pipe_link(proxy_regular,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void prepend_exchange_by_rebirth_filter(slice_index si,
                                               stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const proxy_special = alloc_proxy_slice();
    slice_index const special = alloc_circe_exchange_special_slice();
    slice_index const leaf_special = alloc_true_slice();

    slice_index const proxy_regular = alloc_proxy_slice();

    pipe_link(slices[si].prev,
              alloc_or_slice(proxy_regular,proxy_special));

    pipe_link(proxy_special,special);
    pipe_link(special,leaf_special);

    pipe_link(proxy_regular,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor goal_filter_inserters[] =
{
  { STGoalSteingewinnReachedTester,       &prepend_steingewinn_filter         },
  { STGoalCircuitByRebirthReachedTester,  &prepend_circuit_by_rebirth_filter  },
  { STGoalExchangeByRebirthReachedTester, &prepend_exchange_by_rebirth_filter }
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
