#if !defined(OUTPUT_PLAINTEXT_TREE_REFUTING_VARIATION_WRITER_H)
#define OUTPUT_PLAINTEXT_TREE_REFUTING_VARIATION_WRITER_H

#include "solving/solve.h"

/* Allocate a STRefutingVariationWriter slice.
 * @return index of allocated slice
 */
slice_index alloc_refuting_variation_writer_slice(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
refuting_variation_writer_solve(slice_index si, stip_length_type n);

#endif
