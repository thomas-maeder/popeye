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
  goal_atob, /* TODO remove? is there a difference to goal_proof? */
  goal_mate_or_stale, /* not really a goal */

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

/* TODO get rid of this */
extern boolean testdblmate;

#define ENUMERATION_TYPENAME goal_checker_result_type
#define ENUMERATORS                             \
  ENUMERATOR(goal_not_reached),                 \
    ENUMERATOR(goal_not_reached_selfcheck),     \
    ENUMERATOR(goal_reached)

#define ENUMERATION_DECLARE

#include "pyenum.h"

/* Determine whether a goal has been reached by a side in the current
 * position.
 * @param camp side that has just moved and may have reached the goal
 * @return true iff side has reached goal
 */
goal_checker_result_type goal_checker_mate(Side just_moved);
goal_checker_result_type goal_checker_circuit(Side just_moved);
goal_checker_result_type goal_checker_circuitB(Side just_moved);
goal_checker_result_type goal_checker_exchange(Side just_moved);
goal_checker_result_type goal_checker_exchangeB(Side just_moved);
goal_checker_result_type goal_checker_capture(Side just_moved);
goal_checker_result_type goal_checker_mate_ultraschachzwang(Side just_moved);
goal_checker_result_type goal_checker_stale(Side just_moved);
goal_checker_result_type goal_checker_dblstale(Side just_moved);
goal_checker_result_type goal_checker_autostale(Side just_moved);
goal_checker_result_type goal_checker_check(Side just_moved);
goal_checker_result_type goal_checker_steingewinn(Side just_moved);
goal_checker_result_type goal_checker_ep(Side just_moved);
goal_checker_result_type goal_checker_doublemate(Side just_moved);
goal_checker_result_type goal_checker_castling(Side just_moved);
goal_checker_result_type goal_checker_any(Side just_moved);
goal_checker_result_type goal_checker_proof(Side just_moved);

goal_checker_result_type goal_checker_target(Side just_moved, square target);

#endif
