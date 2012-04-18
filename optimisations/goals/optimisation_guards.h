#if !defined(OPTIMISATION_GOALS_OPTIMISATION_GUARDS_H)
#define OPTIMISATION_GOALS_OPTIMISATION_GUARDS_H

#include "stipulation/moves_traversal.h"
#include "stipulation/goals/goals.h"

/* Instrument the stipulation structure with goal optimisation guards.
 * These guards stop solving if the following move has to reach a
 * goal, but the prerequisites for that goal (if any) aren't met
 * before the move.
 * @param si identifies slice where to start
 */
void stip_insert_goal_optimisation_guards(slice_index si);

/* Initialise a moves traversal for the insertion of optimising goal
 * prerequisite testers.
 * @param st to be initialised
 */
void
init_goal_prerequisite_traversal_with_optimisations(stip_moves_traversal *st);

/* Determine whether moves that are supposed to reach a particular goal are
 * optimisable
 * @param goal goal to be reached
 * @return true iff moves supposed to reach goal are optimisable
 */
boolean is_goal_reaching_move_optimisable(goal_type goal);

/* Insert a goal optimisation filter slice into a battle branch
 * @param si identifies entry slice into battle branch
 * @param context are we instrumenting for a defense or an attack?
 */
void insert_goal_optimisation_battle_filter(slice_index si,
                                            goal_type goal,
                                            stip_traversal_context_type context);

/* Insert a goal optimisation filter slice into a help branch
 * @param si identifies entry slice into battle branch
 * @param goal goal to provide optimisation for
 */
void insert_goal_optimisation_help_filter(slice_index si, goal_type goal);

#endif
