#if !defined(STIPULATION_GOALS_PREREQUISITE_GUARDS_H)
#define STIPULATION_GOALS_PREREQUISITE_GUARDS_H

#include "stipulation/slice.h"
#include "stipulation/goals/goals.h"

/* remember if the prerequistes for the relvant goals are met */
extern unsigned int goal_preprequisites_met[maxply];

/* Instrument the stipulation structure with goal prerequisite guards.
 * These guards stop solving if the following move has to reach a
 * goal, but the prerequisites for that goal (if any) aren't met
 * before the move.
 * @param si identifies slice where to start
 */
void stip_insert_goal_prerequisite_guards(slice_index si);

typedef struct
{
  boolean imminent_goals[nr_goals];
  boolean are_there_other_forks;
} prerequisite_guards_insertion_state;

#endif
