#if !defined(STIPULATION_PROXY_H)
#define STIPULATION_PROXY_H

/* Functionality related to "proxy slices"; proxy slices are pipe
 * slices that only exist while the slice structure is being built, to
 * allow a pipe slice to be conveniently be inserted before another
 * one.
 */

#include "pypipe.h"

/* Allocate an STProxy slice
 * @return newly allocated slice
 */
slice_index alloc_proxy_slice(void);

/* Substitute links to proxy slices by the proxy's target
 * @param si points to variable holding root slice of stipulation; if
 *           that slice's type is STProxy, the variable will be updated
 *           to hold the first non-proxy slice
 */
void resolve_proxies(slice_index *si);

/* Substitute a possible link to a proxy slice by the proxy's target
 * @param si address of slice index; if *si refers to a proxy slice,
 *           the index of the slice refered by the proxy will be
 *           substituted for *si's current value
 * @param st points at the structure holding the state of the traversal
 */
void proxy_slice_resolve(slice_index *si, stip_structure_traversal *st);

#endif
