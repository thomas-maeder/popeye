#if !defined(STIPULATION_SERIES_PLAY_PARRY_FORK_H)
#define STIPULATION_SERIES_PLAY_PARRY_FORK_H

#include "py.h"
#include "pystip.h"
#include "pyslice.h"

/* This module provides functionality dealing with STParryFork
 * stipulation slice
 */

/* Convert a series branch to a parry series branch
 * @param si identifies first slice of the series branch
 * @param non_parrying identifies slice t
 */
void convert_to_parry_series_branch(slice_index si, slice_index non_parrying);

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type parry_fork_series(slice_index si, stip_length_type n);

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type parry_fork_has_series(slice_index si, stip_length_type n);

/* Traverse a subtree
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_parry_fork(slice_index branch,
                                        stip_structure_traversal *st);

/* Traversal of the moves of some pipe slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_parry_fork(slice_index si, stip_moves_traversal *st);

#endif
