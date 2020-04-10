#include "solving/goals/prerequisite_guards.h"
#include "solving/goals/prerequisite_optimiser.h"
#include "solving/goals/countermate.h"
#include "solving/goals/doublemate.h"
#include "solving/fork_on_remaining.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "solving/dead_end.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "optimisations/goals/optimisation_guards.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* remember if the prerequistes for the relvant goals are met */
unsigned int goal_preprequisites_met[maxply];

static boolean insert_goal_prerequisite_guard_battle_filter(slice_index si,
                                                            goal_type goal,
                                                            stip_traversal_context_type context)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal);
  TraceFunctionParam("%u",context);
  TraceFunctionParamListEnd();

  switch (goal)
  {
    case goal_doublemate:
    {
      slice_index const prototype = alloc_doublemate_filter_slice();
      if (context==stip_traversal_context_attack)
        attack_branch_insert_slices(si,&prototype,1);
      else
        defense_branch_insert_slices(si,&prototype,1);
      result = true;
      break;
    }

    case goal_countermate:
    {
      slice_index const prototype = alloc_countermate_filter_slice();
      if (context==stip_traversal_context_attack)
        attack_branch_insert_slices(si,&prototype,1);
      else
        defense_branch_insert_slices(si,&prototype,1);
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

static boolean insert_goal_prerequisite_guard_help(slice_index si, goal_type goal)
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

/* Insert goal prerequisite guards for battle play
 * @param si identifies root of subtree
 * @param state address of structure representing insertion state
 */
static void insert_goal_prerequisite_guards_battle(slice_index si,
                                                   prerequisite_guards_insertion_state const * state,
                                                   stip_traversal_context_type context)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",context);
  TraceFunctionParamListEnd();

  {
    unsigned int nr_optimisable = 0;
    unsigned int nr_unoptimisable = 0;
    goal_type goal;

    for (goal = 0; goal!=nr_goals; ++goal)
      if (state->imminent_goals[goal])
      {
        if (insert_goal_prerequisite_guard_battle_filter(si,goal,context)
            || is_goal_reaching_move_optimisable(goal))
          ++nr_optimisable;
        else
          ++nr_unoptimisable;
      }

    if (nr_optimisable>0 && nr_unoptimisable==0)
    {
      slice_index const prototype = alloc_goal_prerequisite_optimiser_slice();
      if (context==stip_traversal_context_attack)
        attack_branch_insert_slices(si,&prototype,1);
      else
        defense_branch_insert_slices(si,&prototype,1);

      for (goal = 0; goal!=nr_goals; ++goal)
        if (state->imminent_goals[goal])
          insert_goal_optimisation_battle_filter(si,goal,context);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert goal prerequisite guards for help play
 * @param si identifies root of subtree
 * @param state address of structure representing insertion state
 */
static void insert_goal_prerequisite_guards_help(slice_index si,
                                                 prerequisite_guards_insertion_state const * state)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    unsigned int nr_optimisable = 0;
    unsigned int nr_unoptimisable = 0;
    slice_index const proxy1 = alloc_proxy_slice();
    slice_index const proxy2 = alloc_proxy_slice();
    slice_index const proxy_joint = alloc_proxy_slice();
    goal_type goal;

    pipe_link(proxy1,proxy_joint);
    pipe_set_successor(proxy2,proxy_joint);

    for (goal = 0; goal!=nr_goals; ++goal)
      if (state->imminent_goals[goal])
      {
        if (insert_goal_prerequisite_guard_help(proxy1,goal)
            || is_goal_reaching_move_optimisable(goal))
          ++nr_optimisable;
        else
          ++nr_unoptimisable;
      }

    if (nr_optimisable>0 && nr_unoptimisable==0)
    {
      for (goal = 0; goal!=nr_goals; ++goal)
        if (state->imminent_goals[goal])
          insert_goal_optimisation_help_filter(proxy1,goal);

      pipe_append(SLICE_PREV(proxy_joint),
                  alloc_goal_prerequisite_optimiser_slice());
      pipe_append(SLICE_PREV(proxy_joint),alloc_pipe(STDeadEnd));
    }

    if (nr_optimisable>0)
    {
      slice_set_predecessor(proxy_joint,no_slice);
      pipe_link(proxy_joint,SLICE_NEXT1(si));
      pipe_link(si,alloc_fork_on_remaining_slice(proxy2,proxy1,1));
    }
    else
    {
      dealloc_slices(proxy1);
      dealloc_slice(proxy2);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert goal prerequisite guards
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void insert_goal_prerequisite_guards(slice_index si,
                                            stip_moves_traversal *st)
{
  prerequisite_guards_insertion_state * const state = st->param;
  prerequisite_guards_insertion_state const save_state = *state;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_children(si,st);

  if (st->remaining==1)
    switch (st->context)
    {
      case stip_traversal_context_attack:
      case stip_traversal_context_defense:
        insert_goal_prerequisite_guards_battle(si,state,st->context);
        break;

      case stip_traversal_context_help:
        insert_goal_prerequisite_guards_help(si,state);
        break;

      default:
        assert(0);
        break;
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
  { STTestingPrerequisites,         &insert_goal_prerequisite_guards             },
  { STGoalDoubleMateReachedTester,  &insert_goal_prerequisite_guards_doublemate  },
  { STGoalCounterMateReachedTester, &insert_goal_prerequisite_guards_countermate }
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
void goal_prerequisite_guards_initialse_solving(slice_index si)
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
