#if !defined(STIPULATION_OPERATORS_BINARY_H)
#define STIPULATION_OPERATORS_BINARY_H

/* Functionality related to "binary operator slices"
 */

#include "pystip.h"

/* Substitute links to proxy slices by the proxy's target
 * @param si root of sub-tree where to resolve proxies
 * @param st address of structure representing the traversal
 * @return true iff slice si has been successfully traversed
 */
boolean binary_resolve_proxies(slice_index si, slice_traversal *st);

#endif
