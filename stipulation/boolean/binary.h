#if !defined(STIPULATION_OPERATORS_BINARY_H)
#define STIPULATION_OPERATORS_BINARY_H

/* Functionality related to "binary operator slices"
 */

#include "pystip.h"

/* Substitute links to proxy slices by the proxy's target
 * @param si root of sub-tree where to resolve proxies
 * @param st address of structure representing the traversal
 */
void binary_resolve_proxies(slice_index si, stip_structure_traversal *st);

/* Traversal of the moves of an operand of a binary operator
 * @param op identifies operand
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_binary_operand(slice_index op,
                                        stip_move_traversal *st);

/* Traversal of the moves of a binary operator
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_binary(slice_index si, stip_move_traversal *st);

#endif
