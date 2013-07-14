#if !defined(CONDITIONS_MESSIGNY_H)
#define CONDITIONS_MESSIGNY_H

/* This module contains the implementation of Messigny Chess */

#include "solving/solve.h"

extern boolean messigny_rex_exclusive;

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_messigny(slice_index si);

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
stip_length_type messigny_remove_illegal_swap_repetitions_solve(slice_index si,
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
stip_length_type messigny_move_player_solve(slice_index si, stip_length_type n);

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param sq_departure departure square of generated moves
 * @param p walk to be used for generating
 */
void messigny_generate_moves_for_piece(slice_index si,
                                       square sq_departure,
                                       PieNam p);

#endif
