#if !defined(CONDITIONS_TAKE_AND_MAKE_H)
#define CONDITIONS_TAKE_AND_MAKE_H

#include "stipulation/stipulation.h"

/* This module implements the condition Take&Make*/

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type take_and_make_generate_make_solve(slice_index si,
                                                   stip_length_type n);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type take_and_make_avoid_pawn_make_to_base_line_solve(slice_index si,
                                                                  stip_length_type n);

/* Instrument the solvers with Take&Make
 * @param si identifies the root slice of the stipulation
 */
void stip_insert_take_and_make(slice_index si);

#endif
