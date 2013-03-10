#if !defined(SOLVING_KING_CAPTURE_AVOIDER_H)
#define SOLVING_KING_CAPTURE_AVOIDER_H

#include "solving/solve.h"

/* This module provides functionality to avoid king capture in immobility
 * testing for double (and counter) mate goals.
 * According to current knowledge, this can only happen if the king to be
 * mated is neutral.
 */

/* Reset king capture avoiders
 */
void king_capture_avoiders_reset(void);

/* Make stip_insert_king_capture_avoiders() insert slices that prevent moves
 * that leave the moving side without king
 */
void king_capture_avoiders_avoid_own(void);

/* Make stip_insert_king_capture_avoiders() insert slices that prevent moves
 * that leave the moving side's opponent without king
 */
void king_capture_avoiders_avoid_opponent(void);

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_king_capture_avoiders(slice_index si);

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
stip_length_type own_king_capture_avoider_solve(slice_index si,
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
stip_length_type opponent_king_capture_avoider_solve(slice_index si,
                                                     stip_length_type n);

#endif
