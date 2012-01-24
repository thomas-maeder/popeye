#if !defined(STIPULATION_TESTING_PIPE_H)
#define STIPULATION_TESTING_PIPE_H

/* Functionality related to "testing pipe slices"; i.e. pipe slices that may
 * switch to testing mode (i.e. look for a solution or a defense without
 * producing output).
 */

#include "pypipe.h"

/* Allocate a new testing pipe and make an existing pipe its successor
 * @param type which slice type
 * @return newly allocated slice
 */
slice_index alloc_testing_pipe(slice_type type);

/* Traverse a subtree
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_testing_pipe(slice_index pipe,
                                          stip_structure_traversal *st);

/* Callback to stip_spin_off_testers
 * Spin a tester slice off a testing pipe slice
 * @param si identifies the testing pipe slice
 * @param st address of structure representing traversal
 */
void stip_spin_off_testers_testing_pipe(slice_index si,
                                        stip_structure_traversal *st);

/* Substitute links to proxy slices by the proxy's target
 * @param si slice where to resolve proxies
 * @param st points at the structure holding the state of the traversal
 */
void testing_pipe_resolve_proxies(slice_index si, stip_structure_traversal *st);

#endif
