#if !defined(STIPULATION_HELP_PLAY_MOVE_H)
#define STIPULATION_HELP_PLAY_MOVE_H

#include "boolean.h"
#include "pystip.h"
#include "pytable.h"

/* This module provides functionality dealing with STHelpMove
 * stipulation slices.
 */

/* Allocate a STHelpMove slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_help_move_slice(stip_length_type length,
                                  stip_length_type min_length);

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void help_move_insert_root(slice_index si, stip_structure_traversal *st);

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void help_move_detect_starter(slice_index si, stip_structure_traversal *st);

/* Produce slices representing set play
 * @param si slice index
 * @param st state of traversal
 */
void help_move_make_setplay_slice(slice_index si, stip_structure_traversal *st);

/* Spin off set play
 * @param si slice index
 * @param st state of traversal
 */
void help_move_apply_setplay(slice_index si, stip_structure_traversal *st);

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 *         n-2 the previous move has solved the next slice
 */
stip_length_type help_move_solve_in_n(slice_index si, stip_length_type n);

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param si slice index
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean help_move_are_threats_refuted(table threats,
                                      slice_index si);

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean help_move_has_solution_in_n(slice_index si, stip_length_type n);

/* Determine and write threats
 * @param threats table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void help_move_solve_threats_in_n(table threats,
                                  slice_index si,
                                  stip_length_type n);

#endif
