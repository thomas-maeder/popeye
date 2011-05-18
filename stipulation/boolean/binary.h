#if !defined(STIPULATION_BOOLEAN_BINARY_H)
#define STIPULATION_BOOLEAN_BINARY_H

/* Functionality related to "binary operator slices"
 */

#include "pystip.h"

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

/* Traversal of the moves of an operand of a binary operator
 * @param op identifies operand
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_binary_operand(slice_index op,
                                        stip_moves_traversal *st);

/* Traverse a subtree
 * @param fork root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_binary(slice_index fork,
                                    stip_structure_traversal *st);

/* Traversal of the moves of a binary operator
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_binary(slice_index si, stip_moves_traversal *st);

/* Determine contribution of slice subtree to maximum number of moves
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void get_max_nr_moves_binary(slice_index si, stip_moves_traversal *st);

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void binary_detect_starter(slice_index si, stip_structure_traversal *st);

#endif
