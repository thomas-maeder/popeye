#if !defined(SOLVING_GOALS_PREREQUISITE_GUARDS_H)
#define SOLVING_GOALS_PREREQUISITE_GUARDS_H

#include "stipulation/stipulation.h"
#include "stipulation/goals/goals.h"
#include "solving/ply.h"

/* remember if the prerequistes for the relvant goals are met */
extern unsigned int goal_preprequisites_met[maxply];

/* Instrument the stipulation structure with goal prerequisite guards.
 * These guards stop solving if the following move has to reach a
 * goal, but the prerequisites for that goal (if any) aren't met
 * before the move.
 * @param si identifies slice where to start
 */
void goal_prerequisite_guards_initialse_solving(slice_index si);

typedef struct
{
  boolean imminent_goals[nr_goals];
  boolean are_there_other_forks;
} prerequisite_guards_insertion_state;

#endif
