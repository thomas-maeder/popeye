#if !defined(OUTPUT_PLAINTEXT_LINE_LINE_WRITER_H)
#define OUTPUT_PLAINTEXT_LINE_LINE_WRITER_H

#include "stipulation/goals/goals.h"
#include "solving/solve.h"

/* This module provides the STOutputPlaintextLineLineWriter slice type.
 * Slices of this type write solutions in line mode.
 */

/* Write a move
 * @param goal goal reached by that line
 */
void output_plaintext_line_write_line(goal_type goal);

/* Allocate a STOutputPlaintextLineLineWriter slice.
 * @param goal goal to be reached at end of line
 * @return index of allocated slice
 */
slice_index alloc_line_writer_slice(Goal goal);

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
stip_length_type output_plaintext_line_line_writer_solve(slice_index si,
                                                          stip_length_type n);

#endif
