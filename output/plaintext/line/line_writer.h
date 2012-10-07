#if !defined(OUTPUT_PLAINTEXT_LINE_LINE_WRITER_H)
#define OUTPUT_PLAINTEXT_LINE_LINE_WRITER_H

#include "stipulation/goals/goals.h"
#include "solving/solve.h"

/* This module provides the STOutputPlaintextLineLineWriter slice type.
 * Slices of this type write solutions in line mode.
 */

/* Allocate a STOutputPlaintextLineLineWriter slice.
 * @param goal goal to be reached at end of line
 * @return index of allocated slice
 */
slice_index alloc_line_writer_slice(Goal goal);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type output_plaintext_line_line_writer_solve(slice_index si,
                                                          stip_length_type n);

#endif
