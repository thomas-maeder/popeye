#if !defined(OPTIMISATION_GOALS_OPTIMISATION_GUARDS_H)
#define OPTIMISATION_GOALS_OPTIMISATION_GUARDS_H

/* Instrument the stipulation structure with goal optimisation guards.
 * These guards stop solving if the following move has to reach a
 * goal, but the prerequisites for that goal (if any) aren't met
 * before the move.
 */
void stip_insert_goal_optimisation_guards(void);

#endif
