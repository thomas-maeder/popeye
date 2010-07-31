#if !defined(STIPULATION_OPERATORS_BINARY_H)
#define STIPULATION_OPERATORS_BINARY_H

/* Functionality related to "binary operator slices"
 */

#include "pystip.h"

/* Substitute links to proxy slices by the proxy's target
 * @param si slice where to resolve proxies
 */
void binary_resolve_proxies(slice_index si);

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

#endif
