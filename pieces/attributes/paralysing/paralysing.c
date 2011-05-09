#include "pieces/attributes/paralysing/paralysing.h"
#include "pypipe.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/boolean/or.h"
#include "pieces/attributes/paralysing/mate_filter.h"
#include "pieces/attributes/paralysing/stalemate_special.h"
#include "trace.h"

#include <assert.h>

static void append_goal_mate_filter(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  pipe_append(si,alloc_paralysing_mate_filter_slice(goal_applies_to_starter));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void append_goal_stalemate_filter(slice_index si,
                                         stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const tested = branch_find_slice(STGoalReachedTester,si);
    slice_index const proxy_filter = alloc_proxy_slice();
    slice_index const filter = alloc_paralysing_stalemate_special_slice(goal_applies_to_starter);
    slice_index const proxy = alloc_proxy_slice();

    assert(tested!=no_slice);
    pipe_link(slices[si].prev,alloc_or_slice(proxy,proxy_filter));
    pipe_link(proxy_filter,filter);
    pipe_link(filter,tested);
    pipe_link(proxy,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void append_goal_doublestalemate_filter(slice_index si,
                                               stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const tested = branch_find_slice(STGoalReachedTester,si);
    slice_index const proxy_filter = alloc_proxy_slice();
    slice_index const filter1 = alloc_paralysing_stalemate_special_slice(goal_applies_to_adversary);
    slice_index const filter2 = alloc_paralysing_stalemate_special_slice(goal_applies_to_starter);
    slice_index const proxy = alloc_proxy_slice();

    assert(tested!=no_slice);
    pipe_link(slices[si].prev,alloc_or_slice(proxy,proxy_filter));
    pipe_link(proxy_filter,filter1);
    pipe_link(filter1,filter2);
    pipe_link(filter2,tested);
    pipe_link(proxy,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void append_goal_doublemate_filter(slice_index si,
                                          stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  pipe_append(si,alloc_paralysing_mate_filter_slice(goal_applies_to_starter));
  pipe_append(si,alloc_paralysing_mate_filter_slice(goal_applies_to_adversary));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void append_goal_autostalemate_filter(slice_index si,
                                             stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const tested = branch_find_slice(STGoalReachedTester,si);
    slice_index const proxy_filter = alloc_proxy_slice();
    slice_index const filter = alloc_paralysing_stalemate_special_slice(goal_applies_to_adversary);
    slice_index const proxy = alloc_proxy_slice();

    assert(tested!=no_slice);
    pipe_link(slices[si].prev,alloc_or_slice(proxy,proxy_filter));
    pipe_link(proxy_filter,filter);
    pipe_link(filter,tested);
    pipe_link(proxy,si);
	}

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors goal_filter_inserters[] =
{
  { STGoalMateReachedTester,            &append_goal_mate_filter            },
  { STGoalStalemateReachedTester,       &append_goal_stalemate_filter       },
  { STGoalDoubleStalemateReachedTester, &append_goal_doublestalemate_filter },
  { STGoalDoubleMateReachedTester,      &append_goal_doublemate_filter      },
  { STGoalCounterMateReachedTester,     &append_goal_doublemate_filter      },
  { STGoalAutoStalemateReachedTester,   &append_goal_autostalemate_filter   }
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
