#include "pieces/attributes/paralysing/paralysing.h"
#include "pypipe.h"
#include "pyquodli.h"
#include "pybrafrk.h"
#include "stipulation/proxy.h"
#include "pieces/attributes/paralysing/mate_filter.h"
#include "pieces/attributes/paralysing/stalemate_special.h"
#include "trace.h"

#include <assert.h>


static void append_goal_filters(slice_index si, stip_structure_traversal *st)
{
  Side const starter = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  switch (slices[si].u.goal_reached_tester.goal.type)
  {
    case goal_mate:
      pipe_append(si,alloc_paralysing_mate_filter_slice(starter));
      break;

    case goal_doublemate:
    case goal_countermate:
      pipe_append(si,alloc_paralysing_mate_filter_slice(starter));
      pipe_append(si,alloc_paralysing_mate_filter_slice(advers(starter)));
      break;

    case goal_stale:
    {
      slice_index const tested = branch_find_slice(STGoalReachedTested,si);
      slice_index const proxy_filter = alloc_proxy_slice();
      slice_index const filter = alloc_paralysing_stalemate_special_slice(starter);
      slice_index const proxy = alloc_proxy_slice();

      assert(tested!=no_slice);
      pipe_link(slices[si].prev,alloc_quodlibet_slice(proxy,proxy_filter));
      pipe_link(proxy_filter,filter);
      pipe_link(filter,tested);
      pipe_link(proxy,si);
      break;
    }

    case goal_dblstale:
    {
      slice_index const tested = branch_find_slice(STGoalReachedTested,si);
      slice_index const proxy_filter = alloc_proxy_slice();
      slice_index const filter1 = alloc_paralysing_stalemate_special_slice(advers(starter));
      slice_index const filter2 = alloc_paralysing_stalemate_special_slice(starter);
      slice_index const proxy = alloc_proxy_slice();

      assert(tested!=no_slice);
      pipe_link(slices[si].prev,alloc_quodlibet_slice(proxy,proxy_filter));
      pipe_link(proxy_filter,filter1);
      pipe_link(filter1,filter2);
      pipe_link(filter2,tested);
      pipe_link(proxy,si);
      break;
    }

    case goal_autostale:
    {
      slice_index const tested = branch_find_slice(STGoalReachedTested,si);
      slice_index const proxy_filter = alloc_proxy_slice();
      slice_index const filter = alloc_paralysing_stalemate_special_slice(advers(starter));
      slice_index const proxy = alloc_proxy_slice();

      assert(tested!=no_slice);
      pipe_link(slices[si].prev,alloc_quodlibet_slice(proxy,proxy_filter));
      pipe_link(proxy_filter,filter);
      pipe_link(filter,tested);
      pipe_link(proxy,si);
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
  { STGoalReachedTester, &append_goal_filters }
};

enum
{
  nr_goal_filter_inserters = (sizeof goal_filter_inserters
                              / sizeof goal_filter_inserters[0])
};

/* Instrument a stipulation with goal filter slices
 * @param si root of branch to be instrumented
 */
void stip_insert_paralysing_goal_filters(slice_index si)
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
