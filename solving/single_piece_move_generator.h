#if !defined(SOLVING_SINGLE_PIECE_MOVE_GENERATOR_H)
#define SOLVING_SINGLE_PIECE_MOVE_GENERATOR_H

#include "solving/battle_play/attack_play.h"

/* This module provides functionality dealing with the attacking side
 * in STSinglePieceMoveGenerator stipulation slices.
 */

void init_single_piece_move_generator(square sq_departure, piece pi_moving);

/* Allocate a STSinglePieceMoveGenerator slice.
 * @return index of allocated slice
 */
slice_index alloc_single_piece_move_generator_slice(void);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type single_piece_move_generator_attack(slice_index si,
                                                    stip_length_type n);

#endif
