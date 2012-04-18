#include "pieces/attributes/kamikaze/kamikaze.h"
#include "pystip.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/boolean/or.h"
#include "stipulation/boolean/true.h"
#include "conditions/anticirce/target_square_filter.h"
#include "conditions/anticirce/exchange_special.h"
#include "conditions/anticirce/exchange_filter.h"
#include "debugging/trace.h"

#include <assert.h>

static void instrument_goal_target_filter(slice_index si,
                                          stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    /* reusing the filter created for Anticirce */
    square const target = slices[si].u.goal_target_reached_tester.target;
    pipe_append(si,alloc_anticirce_target_square_filter_slice(target));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_goal_exchange_filter(slice_index si,
                                            stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const proxy_special = alloc_proxy_slice();
    /* reusing the special exchange detection created for Anticirce */
    slice_index const special = alloc_anticirce_exchange_special_slice();
    slice_index const leaf_special = alloc_true_slice();

    slice_index const proxy_filter = alloc_proxy_slice();
    /* reusing the special exchange filter created for Anticirce */
    slice_index const filter = alloc_anticirce_exchange_filter_slice();

    pipe_link(slices[si].prev,
              alloc_or_slice(proxy_filter,proxy_special));

    pipe_link(proxy_special,special);
    pipe_link(special,leaf_special);

    pipe_link(proxy_filter,filter);
    pipe_link(filter,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors goal_filter_inserters[] =
{
  { STGoalTargetReachedTester,   &instrument_goal_target_filter   },
  { STGoalExchangeReachedTester, &instrument_goal_exchange_filter }
};

enum
{
  nr_goal_filter_inserters = (sizeof goal_filter_inserters
                              / sizeof goal_filter_inserters[0])
};

/* Instrument a stipulation with goal filter slices
 * @param si root of branch to be instrumented
 */
void stip_insert_kamikaze_goal_filters(slice_index si)
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
