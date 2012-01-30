#if !defined(STIPULATION_CONDITIONAL_PIPE_H)
#define STIPULATION_CONDITIONAL_PIPE_H

/* Functionality related to "conditional pipe slices"; i.e. pipe slices that
 * test a certain condition using an auxiliary branch
 */

#include "pypipe.h"

/* Allocate a new conditional pipe and make an existing pipe its successor
 * @param type which slice type
 * @param condition entry slice into condition
 * @return newly allocated slice
 */
slice_index alloc_conditional_pipe(slice_type type, slice_index condition);

/* Traverse a subtree
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_conditional_pipe(slice_index si,
                                              stip_structure_traversal *st);

/* Substitute links to proxy slices by the proxy's target
 * @param si slice where to resolve proxies
 * @param st points at the structure holding the state of the traversal
 */
void conditional_pipe_resolve_proxies(slice_index si,
                                      stip_structure_traversal *st);

/* Callback to stip_spin_off_testers
 * Spin a tester slice off a pipe slice
 * @param si identifies the pipe slice
 * @param st address of structure representing traversal
 */
void stip_spin_off_testers_conditional_pipe(slice_index si,
                                            stip_structure_traversal *st);

/* Spin a copy off a conditional pipe to add it to the root or set play branch
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void conditional_pipe_spin_off_copy(slice_index si,
                                    stip_structure_traversal *st);

#endif
