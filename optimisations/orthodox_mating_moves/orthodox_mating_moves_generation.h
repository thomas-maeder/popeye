#if !defined(OPTIMISATIONS_ORTHODOX_MATING_MOVES_GENERATION_H)
#define OPTIMISATIONS_ORTHODOX_MATING_MOVES_GENERATION_H

#include "boolean.h"
#include "pygoal.h"
#include "py.h"

extern Goal empile_for_goal;


/* This module provides functionality regarding the optimised
 * generation of orthodox moves that are to reach a goal
 */

/* Reset the number of obstacles that might prevent
 * generate_move_reaching_goal() from optimising by only generating
 * orthodox moves
 */
void reset_ortho_mating_moves_generation_obstacles(void);

/* Add an obstacle.
 */
void add_ortho_mating_moves_generation_obstacle(void);

/* Remove an obstacle.
 */
void remove_ortho_mating_moves_generation_obstacle(void);

/* Determine whether the prerequisites met for reaching a goal with
 * the next move
 * @param goal goal to be reached
 * @param side_at_move side to execute the move reaching the goal
 * @return true iff the prerequisites are met
 */
boolean are_prerequisites_for_reaching_goal_met(goal_type goal,
                                                Side side_at_move);

/* Generate moves for side side_at_move; optimise for moves reaching a
 * specific goal. If the number of obstacles is ==0, the function
 * optimises by only generating orthodox moves.
 * @param side_at_move side for which to generate moves
 */
void generate_move_reaching_goal(Side side_at_move);

#endif
