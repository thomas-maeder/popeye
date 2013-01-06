#if !defined(SOLVING_MAXIMUMMER_CANDIDATE_MOVE_GENERATOR_H)
#define SOLVING_MAXIMUMMER_CANDIDATE_MOVE_GENERATOR_H

#include "solving/solve.h"

/* This module provides functionality dealing with the attacking side
 * in STSingleMoveGenerator stipulation slices.
 */

/* Allocate a STSingleMoveGenerator slice.
 * @return index of allocated slice
 */
slice_index alloc_single_move_generator_slice(void);

/* Initialise the next move generation
 * @param sq_departure departure square of move to be generated
 * @param sq_arrival arrival square of move to be generated
 * @param sq_capture capture square of move to be generated
 */
void init_single_move_generator(square sq_departure,
                                square sq_arrival,
                                square sq_capture);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type single_move_generator_solve(slice_index si,
                                              stip_length_type n);

#endif
