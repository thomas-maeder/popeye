#if !defined(STIPULATION_GOALS_GOALS_H)
#define STIPULATION_GOALS_GOALS_H

#include "position/board.h"
#include "utilities/boolean.h"

/* A goal describes a property
 * - of the position reached at the end of play, or
 * - of the last move of play.
 */

typedef enum
{
  goal_mate,
  goal_stale,
  goal_dblstale,
  goal_target,
  goal_check,
  goal_capture,
  goal_steingewinn,
  goal_ep,
  goal_doublemate,
  goal_countermate,
  goal_castling,
  goal_autostale,
  goal_circuit,
  goal_exchange,
  goal_circuit_by_rebirth,
  goal_exchange_by_rebirth,
  goal_any,
  goal_proofgame,
  goal_atob, /* TODO remove? is there a difference to goal_proofgame? */
  goal_mate_or_stale,
  goal_chess81,
  goal_kiss,
  goal_automate,

  goal_negated,

  nr_goals,
  no_goal = nr_goals
} goal_type;

typedef struct
{
    goal_type type;
    square target; /* for goal==goal_target */
} Goal;

/* how to decorate a move that reached a goal */
extern char const *goal_end_marker[nr_goals];

/* Determine whether two goals are equal
 * @param goal1 first goal
 * @param goal2 second goal
 * @return true iff goal1 and goal2 are equal
 */
boolean are_goals_equal(Goal goal1, Goal goal2);

#endif
