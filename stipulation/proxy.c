#include "stipulation/proxy.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/fork.h"
#include "stipulation/branch.h"
#include "stipulation/binary.h"
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
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  while (*si!=no_slice
         && slice_type_get_functional_type(slices[*si].type)==slice_function_proxy)
  {
    (*is_resolved_proxy)[*si] = true;
    *si = slices[*si].next1;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void binary_resolve_proxies(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  proxy_slice_resolve(&slices[si].next1,st);
  proxy_slice_resolve(&slices[si].next2,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void pipe_resolve_proxies(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].next1!=no_slice)
  {
    stip_traverse_structure_children_pipe(si,st);
    proxy_slice_resolve(&slices[si].next1,st);
  }

  if (slices[si].tester!=no_slice)
  {
    stip_traversal_activity_type const save_activity = st->activity;
    st->activity = stip_traversal_activity_testing;
    stip_traverse_structure(slices[si].tester,st);
    st->activity = save_activity;
    proxy_slice_resolve(&slices[si].tester,st);
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

  if (slices[si].next2!=no_slice)
  {
    stip_traverse_structure_next_branch(si,st);
    proxy_slice_resolve(&slices[si].next2,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Substitute links to proxy slices by the proxy's target
 * @param si points to variable holding root slice of stipulation; if
 *           that slice's type is STProxy, the variable will be updated
 *           to hold the first non-proxy slice
 */
void resolve_proxies(slice_index *si)
{
  slice_index i;
  stip_structure_traversal st;
  boolean is_resolved_proxy[max_nr_slices] = { false };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",*si);
  TraceFunctionParamListEnd();

  TraceStipulation(*si);

  assert(slices[*si].type==STProxy);

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
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_testing_pipe,
                                                &binary_resolve_proxies);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_binary,
                                                &binary_resolve_proxies);
  stip_traverse_structure(*si,&st);

  proxy_slice_resolve(si,&st);

  for (i = 0; i!=max_nr_slices; ++i)
    if (is_resolved_proxy[i])
      dealloc_slice(i);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
