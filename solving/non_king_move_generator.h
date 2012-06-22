#if !defined(SOLVING_NON_KING_MOVE_GENERATOR_H)
#define SOLVING_NON_KING_MOVE_GENERATOR_H

#include "solving/battle_play/attack_play.h"
#include "solving/battle_play/defense_play.h"

/* This module provides functionality dealing with the attacking side
 * in STNonKingMoveGenerator stipulation slices.
 */

/* Allocate a STNonKingMoveGenerator slice.
 * @return index of allocated slice
 */
slice_index alloc_non_king_move_generator_slice(void);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type non_king_move_generator_attack(slice_index si,
                                                stip_length_type n);

#endif
