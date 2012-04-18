#if !defined(STIPULATION_BOOLEAN_BINARY_H)
#define STIPULATION_BOOLEAN_BINARY_H

/* Functionality related to "binary operator slices"
 */

#include "stipulation/structure_traversal.h"

/* Allocate a binary slice.
 * @param op1 proxy to 1st operand
 * @param op2 proxy to 2nd operand
 * @return index of allocated slice
 */
slice_index alloc_binary_slice(slice_type type,
                               slice_index op1, slice_index op2);

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void binary_make_root(slice_index si, stip_structure_traversal *st);

/* Substitute links to proxy slices by the proxy's target
 * @param si slice where to resolve proxies
 * @param st points at the structure holding the state of the traversal
 */
void binary_resolve_proxies(slice_index si, stip_structure_traversal *st);

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void binary_detect_starter(slice_index si, stip_structure_traversal *st);

/* Callback to stip_spin_off_testers
 * Spin a tester slice off a binary slice
 * @param si identifies the testing pipe slice
 * @param st address of structure representing traversal
 */
void stip_spin_off_testers_binary(slice_index si, stip_structure_traversal *st);

#endif
