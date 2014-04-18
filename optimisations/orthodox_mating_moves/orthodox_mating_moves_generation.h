#if !defined(OPTIMISATIONS_ORTHODOX_MATING_MOVES_GENERATION_H)
#define OPTIMISATIONS_ORTHODOX_MATING_MOVES_GENERATION_H

#include "position/side.h"
#include "stipulation/stipulation.h"

/* This module provides functionality regarding the optimised
 * generation of orthodox moves that are to reach a goal
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

#endif
