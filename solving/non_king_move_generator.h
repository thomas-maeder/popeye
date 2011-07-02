#if !defined(SOLVING_NON_KING_MOVE_GENERATOR_H)
#define SOLVING_NON_KING_MOVE_GENERATOR_H

#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/defense_play.h"
#include "stipulation/help_play/play.h"

/* This module provides functionality dealing with the attacking side
 * in STNonKingMoveGenerator stipulation slices.
 */

/* Allocate a STNonKingMoveGenerator slice.
 * @return index of allocated slice
 */
slice_index alloc_non_king_move_generator_slice(void);

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type non_king_move_generator_can_help(slice_index si,
                                                  stip_length_type n);

#endif
