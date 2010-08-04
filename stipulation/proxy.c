#include "stipulation/proxy.h"
#include "pypipe.h"
#include "pybrafrk.h"
#include "stipulation/operators/binary.h"
#include "trace.h"

#include <assert.h>

/* remember proxy slices
 */
static boolean is_proxy[max_nr_slices];

/* Allocate a proxy pipe
 * @return newly allocated slice
 */
slice_index alloc_proxy_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STProxy);
  is_proxy[result] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Deallocate a proxy slice
 * @param proxy identifies the proxy slice
 */
void dealloc_proxy_slice(slice_index proxy)
{
  slice_index const refered = slices[proxy].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParamListEnd();

  if (slices[refered].prev==proxy)
    slices[refered].prev = slices[proxy].prev;
  dealloc_slice(proxy);
  is_proxy[proxy] = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Deallocate all proxy pipes
 */
void dealloc_proxy_slices(void)
{
  slice_index i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 0; i!=max_nr_slices; ++i)
    if (is_proxy[i])
      dealloc_proxy_slice(i);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Substitute a possible link to a proxy slice by the proxy's target
 * @param si address of slice index
 * @param st points at the structure holding the state of the traversal
 */
void proxy_slice_resolve(slice_index *si, stip_structure_traversal *st)
{
  boolean (* const is_resolved_proxy)[max_nr_slices] = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",*si);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(*si,st);

  TraceEnumerator(SliceType,slices[*si].type,"\n");
  while (slices[*si].type==STProxy)
  {
    (*is_resolved_proxy)[*si] = true;
    *si = slices[*si].u.pipe.next;
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
  stip_structure_traversal_override_by_type(&st,
                                            slice_structure_pipe,
                                            &pipe_resolve_proxies);
  stip_structure_traversal_override_by_type(&st,
                                            slice_structure_branch,
                                            &pipe_resolve_proxies);
  stip_structure_traversal_override_by_type(&st,
                                            slice_structure_fork,
                                            &branch_fork_resolve_proxies);
  stip_structure_traversal_override_by_type(&st,
                                            slice_structure_binary,
                                            &binary_resolve_proxies);

  stip_traverse_structure(*si,&st);

  proxy_slice_resolve(si,&st);

  for (i = 0; i!=max_nr_slices; ++i)
    if (is_resolved_proxy[i])
      dealloc_proxy_slice(i);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void proxy_make_root(slice_index si, stip_structure_traversal *st)
{
  slice_index * const root = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const proxy = alloc_proxy_slice();

    if (slices[*root].prev==no_slice)
      pipe_link(proxy,*root);
    else
      pipe_set_successor(proxy,*root);

    *root = proxy;
  }

  if (slices[slices[si].u.pipe.next].prev!=si)
    slices[si].u.pipe.next = no_slice;
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
