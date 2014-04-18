#if !defined(PIECES_PARALYSING_STALEMATE_FILTER_H)
#define PIECES_PARALYSING_STALEMATE_FILTER_H

#include "stipulation/stipulation.h"
#include "solving/machinery/solve.h"

/* This module provides slice type STPiecesParalysingStalemateSpecial - as a
 * consequence of the special mating rule, a side is stalemate if it is in check
 * but totally paralysed
 */

/* Allocate a STPiecesParalysingStalemateSpecial slice.
 * @param starter_or_adversary is the starter stalemated or its adversary?
 * @return index of allocated slice
 */
slice_index
alloc_paralysing_stalemate_special_slice(goal_applies_to_starter_or_adversary starter_or_adversary);

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
void paralysing_stalemate_special_solve(slice_index si);

#endif
