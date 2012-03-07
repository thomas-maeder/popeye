#if !defined(SOLVING_CASTLING_INTERMEDIATE_MOVE_GENERATOR_H)
#define SOLVING_CASTLING_INTERMEDIATE_MOVE_GENERATOR_H

#include "stipulation/help_play/play.h"

/* This module provides functionality dealing with the attacking side
 * in STCastlingIntermediateMoveGenerator stipulation slices.
 */

/* Allocate a STCastlingIntermediateMoveGenerator slice.
 * @return index of allocated slice
 */
slice_index alloc_castling_intermediate_move_generator_slice(void);

/* Initialise the next move generation
 * @param sq_departure departure square of move to be generated
 * @param sq_arrival arrival square of move to be generated
 */
void castling_intermediate_move_generator_init_next(square sq_departure,
                                                    square sq_arrival);

/* Solve in a number of half-moves
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move leading to the current position has
 *                           turned out to be illegal
 *            n   solution found
 *            n+2 no solution found
 */
stip_length_type castling_intermediate_move_generator_help(slice_index si,
                                                           stip_length_type n);

#endif
