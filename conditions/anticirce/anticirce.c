#include "conditions/anticirce/anticirce.h"
#include "pypipe.h"
#include "pyquodli.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "conditions/anticirce/target_square_filter.h"
#include "conditions/anticirce/circuit_special.h"
#include "conditions/anticirce/exchange_special.h"
#include "conditions/anticirce/exchange_filter.h"
#include "trace.h"

#include <assert.h>


static void append_goal_filters(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  switch (slices[si].u.goal_reached_tester.goal.type)
  {
    case goal_target:
      assert(0);
      break;

    case goal_circuit:
    {
      slice_index const tested = branch_find_slice(STGoalReachedTested,si);
      slice_index const proxy_special = alloc_proxy_slice();
      slice_index const special = alloc_anticirce_circuit_special_slice();
      slice_index const proxy_regular = alloc_proxy_slice();

      assert(tested!=no_slice);
      pipe_link(slices[si].prev,
                alloc_quodlibet_slice(proxy_regular,proxy_special));
      pipe_link(proxy_special,special);
      pipe_link(special,tested);
      pipe_link(proxy_regular,si);
      break;
    }

    case goal_exchange:
    {
      slice_index const tested = branch_find_slice(STGoalReachedTested,si);

      slice_index const proxy_special = alloc_proxy_slice();
      slice_index const special = alloc_anticirce_exchange_special_slice();

      slice_index const proxy_filter = alloc_proxy_slice();
      slice_index const filter = alloc_anticirce_exchange_filter_slice();

      assert(tested!=no_slice);
      pipe_link(slices[si].prev,
                alloc_quodlibet_slice(proxy_filter,proxy_special));
      pipe_link(proxy_special,special);
      pipe_link(special,tested);
      pipe_link(proxy_filter,filter);
      pipe_link(filter,si);
      break;
    }

    default:
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_goal_target_filter(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    square const target = slices[si].u.goal_reached_tester.goal.target;
    slice_index const filter = alloc_anticirce_target_square_filter_slice(target);
    pipe_append(slices[si].prev,filter);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors goal_filter_inserters[] =
{
  { STGoalReachedTester,       &append_goal_filters           },
  { STGoalTargetReachedTester, &instrument_goal_target_filter }
};

enum
{
  nr_goal_filter_inserters = (sizeof goal_filter_inserters
                              / sizeof goal_filter_inserters[0])
};

/* Instrument a stipulation with goal filter slices
 * @param si root of branch to be instrumented
 */
void stip_insert_anticirce_goal_filters(slice_index si)
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
