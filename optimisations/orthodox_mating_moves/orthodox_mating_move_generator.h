#if !defined(OPTIMISATIONS_ORTHODOX_MATING_MOVES_GENERATOR_H)
#define OPTIMISATIONS_ORTHODOX_MATING_MOVES_GENERATOR_H

#include "solving/solve.h"
#include "stipulation/goals/goals.h"

/* This module provides a generator for moves that are supposed to reach a goal.
 * The generator is optimised for mostly orthodox problems.
 */

/* Allocate a STOrthodoxMatingMoveGenerator slice.
 * @param goal goal to be reached
 * @return index of allocated slice
 */
slice_index alloc_orthodox_mating_move_generator_slice(Goal goal);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
orthodox_mating_move_generator_solve(slice_index si, stip_length_type n);

#endif
