#if !defined(OUTPUT_PLAINTEXT_TREE_TRIVIAL_VARIATION_FILTER_H)
#define OUTPUT_PLAINTEXT_TREE_TRIVIAL_VARIATION_FILTER_H

#include "solving/solve.h"

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
trivial_end_filter_solve(slice_index si, stip_length_type n);

/* Instrument a stipulation with trivial variation filters
 * @param si identifies the entry slice of the stipulation to be instrumented
 */
void stip_insert_trivial_variation_filters(slice_index si);

#endif
