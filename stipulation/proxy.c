#include "stipulation/proxy.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/fork.h"
#include "stipulation/branch.h"
#include "stipulation/binary.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Allocate a proxy pipe
 * @return newly allocated slice
 */
slice_index alloc_proxy_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STProxy);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void proxy_slice_resolve(slice_index *si, stip_structure_traversal *st)
{
  boolean (* const is_resolved_proxy)[max_nr_slices] = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",*si);
  TraceFunctionParam("%p",(void *)st);
  TraceFunctionParamListEnd();

  while (*si!=no_slice
         && slice_type_get_functional_type(SLICE_TYPE(*si))==slice_function_proxy)
  {
    (*is_resolved_proxy)[*si] = true;
    *si = SLICE_NEXT1(*si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void binary_resolve_proxies(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",(void *)st);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  proxy_slice_resolve(&SLICE_NEXT1(si),st);
  proxy_slice_resolve(&SLICE_NEXT2(si),st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void pipe_resolve_proxies(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (SLICE_NEXT1(si)!=no_slice)
  {
    stip_traverse_structure_children_pipe(si,st);
    proxy_slice_resolve(&SLICE_NEXT1(si),st);
    if (SLICE_NEXT1(si)!=no_slice)
      SLICE_PREV(SLICE_NEXT1(si)) = si;
  }

  if (SLICE_TESTER(si)!=no_slice)
  {
    stip_traversal_activity_type const save_activity = st->activity;
    st->activity = stip_traversal_activity_testing;
    stip_traverse_structure(SLICE_TESTER(si),st);
    st->activity = save_activity;
    proxy_slice_resolve(&SLICE_TESTER(si),st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void fork_resolve_proxies(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_resolve_proxies(si,st);

  if (SLICE_NEXT2(si)!=no_slice)
  {
    stip_traverse_structure_next_branch(si,st);
    proxy_slice_resolve(&SLICE_NEXT2(si),st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Substitute links to proxy slices by the proxy's target
 * @param si slice_index
 */
void proxies_resolve(slice_index si)
{
  slice_index i;
  stip_structure_traversal st;
  boolean is_resolved_proxy[max_nr_slices] = { false };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&is_resolved_proxy);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_pipe,
                                                 &pipe_resolve_proxies);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_branch,
                                                 &pipe_resolve_proxies);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_fork,
                                                 &fork_resolve_proxies);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_testing_pipe,
                                                  &binary_resolve_proxies);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_binary,
                                                  &binary_resolve_proxies);
  stip_traverse_structure(si,&st);

  for (i = 0; i!=max_nr_slices; ++i)
    if (is_resolved_proxy[i])
      dealloc_slice(i);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
