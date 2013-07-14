#if !defined(SOLVING_MOVE_GENERATOR_H)
#define SOLVING_MOVE_GENERATOR_H

#include "solving/solve.h"
#include "stipulation/slice_type.h"

/* This module provides functionality dealing with the attacking side
 * in STMoveGenerator stipulation slices.
 */

/* Instrument move generation with a slice type
 * @param identifies where to start instrumentation
 * @param side which side (pass nr_sides for both sides)
 * @param type type of slice with which to instrument moves
 */
void solving_instrument_move_generation(slice_index si,
                                        Side side,
                                        slice_type type);

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param sq_departure departure square of generated moves
 * @param p walk to be used for generating
 */
void generate_moves_for_piece(slice_index si, square sq_departure, PieNam p);

/* Allocate a STMoveGenerator slice.
 * @return index of allocated slice
 */
slice_index alloc_move_generator_slice(void);

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
stip_length_type move_generator_solve(slice_index si, stip_length_type n);

/* Instrument a stipulation with move generator slices
 * @param si root of branch to be instrumented
 */
void stip_insert_move_generators(slice_index si);

/* From the moves just generated, remove all those that don't meet a certain
 * criterion
 * @param criterion criterion to be met by moves to not be removed
 */
typedef boolean (*move_filter_criterion_type)(square sq_departure,
                                              square sq_arrival,
                                              square sq_capture);
void move_generator_filter_moves(move_filter_criterion_type criterion);

#endif
