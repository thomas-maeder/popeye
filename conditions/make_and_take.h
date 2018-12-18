#if !defined(CONDITIONS_MAKE_AND_TAKE_H)
#define CONDITIONS_MAKE_AND_TAKE_H

#include "stipulation/stipulation.h"
#include "solving/move_generator.h"

/* This module implements the condition Make&Take*/

/* Continue determining whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean
make_and_take_limit_move_generation_make_walk_is_in_check(slice_index si,
                                                          Side side_king_attacked);

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void make_and_take_generate_captures_by_walk_solve(slice_index si);

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void make_and_take_move_castling_partner(slice_index si);

/* Determine whether a particular move starts with castling as a make part
 * @param ply the ply that the move is played in
 * @param move_id the id of the move
 * @return kingside_castling, queenside_castling or initsquare
 */
square make_and_take_has_move_castling_as_make(ply ply, numecoup move_id);

/* Instrument the solvers with Make&Take
 * @param si identifies the root slice of the stipulation
 */
void solving_insert_make_and_take(slice_index si);

#endif
