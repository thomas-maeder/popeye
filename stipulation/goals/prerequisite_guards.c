#include "stipulation/goals/prerequisite_guards.h"
#include "pystip.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/goals/countermate/attacker_filter.h"
#include "stipulation/goals/countermate/help_filter.h"
#include "stipulation/goals/countermate/series_filter.h"
#include "stipulation/goals/doublemate/filter.h"

#include "trace.h"

#include <assert.h>

void insert_goal_prerequisite_guard_attacker_filter(slice_index si, Goal goal)
{
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal.type);
  TraceFunctionParamListEnd();

  switch (goal.type)
  {
    case goal_doublemate:
      pipe_append(slices[si].prev,alloc_doublemate_filter_slice());
      break;

    case goal_countermate:
      pipe_append(slices[si].prev,
                  alloc_countermate_attacker_filter_slice(length,min_length));
      break;

    default:
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void insert_goal_prerequisite_guard_help_move(slice_index si, Goal goal)
{
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal.type);
  TraceFunctionParamListEnd();

  switch (goal.type)
  {
    case goal_doublemate:
      pipe_append(slices[si].prev,alloc_doublemate_filter_slice());
      break;

    case goal_countermate:
      pipe_append(slices[si].prev,
                  alloc_countermate_help_filter_slice(length,min_length));
      break;

    default:
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void insert_goal_prerequisite_guard_series_move(slice_index si, Goal goal)
{
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal.type);
  TraceFunctionParamListEnd();

  switch (goal.type)
  {
    case goal_doublemate:
      pipe_append(slices[si].prev,alloc_doublemate_filter_slice());
      break;

    case goal_countermate:
       pipe_append(slices[si].prev,
                   alloc_countermate_series_filter_slice(length,min_length));
      break;

    default:
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
    Goal imminent;
    boolean is_provided[max_nr_slices];
} prerequisite_guards_insertion_state;

/* Insert goal prerequisite guards
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static
void insert_goal_prerequisite_guards_attack_move(slice_index si,
                                                 stip_moves_traversal *st)
{
  prerequisite_guards_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!state->is_provided[si])
  {
    Goal const goal = slices[si].u.branch.imminent_goal;

    stip_traverse_moves_move_slice(si,st);

    if (goal.type!=no_goal)
      insert_goal_prerequisite_guard_attacker_filter(si,goal);

    state->is_provided[si] = true;
    state->imminent.type = no_goal;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert goal prerequisite guards
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static
void
insert_goal_prerequisite_guards_help_move_to_goal(slice_index si,
                                                  stip_moves_traversal *st)
{
  prerequisite_guards_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!state->is_provided[si])
  {
    stip_traverse_moves_move_slice(si,st);

    assert(st->remaining==slack_length_help+1);

    {
      Goal const goal = slices[si].u.branch.imminent_goal;
      if (goal.type!=no_goal)
        insert_goal_prerequisite_guard_help_move(si,goal);
      state->is_provided[si] = true;
    }

    state->imminent.type = no_goal;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert goal prerequisite guards
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static
void insert_goal_prerequisite_guards_help_move(slice_index si,
                                               stip_moves_traversal *st)
{
  prerequisite_guards_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!state->is_provided[si])
  {
    stip_traverse_moves_move_slice(si,st);

    if (st->remaining<slack_length_help+2)
    {
      if (state->imminent.type!=no_goal)
        insert_goal_prerequisite_guard_help_move(si,state->imminent);
      state->is_provided[si] = true;
    }

    state->imminent.type = no_goal;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert goal prerequisite guards
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static
void insert_goal_prerequisite_guards_help_fork(slice_index si,
                                               stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->remaining<slack_length_help+2)
  {
    stip_length_type const save_remaining = st->remaining;
    st->remaining = 0;
    stip_traverse_moves(slices[si].u.branch_fork.towards_goal,st);
    st->remaining = save_remaining;
  }
  else
    stip_traverse_moves_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert goal prerequisite guards
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static
void insert_goal_prerequisite_guards_series_move(slice_index si,
                                                 stip_moves_traversal *st)
{
  prerequisite_guards_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!state->is_provided[si])
  {
    stip_traverse_moves_move_slice(si,st);

    TraceValue("%u\n",st->remaining);
    if (st->remaining==slack_length_series+1)
    {
      Goal const goal = slices[si].u.branch.imminent_goal;
      if (goal.type!=no_goal)
        insert_goal_prerequisite_guard_series_move(si,goal);
      state->is_provided[si] = true;
    }

    state->imminent.type = no_goal;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert goal prerequisite guards
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static
void
insert_goal_prerequisite_guards_goal_doublemate_tester(slice_index si,
                                                       stip_moves_traversal *st)
{
  prerequisite_guards_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_branch_slice(si,st);
  state->imminent.type = goal_doublemate;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert goal prerequisite guards
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static
void
insert_goal_prerequisite_guards_goal_countermate_tester(slice_index si,
                                                        stip_moves_traversal *st)
{
  prerequisite_guards_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_branch_slice(si,st);
  state->imminent.type = goal_countermate;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* No provision for defense moves so far.
 * The goals currently supported can't be forced.
 */
static moves_traversers_visitors const prerequisite_guard_inserters[] =
{
  { STAttackMoveToGoal,             &insert_goal_prerequisite_guards_attack_move             },
  { STHelpFork,                     &insert_goal_prerequisite_guards_help_fork               },
  { STHelpMove,                     &insert_goal_prerequisite_guards_help_move               },
  { STHelpMoveToGoal,               &insert_goal_prerequisite_guards_help_move_to_goal       },
  { STSeriesMoveToGoal,             &insert_goal_prerequisite_guards_series_move             },
  { STGoalDoubleMateReachedTester,  &insert_goal_prerequisite_guards_goal_doublemate_tester  },
  { STGoalCounterMateReachedTester, &insert_goal_prerequisite_guards_goal_countermate_tester }
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
  stip_moves_traversal st;
  prerequisite_guards_insertion_state state = { { no_goal, initsquare},
                                                { false } };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_moves_traversal_init(&st,&state);
  stip_moves_traversal_override(&st,
                                prerequisite_guard_inserters,
                                nr_prerequisite_guard_inserters);
  stip_traverse_moves(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
