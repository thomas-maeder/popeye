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

/* Deallocate a proxy slice
 * @param proxy identifies the proxy slice
 */
void dealloc_proxy_slice(slice_index proxy);

/* Deallocate all proxy slices
 */
void dealloc_proxy_slices(void);

/* Substitute a possible link to a proxy slice by the proxy's target
 * @param si address of slice index; if *si refers to a proxy slice,
 *           the index of the slice refered by the proxy will be
 *           substituted for *si's current value
 */
void proxy_slice_resolve(slice_index *si);

/* Spin off a set play slice
 * @param si slice index
 * @param st state of traversal
 */
void proxy_make_setplay_slice(slice_index si, stip_structure_traversal *st);

#endif
