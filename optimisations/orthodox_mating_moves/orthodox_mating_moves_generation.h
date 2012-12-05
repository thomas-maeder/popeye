#if !defined(OPTIMISATIONS_ORTHODOX_MATING_MOVES_GENERATION_H)
#define OPTIMISATIONS_ORTHODOX_MATING_MOVES_GENERATION_H

#include "stipulation/goals/goals.h"
#include "py.h"

extern Goal empile_for_goal;


/* This module provides functionality regarding the optimised
 * generation of orthodox moves that are to reach a goal
 */

/* Generate moves for side side_at_move; optimise for moves reaching a
 * specific goal. If the number of obstacles is ==0, the function
 * optimises by only generating orthodox moves.
 * @param side_at_move side for which to generate moves
 */
void generate_move_reaching_goal(Side side_at_move);

#endif
