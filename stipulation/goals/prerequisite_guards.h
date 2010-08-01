#if !defined(STIPULATION_GOALS_PREREQUISITE_GUARDS_H)
#define STIPULATION_GOALS_PREREQUISITE_GUARDS_H

/* Instrument the stipulation structure with goal prerequisite guards.
 * These guards stop solving if the following move has to reach a
 * goal, but the prerequisites for that goal (if any) aren't met
 * before the move.
 */
void stip_insert_goal_prerequisite_guards(void);

#endif
