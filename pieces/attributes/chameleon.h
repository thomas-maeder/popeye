#if !defined(PIECES_ATTRIBUTES_CHAMELEON_H)
#define PIECES_ATTRIBUTES_CHAMELEON_H

/* This module implements Chameleon pieces and Chameleon Chess */

#include "solving/solve.h"

extern char ChameleonSequence[256];

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
stip_length_type chameleon_change_promotee_into_solve(slice_index si,
                                                      stip_length_type n);

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
stip_length_type chameleon_arriving_adjuster_solve(slice_index si,
                                                    stip_length_type n);

/* Instrument the solving machinery for solving problems with some
 * chameleon pieces
 * @param si identifies root slice of stipulation
 */
void chameleon_initialise_solving(slice_index si);

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
stip_length_type chameleon_chess_arriving_adjuster_solve(slice_index si,
                                                         stip_length_type n);

/* Instrument the solving machinery for solving problems with the condition
 * Chameleon Chess
 * @param si identifies root slice of stipulation
 */
void chameleon_chess_initialise_solving(slice_index si);

#endif
