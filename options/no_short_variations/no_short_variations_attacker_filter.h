#if !defined(OPTIONS_NO_SHORT_VARIATIONS_NO_SHORT_VARIATIONS_ATTACKER_FILTER_H)
#define OPTIONS_NO_SHORT_VARIATIONS_NO_SHORT_VARIATIONS_ATTACKER_FILTER_H

#include "stipulation/stipulation.h"

/* This module provides functionality dealing with the attacking side
 * in STNoShortVariations stipulation slices.
 */

/* Allocate a STNoShortVariations slice.
 * @return index of allocated slice
 */
slice_index alloc_no_short_variations_slice(void);

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
void no_short_variations_solve(slice_index si);

#endif
