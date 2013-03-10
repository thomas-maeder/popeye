#if !defined(OPTIMISATIONS_ORTHODOX_MATING_MOVES_KING_CONTACT_GENERATOR_H)
#define OPTIMISATIONS_ORTHODOX_MATING_MOVES_KING_CONTACT_GENERATOR_H

#include "solving/solve.h"

/* This module provides a generator for moves that are supposed to reach a goal.
 * The generator is optimised for mostly orthodox problems.
 */

/* Allocate a STOrthodoxMatingKingContactGenerator slice.
 * @return index of allocated slice
 */
slice_index alloc_orthodox_mating_king_contact_generator_generator_slice(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type orthodox_mating_king_contact_generator_solve(slice_index si,
                                                              stip_length_type n);

#endif
