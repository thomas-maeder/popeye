#if !defined(PYBRAFRK_H)
#define PYBRAFRK_H

/* Branch fork - branch decides that when to continue play in branch
 * and when to change to slice representing subsequent play
 */

#include "pyslice.h"

/* Allocate a new branch fork slice
 * @param type which slice type
 * @param length maximum number of half moves until end of slice
 * @param min_length minimum number of half moves until end of slice
 * @param proxy_to_goal identifies proxy slice that leads towards goal
 *                      from the branch
 * @return newly allocated slice
 */
slice_index alloc_branch_fork(SliceType type,
                              stip_length_type length,
                              stip_length_type min_length,
                              slice_index proxy_to_goal);

/* Substitute links to proxy slices by the proxy's target
 * @param si root of sub-tree where to resolve proxies
 * @param st address of structure representing the traversal
 */
void branch_fork_resolve_proxies(slice_index si, slice_traversal *st);

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void branch_fork_detect_starter(slice_index si, slice_traversal *st);

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type branch_fork_has_solution(slice_index si);

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean branch_fork_root_solve(slice_index si);

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 */
void branch_fork_impose_starter(slice_index si, slice_traversal *st);

#endif
