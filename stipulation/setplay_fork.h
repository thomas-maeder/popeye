#if !defined(STIPULATION_SETPLAY_FORK_H)
#define STIPULATION_SETPLAY_FORK_H

/* Functionality related to STSetplayFork slices
 */

#include "pypipe.h"
#include "pyslice.h"

/* Allocate a STSetplayFork slice
 * @param set entry branch of set play
 * @return newly allocated slice
 */
slice_index alloc_setplay_fork_slice(slice_index set);

/* Traverse a subtree
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_setplay_fork(slice_index si,
                                          stip_structure_traversal *st);

/* Traversal of the moves of some pipe slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_setplay_fork(slice_index si, stip_moves_traversal *st);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type setplay_fork_solve(slice_index si);

#endif
