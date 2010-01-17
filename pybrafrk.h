#if !defined(PYBRAFRK_H)
#define PYBRAFRK_H

/* Branch fork - decides when to continue play in branch and when to
 * change to slice representing subsequent play
 */

#include "pyslice.h"
#include "pydirect.h"

/* Allocate a STHelpFork slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param towards_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_help_fork_slice(stip_length_type length,
                                  stip_length_type min_length,
                                  slice_index towards_goal);

/* Allocate a STSeriesFork slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param towards_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_series_fork_slice(stip_length_type length,
                                    stip_length_type min_length,
                                    slice_index towards_goal);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean help_fork_solve_in_n(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean help_fork_has_solution_in_n(slice_index si, stip_length_type n);

/* Determine and write threats
 * @param threats table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void help_fork_solve_threats_in_n(table threats,
                                  slice_index si,
                                  stip_length_type n);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean series_fork_solve_in_n(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean series_fork_has_solution_in_n(slice_index si, stip_length_type n);

/* Determine and write threats
 * @param threats table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void series_fork_solve_threats_in_n(table threats,
                                    slice_index si,
                                    stip_length_type n);

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 * @return true iff slice has been successfully traversed
 */
boolean branch_fork_detect_starter(slice_index si, slice_traversal *st);

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
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean branch_fork_impose_starter(slice_index si, slice_traversal *st);

/* Deallocate a branch
 * @param branch identifies branch
 * @return index of slice representing the play after the branch
 */
slice_index branch_deallocate_to_fork(slice_index branch);

/* Find the next slice with a specific type in a branch
 * @param type type of slice to be found
 * @param si identifies the slice where to start searching (si is not
 *           visited at the start of the search, but if the branch is
 *           recursive, it may be visited as the last slice of the search)
 * @return identifier for slice with type type; no_slice if none is found
 */
slice_index branch_find_slice(SliceType type, slice_index si);

#endif
