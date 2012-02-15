#if !defined(SOLVING_SINGLE_PIECE_MOVE_GENERATOR_H)
#define SOLVING_SINGLE_PIECE_MOVE_GENERATOR_H

#include "stipulation/help_play/play.h"

/* This module provides functionality dealing with the attacking side
 * in STSinglePieceMoveGenerator stipulation slices.
 */

void init_single_piece_move_generator(square sq_departure, piece pi_moving);

/* Allocate a STSinglePieceMoveGenerator slice.
 * @return index of allocated slice
 */
slice_index alloc_single_piece_move_generator_slice(void);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type single_piece_move_generator_help(slice_index si,
                                                  stip_length_type n);

#endif
