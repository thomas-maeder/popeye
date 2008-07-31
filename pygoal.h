#if !defined(PYGOAL_H)
#define PYGOAL_H

#include "boolean.h"
#include "py.h"

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
  goal_circuitB,
  goal_exchangeB,
  goal_any,
  goal_proof,
#if !defined(DATABASE)
  /* TODO why not if DATABASE? */
  goal_atob, /* TODO remove? is there a difference to goal_proof? */
#endif
  goal_mate_or_stale, /* not really a goal */

  nr_goals,
  no_goal = nr_goals
} Goal;

/* how to decorate a move that reached a goal */
extern char const *goal_end_marker[nr_goals];

/* TODO get rid of this */
extern boolean testdblmate;

/* Determine whether a goal has been reached by a side in the current
 * position.
 * @param camp side for which to test goal
 * @return true iff side has reached goal
 */
boolean goal_checker_mate(Side camp);
boolean goal_checker_circuit(Side camp);
boolean goal_checker_circuitB(Side camp);
boolean goal_checker_exchange(Side camp);
boolean goal_checker_exchangeB(Side camp);
boolean goal_checker_capture(Side camp);
boolean goal_checker_mate_ultraschachzwang(Side camp);
boolean goal_checker_stale(Side camp);
boolean goal_checker_dblstale(Side camp);
boolean goal_checker_autostale(Side camp);
boolean goal_checker_check(Side camp);
boolean goal_checker_steingewinn(Side camp);
boolean goal_checker_ep(Side camp);
boolean goal_checker_doublemate(Side camp);
boolean goal_checker_castling(Side camp);
boolean goal_checker_any(Side camp);

boolean goal_checker_target(Side camp, square target);

#endif
