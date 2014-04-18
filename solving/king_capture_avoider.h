#if !defined(SOLVING_KING_CAPTURE_AVOIDER_H)
#define SOLVING_KING_CAPTURE_AVOIDER_H

#include "solving/machinery/solve.h"

/* This module provides functionality to avoid king capture in immobility
 * testing for double (and counter) mate goals.
 * According to current knowledge, this can only happen if the king to be
 * mated is neutral.
 */

/* Make stip_insert_king_capture_avoiders() insert slices that prevent moves
 * that leave the moving side without king
 */
void king_capture_avoiders_avoid_own(void);

/* Make stip_insert_king_capture_avoiders() insert slices that prevent moves
 * that leave the moving side's opponent without king
 */
void king_capture_avoiders_avoid_opponent(void);

/* Instrument the solving machinery with king capture avoiders
 * @param si identifies root slice of the solving machinery
 */
void solving_insert_king_capture_avoiders(slice_index si);

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
void own_king_capture_avoider_solve(slice_index si);

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
void opponent_king_capture_avoider_solve(slice_index si);

#endif
