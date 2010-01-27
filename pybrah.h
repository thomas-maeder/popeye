#if !defined(PYBRAH_H)
#define PYBRAH_H

#include "py.h"
#include "pystip.h"
#include "pyslice.h"
#include "pytable.h"

/* This module provides functionality dealing with STBranchHelp
 * stipulation slices.
 */

/* Allocate a STHelpRoot slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies proxy slice leading towards goal
 * @param short_sols identifies slice to delegate to when looking for
 *                   short solutions
 * @return index of allocated slice
 */
slice_index alloc_help_root_slice(stip_length_type length,
                                  stip_length_type min_length,
                                  slice_index proxy_to_goal,
                                  slice_index short_sols);

/* Allocate a help branch.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param to_goal identifies slice leading towards goal
 * @return index of initial slice of allocated help branch
 */
slice_index alloc_help_branch(stip_length_type length,
                              stip_length_type min_length,
                              slice_index to_goal);

/* Allocate a STBranchHelp slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies proxy slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_branch_h_slice(stip_length_type length,
                                 stip_length_type min_length,
                                 slice_index proxy_to_goal);

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 * @return true iff slice has been successfully traversed
 */
boolean branch_h_insert_root(slice_index si, slice_traversal *st);

/* Substitute links to proxy slices by the proxy's target
 * @param si root of sub-tree where to resolve proxies
 * @param st address of structure representing the traversal
 * @return true iff slice si has been successfully traversed
 */
boolean help_root_resolve_proxies(slice_index si, slice_traversal *st);

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 * @return true iff slice has been successfully traversed
 */
boolean branch_h_detect_starter(slice_index si, slice_traversal *st);

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean branch_h_solve_in_n(slice_index si, stip_length_type n);

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param si slice index
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean branch_h_are_threats_refuted(table threats,
                                     slice_index si);

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean branch_h_has_solution_in_n(slice_index si, stip_length_type n);

/* Determine and write threats
 * @param threats table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void branch_h_solve_threats_in_n(table threats,
                                 slice_index si,
                                 stip_length_type n);

/* Shorten a help branch by a half-move. If the branch represents a
 * half-move only, deallocates the branch.
 * @param si identifies the branch
 * @return if the branch slice represents a half-move only, the slice
 *         representing the subsequent play; otherwise si
 */
slice_index help_root_shorten_help_play(slice_index si);

/* Spin off a set play slice at root level
 * @param si slice index
 * @param st state of traversal
 * @return true iff this slice has been sucessfully traversed
 */
boolean help_root_make_setplay_slice(slice_index si,
                                     struct slice_traversal *st);

/* Solve a branch slice at root level.
 * @param si slice index
 * @return no_slice if set play not applicable
 *         new root slice index (may be equal to old one) otherwise
 * @return true iff >=1 solution was found
 */
boolean help_root_root_solve(slice_index si);

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type help_root_has_solution(slice_index si);

#endif
