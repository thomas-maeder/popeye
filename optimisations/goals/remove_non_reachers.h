#if !defined(OPTIMISATION_GOALS_REMOVE_NON_REACHERS_H)
#define OPTIMISATION_GOALS_REMOVE_NON_REACHERS_H

#include "solving/machinery/solve.h"

/* This module optimises generation of moves that are supposed to reach goals
 * by inserting slices who remove moves that can't possibly reach the goal
 * to be reached
 */

/* Optimise move generation by inserting orthodox mating move generators
 * @param si identifies the root slice of the stipulation
 */
void solving_optimise_with_goal_non_reacher_removers(slice_index si);

#endif
