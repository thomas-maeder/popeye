#if !defined(STIPULATION_HELP_PLAY_MOVE_GENERATOR_H)
#define STIPULATION_HELP_PLAY_MOVE_GENERATOR_H

#include "stipulation/help_play/play.h"

/* This module provides functionality dealing with STHelpMoveGenerator
 * stipulation slices.
 */

/* Allocate a STHelpMoveGenerator slice.
 * @return index of allocated slice
 */
slice_index alloc_help_move_generator_slice(void);

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type help_move_generator_help(slice_index si,
                                          stip_length_type n);

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type help_move_generator_can_help(slice_index si,
                                              stip_length_type n);

#endif
