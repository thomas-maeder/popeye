#if !defined(CONDITIONS_IMMUNE_H)
#define CONDITIONS_IMMUNE_H

/* This module implements the condition Immune Chess */

#include "utilities/boolean.h"
#include "position/piece.h"
#include "position/board.h"
#include "position/position.h"
#include "py.h"

/* Address of function used to determine the relevant square for finding out
 * whether a piece is immune
 */
extern square (*immunrenai)(piece, Flags, square, square, square, Side);

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
stip_length_type immune_remove_captures_of_immune_solve(slice_index si,
                                                        stip_length_type n);

/* Instrument the solvers with Immune Chess
 * @param si identifies the root slice of the stipulation
 */
void stip_insert_immune(slice_index si);

#endif
