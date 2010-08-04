#include "stipulation/goals/prerequisite_guards.h"
#include "pystip.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/goals/countermate/attacker_filter.h"
#include "stipulation/goals/countermate/help_filter.h"
#include "stipulation/goals/countermate/series_filter.h"
#include "stipulation/goals/doublemate/attacker_filter.h"
#include "stipulation/goals/doublemate/help_filter.h"
#include "stipulation/goals/doublemate/series_filter.h"

#include "trace.h"

void insert_goal_prerequisite_guard_attack_move(slice_index si, Goal goal)
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
      pipe_append(slices[si].prev,
                  alloc_doublemate_attacker_filter_slice(length,min_length));
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
      pipe_append(slices[si].prev,
                  alloc_doublemate_help_filter_slice(length,min_length));
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
      pipe_append(slices[si].prev,
                  alloc_doublemate_series_filter_slice(length,min_length));
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
    Goal goal;
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
    Goal const save_goal = state->goal;

    stip_traverse_moves_branch_slice(si,st);

    if (st->remaining<=slack_length_battle+2)
    {
      if (state->goal.type!=no_goal)
        insert_goal_prerequisite_guard_attack_move(si,state->goal);
      state->is_provided[si] = true;
    }

    state->goal = save_goal;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert goal prerequisite guards
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static
void insert_goal_prerequisite_guards_attack_root(slice_index si,
                                                 stip_moves_traversal *st)
{
  prerequisite_guards_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_branch_slice(si,st);

  if (slices[si].u.branch.min_length==slack_length_battle+1)
  {
    stip_length_type const save_remaining = st->remaining;
    st->remaining = slack_length_battle+1;
    stip_traverse_moves_branch_slice(si,st);
    st->remaining = save_remaining;
    if (state->goal.type!=no_goal)
      insert_goal_prerequisite_guard_attack_move(si,state->goal);
    state->goal.type = no_goal;
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
    Goal const save_goal = state->goal;

    stip_traverse_moves_branch_slice(si,st);

    if (st->remaining==slack_length_help+1)
    {
      if (state->goal.type!=no_goal)
        insert_goal_prerequisite_guard_help_move(si,state->goal);
      state->is_provided[si] = true;
    }

    state->goal = save_goal;
  }

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
    Goal const save_goal = state->goal;

    stip_traverse_moves_branch_slice(si,st);

    if (st->remaining==slack_length_series+1)
    {
      if (state->goal.type!=no_goal)
        insert_goal_prerequisite_guard_series_move(si,state->goal);
      state->is_provided[si] = true;
    }

    state->goal = save_goal;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert goal prerequisite guards
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void insert_goal_prerequisite_guards_goal(slice_index si,
                                                 stip_moves_traversal *st)
{
  prerequisite_guards_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->goal = slices[si].u.goal_reached_tester.goal;
  TraceValue("->%u\n",state->goal.type);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* No provision for defense moves so far.
 * The goals currently supported can't be forced.
 */
static moves_traversers_visitors const prerequisite_guard_inserters[] =
{
  { STAttackEnd,         &insert_goal_prerequisite_guards_attack_move },
  { STGoalReachedTester, &insert_goal_prerequisite_guards_goal        },
  { STAttackRoot,        &insert_goal_prerequisite_guards_attack_root },
  { STHelpMoveToGoal,    &insert_goal_prerequisite_guards_help_move   },
  { STSeriesMoveToGoal,  &insert_goal_prerequisite_guards_series_move }
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
  prerequisite_guards_insertion_state state = { { no_goal, initsquare },
                                                { false } };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_moves_traversal_init(&st,
                            prerequisite_guard_inserters,
                            nr_prerequisite_guard_inserters,
                            &state);
  stip_traverse_moves(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
