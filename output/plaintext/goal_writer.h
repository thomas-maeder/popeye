#if !defined(OUTPUT_PLAINTEXT_GOAL_WRITER_H)
#define OUTPUT_PLAINTEXT_GOAL_WRITER_H

#include "stipulation/goals/goals.h"
#include "solving/solve.h"

/* This module provides the STOutputPlaintextGoalWriter slice type.
 * Slices of this type write the goal at the end of a variation
 */

/* Allocate a STOutputPlaintextGoalWriter slice.
 * @param goal goal to be reached at end of line
 * @return index of allocated slice
 */
slice_index alloc_goal_writer_slice(Goal goal);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type output_plaintext_goal_writer_solve(slice_index si,
                                                    stip_length_type n);

#endif
