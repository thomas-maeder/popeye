#if !defined(SOLVING_SINGLE_MOVE_GENERATOR_WITH_KING_CAPTURE_H)
#define SOLVING_SINGLE_MOVE_GENERATOR_WITH_KING_CAPTURE_H

#include "stipulation/help_play/play.h"

/* This module provides functionality dealing with the attacking side
 * in STSingleMoveGeneratorWithKingCapture stipulation slices.
 */

/* Allocate a STSingleMoveGeneratorWithKingCapture slice.
 * @return index of allocated slice
 */
slice_index alloc_single_move_generator_with_king_capture_slice(void);

/* Initialise the next move generation
 * @param sq_departure departure square of move to be generated
 * @param sq_arrival arrival square of move to be generated
 * @param sq_capture capture square of move to be generated
 */
void single_move_generator_with_king_capture_init_next(square sq_departure,
                                                  square sq_arrival,
                                                  square sq_capture);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type single_move_generator_with_king_capture_help(slice_index si,
                                                              stip_length_type n);

#endif
