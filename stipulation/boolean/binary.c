#include "stipulation/operators/binary.h"
#include "stipulation/proxy.h"
#include "trace.h"

/* Substitute links to proxy slices by the proxy's target
 * @param si root of sub-tree where to resolve proxies
 * @param st address of structure representing the traversal
 * @return true iff slice si has been successfully traversed
 */
boolean binary_resolve_proxies(slice_index si, slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);
  proxy_slice_resolve(&slices[si].u.fork.op1);
  proxy_slice_resolve(&slices[si].u.fork.op2);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
