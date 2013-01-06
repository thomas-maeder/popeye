#if !defined(OUTPUT_PLAINTEXT_TREE_TRIVIAL_VARIATION_FILTER_H)
#define OUTPUT_PLAINTEXT_TREE_TRIVIAL_VARIATION_FILTER_H

#include "solving/solve.h"

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type
trivial_end_filter_solve(slice_index si, stip_length_type n);

/* Instrument a stipulation with trivial variation filters
 * @param si identifies the entry slice of the stipulation to be instrumented
 */
void stip_insert_trivial_variation_filters(slice_index si);

#endif
