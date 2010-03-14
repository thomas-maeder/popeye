#include "stipulation/operators/binary.h"
#include "stipulation/proxy.h"
#include "trace.h"

/* Substitute links to proxy slices by the proxy's target
 * @param si root of sub-tree where to resolve proxies
 * @param st address of structure representing the traversal
 */
void binary_resolve_proxies(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  proxy_slice_resolve(&slices[si].u.binary.op1);
  proxy_slice_resolve(&slices[si].u.binary.op2);
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
