#if !defined(OPTIMISATIONS_ORTHODOX_MATING_MOVES_GENERATOR_H)
#define OPTIMISATIONS_ORTHODOX_MATING_MOVES_GENERATOR_H

#include "solving/solve.h"

/* This module provides a generator for moves that are supposed to reach a goal.
 * The generator is optimised for mostly orthodox problems.
 */

/* Reset the enabled state of the optimisation of final defense moves
 */
void reset_orthodox_mating_move_optimisation(void);

/* Disable the optimisation of final defense moves for defense by a side
 * @param side side for which to disable the optimisation
 */
void disable_orthodox_mating_move_optimisation(Side side);

/* Optimise move generation by inserting orthodox mating move generators
 * @param si identifies the root slice of the stipulation
 */
void stip_optimise_with_orthodox_mating_move_generators(slice_index si);

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
