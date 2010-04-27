#include "stipulation/proxy.h"
#include "pypipe.h"
#include "trace.h"

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
  if (slices[refered].prev==proxy)
    slices[refered].prev = slices[proxy].prev;
  dealloc_slice(proxy);
  is_proxy[proxy] = false;
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
 */
void proxy_slice_resolve(slice_index *si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",*si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[*si].type,"\n");
  while (slices[*si].type==STProxy)
    *si = slices[*si].u.pipe.next;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Spin off a set play slice
 * @param si slice index
 * @param st state of traversal
 */
void proxy_make_setplay_slice(slice_index si, stip_structure_traversal *st)
{
  setplay_slice_production * const prod = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (prod->setplay_slice==slices[si].u.pipe.next)
    prod->setplay_slice = si;

  prod->sibling = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
