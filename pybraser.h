#if !defined(PYBRASER_H)
#define PYBRASER_H

#include "py.h"
#include "pystip.h"
#include "pyslice.h"

/* This module provides functionality dealing with STBranchSeries
 * stipulation slices.
 */

/* Allocate a series branch.
 * @param level is this a top-level branch or one nested into another
 *              branch?
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies next slice
 * @return index of adapter slice of allocated series branch
 */
slice_index alloc_series_branch(branch_level level,
                                stip_length_type length,
                                stip_length_type min_length,
                                slice_index next);

/* Promote a slice that was created as STBranchSeries to STSeriesRoot
 * because the assumption that the slice is nested in some other slice
 * turned out to be wrong.
 * @param adapter identifies slice to be promoted
 */
void branch_ser_promote_to_toplevel(slice_index adapter);

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 * @return true iff slice has been successfully traversed
 */
boolean branch_ser_detect_starter(slice_index si, slice_traversal *st);

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean branch_ser_solve_in_n(slice_index si, stip_length_type n);

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean branch_ser_are_threats_refuted(table threats,
                                       stip_length_type len_threat,
                                       slice_index si);

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean branch_ser_has_solution_in_n(slice_index si, stip_length_type n);

/* Determine and write threats
 * @param threats table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void branch_ser_solve_threats_in_n(table threats,
                                   slice_index si,
                                   stip_length_type n);

/* Shorten a help branch by a half-move. If the branch represents a
 * half-move only, deallocates the branch.
 * @param si identifies the branch
 * @return if the branch slice represents a half-move only, the slice
 *         representing the subsequent play; otherwise si
 */
slice_index series_root_shorten_help_play(slice_index si);

/* Spin off a set play slice at root level
 * @param si slice index
 * @param st state of traversal
 * @return true iff this slice has been sucessfully traversed
 */
boolean series_root_make_setplay_slice(slice_index si,
                                       struct slice_traversal *st);

/* Solve a branch slice at root level.
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean series_root_solve(slice_index si);

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type series_root_has_solution(slice_index si);

/* Solve a branch in exactly n moves at root level
 * @param si slice index
 * @param n exact exact number of moves
 * @return true iff >=1 solution was found
 */
boolean series_root_solve_in_n(slice_index si, stip_length_type n);

#endif
