#include "stipulation/goals/prerequisite_guards.h"
#include "pystip.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/series_play/branch.h"
#include "stipulation/goals/prerequisite_optimiser.h"
#include "stipulation/goals/countermate/filter.h"
#include "stipulation/goals/doublemate/filter.h"
#include "optimisations/goals/optimisation_guards.h"

#include "trace.h"

#include <assert.h>

/* remember if the prerequistes for the relvant goals are met */
unsigned int goal_preprequisites_met[maxply];

boolean insert_goal_prerequisite_guard_battle_filter(slice_index si,
                                                     goal_type goal)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  switch (goal)
  {
    case goal_doublemate:
    {
      slice_index const prototype = alloc_doublemate_filter_slice();
      battle_branch_insert_slices(si,&prototype,1);
      result = true;
      break;
    }

    case goal_countermate:
    {
      slice_index const prototype = alloc_countermate_filter_slice();
      battle_branch_insert_slices(si,&prototype,1);
      result = true;
      break;
    }

    default:
      result = false;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean insert_goal_prerequisite_guard_help(slice_index si, goal_type goal)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  switch (goal)
  {
    case goal_doublemate:
      pipe_append(si,alloc_doublemate_filter_slice());
      result = true;
      break;

    case goal_countermate:
      pipe_append(si,alloc_countermate_filter_slice());
      result = true;
      break;

    default:
      result = false;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean insert_goal_prerequisite_guard_series(slice_index si, goal_type goal)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  switch (goal)
  {
    case goal_doublemate:
    {
      pipe_append(si,alloc_doublemate_filter_slice());
      result = true;
      break;
    }

    case goal_countermate:
    {
      pipe_append(si,alloc_countermate_filter_slice());
      result = true;
      break;
    }

    default:
      result = false;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert goal prerequisite guards
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void insert_goal_prerequisite_guards_battle(slice_index si,
                                                   stip_moves_traversal *st)
{
  prerequisite_guards_insertion_state * const state = st->param;
  prerequisite_guards_insertion_state const save_state = *state;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_children(si,st);

  TraceValue("%u\n",st->remaining);
  if (st->remaining==1)
  {
    unsigned int nr_optimisable = 0;
    unsigned int nr_unoptimisable = 0;
    goal_type goal;

    for (goal = 0; goal!=nr_goals; ++goal)
      if (state->imminent_goals[goal])
      {
        if (insert_goal_prerequisite_guard_battle_filter(si,goal)
            || is_goal_reaching_move_optimisable(goal))
          ++nr_optimisable;
        else
          ++nr_unoptimisable;
      }

    if (nr_optimisable>0 && nr_unoptimisable==0)
    {
      slice_index const prototype = alloc_goal_prerequisite_optimiser_slice();
      battle_branch_insert_slices(si,&prototype,1);

      for (goal = 0; goal!=nr_goals; ++goal)
        if (state->imminent_goals[goal])
          insert_goal_optimisation_battle_filter(si,goal);
    }
  }

  *state = save_state;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert goal prerequisite guards
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void insert_goal_prerequisite_guards_help(slice_index si,
                                                 stip_moves_traversal *st)
{
  prerequisite_guards_insertion_state * const state = st->param;
  prerequisite_guards_insertion_state const save_state = *state;
  slice_index const fork = slices[si].u.fork.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_children(si,st);

  if (st->remaining==1)
  {
    unsigned int nr_optimisable = 0;
    unsigned int nr_unoptimisable = 0;
    goal_type goal;

    for (goal = 0; goal!=nr_goals; ++goal)
      if (state->imminent_goals[goal])
      {
        if (insert_goal_prerequisite_guard_help(fork,goal)
            || is_goal_reaching_move_optimisable(goal))
          ++nr_optimisable;
        else
          ++nr_unoptimisable;
      }

    if (nr_optimisable>0 && nr_unoptimisable==0)
    {
      for (goal = 0; goal!=nr_goals; ++goal)
        if (state->imminent_goals[goal])
          insert_goal_optimisation_help_filter(fork,goal);
      {
        slice_index const prototype = alloc_goal_prerequisite_optimiser_slice();
        help_branch_insert_slices(slices[fork].u.pipe.next,&prototype,1);
      }
    }
  }

  *state = save_state;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert goal prerequisite guards
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void insert_goal_prerequisite_guards_series(slice_index si,
                                                   stip_moves_traversal *st)
{
  prerequisite_guards_insertion_state * const state = st->param;
  prerequisite_guards_insertion_state const save_state = *state;
  slice_index const fork = slices[si].u.fork.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_children(si,st);

  if (st->remaining==1)
  {
    unsigned int nr_optimisable = 0;
    unsigned int nr_unoptimisable = 0;
    goal_type goal;

    for (goal = 0; goal!=nr_goals; ++goal)
      if (state->imminent_goals[goal])
      {
        if (insert_goal_prerequisite_guard_series(fork,goal)
            || is_goal_reaching_move_optimisable(goal))
          ++nr_optimisable;
        else
          ++nr_unoptimisable;
      }

    if (nr_optimisable>0 && nr_unoptimisable==0)
    {
      for (goal = 0; goal!=nr_goals; ++goal)
        if (state->imminent_goals[goal])
          insert_goal_optimisation_series_filter(fork,goal);

      {
        slice_index const prototype = alloc_goal_prerequisite_optimiser_slice();
        series_branch_insert_slices(slices[fork].u.pipe.next,&prototype,1);
      }
    }
  }

  *state = save_state;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert goal prerequisite guards
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static
void insert_goal_prerequisite_guards_doublemate(slice_index si,
                                                stip_moves_traversal *st)
{
  prerequisite_guards_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_children(si,st);
  state->imminent_goals[goal_doublemate] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert goal prerequisite guards
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static
void insert_goal_prerequisite_guards_countermate(slice_index si,
                                                 stip_moves_traversal *st)
{
  prerequisite_guards_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_children(si,st);
  state->imminent_goals[goal_countermate] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static moves_traversers_visitors const prerequisite_guard_inserters[] =
{
  { STReadyForAttack,               &insert_goal_prerequisite_guards_battle      },
  { STReadyForDefense,              &insert_goal_prerequisite_guards_battle      },
  { STHelpFork,                     &insert_goal_prerequisite_guards_help        },
  { STSeriesFork,                   &insert_goal_prerequisite_guards_series      },
  { STGoalDoubleMateReachedTester,  &insert_goal_prerequisite_guards_doublemate  },
  { STGoalCounterMateReachedTester, &insert_goal_prerequisite_guards_countermate },
};

enum
{
  nr_prerequisite_guard_inserters
  = (sizeof prerequisite_guard_inserters
     / sizeof prerequisite_guard_inserters[0])
};

/* Instrument the stipulation structure with goal prerequisite guards.
 * These guards stop solving if the following move has to reach a
 * goal, but the prerequisites for that goal (if any) aren't met
 * before the move.
 * @param si identifies slice where to start
 */
void stip_insert_goal_prerequisite_guards(slice_index si)
{
  stip_moves_traversal traversal;
  prerequisite_guards_insertion_state state = { { false }, false };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_moves_traversal_init(&traversal,&state);
  stip_moves_traversal_override(&traversal,
                                prerequisite_guard_inserters,
                                nr_prerequisite_guard_inserters);
  init_goal_prerequisite_traversal_with_optimisations(&traversal);
  stip_traverse_moves(si,&traversal);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
