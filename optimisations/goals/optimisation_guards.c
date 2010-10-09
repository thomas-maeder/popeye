#include "optimisations/goals/optimisation_guards.h"
#include "pystip.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "optimisations/goals/enpassant/attacker_filter.h"
#include "optimisations/goals/enpassant/defender_filter.h"
#include "optimisations/goals/enpassant/help_filter.h"
#include "optimisations/goals/castling/attacker_filter.h"
#include "optimisations/goals/castling/help_filter.h"
#include "optimisations/goals/castling/series_filter.h"

#include "trace.h"

#include <assert.h>

static void insert_goal_optimisation_attacker_filter(slice_index si, Goal goal)
{
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal.type);
  TraceFunctionParamListEnd();

  switch (goal.type)
  {
    case goal_ep:
      pipe_append(slices[si].prev,
                  alloc_enpassant_attacker_filter_slice(length,min_length));
      break;

    case goal_castling:
      pipe_append(slices[si].prev,
                  alloc_castling_attacker_filter_slice(length,min_length));
      break;

    default:
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_goal_optimisation_defender_filter(slice_index si, Goal goal)
{
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal.type);
  TraceFunctionParamListEnd();

  switch (goal.type)
  {
    case goal_ep:
      pipe_append(slices[si].prev,
                  alloc_enpassant_defender_filter_slice(length,min_length));
      break;

    case goal_castling:
      /* intentionally nothing (assuming that castling can't be
       * forced)
       */
      break;

    default:
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_goal_optimisation_help_filter(slice_index si, Goal goal)
{
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal.type);
  TraceFunctionParamListEnd();

  switch (goal.type)
  {
    case goal_ep:
      pipe_append(slices[si].prev,
                  alloc_enpassant_help_filter_slice(length,min_length));
      break;

    case goal_castling:
      pipe_append(slices[si].prev,
                  alloc_castling_help_filter_slice(length,min_length));
      break;

    default:
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_goal_optimisation_series_filter(slice_index si, Goal goal)
{
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal.type);
  TraceFunctionParamListEnd();

  switch (goal.type)
  {
    case goal_ep:
      /* intentionally nothing (assuming that ser-ep is impossible)
       */
      break;

    case goal_castling:
      pipe_append(slices[si].prev,
                  alloc_castling_series_filter_slice(length,min_length));
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
    boolean is_optimised[max_nr_slices];
} optimisation_guards_insertion_state;

/* Instrument the stipulation structure with goal optimisation guards.
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static
void insert_goal_optimisation_guards_attack_to_goal(slice_index si,
                                                    stip_moves_traversal *st)
{
  optimisation_guards_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!state->is_optimised[si])
  {
    Goal const goal = slices[si].u.branch.imminent_goal;

    stip_traverse_moves_pipe(si,st);

    if (goal.type!=no_goal)
      insert_goal_optimisation_attacker_filter(si,goal);
    state->is_optimised[si] = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the stipulation structure with goal optimisation guards.
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static
void insert_goal_optimisation_guards_killer_defense(slice_index si,
                                                    stip_moves_traversal *st)
{
  optimisation_guards_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!state->is_optimised[si])
  {
    Goal const save_goal = state->goal;

    TraceValue("%u\n",st->remaining);
    if (st->remaining==slack_length_battle+1)
    {
      stip_traverse_moves_children(slices[si].u.pipe.next,st);
      if (state->goal.type!=no_goal)
        insert_goal_optimisation_defender_filter(si,state->goal);
      state->is_optimised[si] = true;
    }
    else
      stip_traverse_moves_children(slices[si].u.pipe.next,st);

    state->goal = save_goal;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the stipulation structure with goal optimisation guards.
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void insert_goal_optimisation_guards_defense(slice_index si,
                                                    stip_moves_traversal *st)
{
  optimisation_guards_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!state->is_optimised[si])
  {
    Goal const save_goal = state->goal;

    TraceValue("%u\n",st->remaining);
    if (st->remaining==slack_length_battle+1)
    {
      stip_traverse_moves_children(si,st);
      if (state->goal.type!=no_goal)
        insert_goal_optimisation_defender_filter(si,state->goal);
      state->is_optimised[si] = true;
    }
    else
      stip_traverse_moves_children(si,st);

    state->goal = save_goal;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the stipulation structure with goal optimisation guards.
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static
void insert_goal_optimisation_guards_defense_fork(slice_index si,
                                                  stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->remaining==slack_length_battle+1)
    stip_traverse_moves(slices[si].u.branch_fork.towards_goal,st);
  else
    stip_traverse_moves(slices[si].u.branch_fork.next,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert goal prerequisite guards
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static
void insert_goal_optimisation_guards_help_fork(slice_index si,
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

/* Instrument the stipulation structure with goal optimisation guards.
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void insert_goal_optimisation_guards_help_move(slice_index si,
                                                      stip_moves_traversal *st)
{
  optimisation_guards_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!state->is_optimised[si])
  {
    Goal const save_goal = state->goal;

    stip_traverse_moves_move_slice(si,st);

    if (st->remaining<slack_length_help+2)
    {
      if (state->goal.type!=no_goal)
        insert_goal_optimisation_help_filter(si,state->goal);
      state->is_optimised[si] = true;
    }

    state->goal = save_goal;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the stipulation structure with goal optimisation guards.
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static
void
insert_goal_optimisation_guards_help_move_to_goal(slice_index si,
                                                  stip_moves_traversal *st)
{
  optimisation_guards_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!state->is_optimised[si])
  {
    Goal const save_goal = state->goal;

    stip_traverse_moves_move_slice(si,st);

    assert(st->remaining==slack_length_help+1);

    {
      Goal const goal = slices[si].u.branch.imminent_goal;
      if (goal.type!=no_goal)
        insert_goal_optimisation_help_filter(si,goal);
      state->is_optimised[si] = true;
    }

    state->goal = save_goal;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the stipulation structure with goal optimisation guards.
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static
 void insert_goal_optimisation_guards_series_move(slice_index si,
                                                  stip_moves_traversal *st)
{
  optimisation_guards_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!state->is_optimised[si])
  {
    Goal const save_goal = state->goal;

    stip_traverse_moves_move_slice(si,st);

    if (st->remaining==slack_length_series+1)
    {
      Goal const goal = slices[si].u.branch.imminent_goal;
      if (goal.type!=no_goal)
        insert_goal_optimisation_series_filter(si,goal);
      state->is_optimised[si] = true;
    }

    state->goal = save_goal;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the stipulation structure with goal optimisation guards.
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void insert_goal_optimisation_guards_goal(slice_index si,
                                                 stip_moves_traversal *st)
{
  optimisation_guards_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->goal = slices[si].u.goal_reached_tester.goal;
  TraceValue("->%u\n",state->goal.type);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the stipulation structure with goal optimisation guards.
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static
void insert_goal_optimisation_guards_goal_non_target(slice_index si,
                                                     stip_moves_traversal *st)
{
  optimisation_guards_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->goal.type = goal_mate+(slices[si].type-STGoalMateReachedTester);
  TraceValue("->%u\n",state->goal.type);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the stipulation structure with goal optimisation guards.
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void insert_goal_optimisation_guards_goal_target(slice_index si,
                                                        stip_moves_traversal *st)
{
  optimisation_guards_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->goal.type = goal_target;
  state->goal.target = slices[si].u.goal_reached_tester.goal.target;
  TraceValue("->%u\n",state->goal.type);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static moves_traversers_visitors const optimisation_guard_inserters[] =
{
  { STDefenseFork,                      &insert_goal_optimisation_guards_defense_fork      },
  { STKillerMoveFinalDefenseMove,       &insert_goal_optimisation_guards_killer_defense    },
  { STDefenseMove,                      &insert_goal_optimisation_guards_defense           },
  { STAttackMoveToGoal,                 &insert_goal_optimisation_guards_attack_to_goal    },
  { STGoalReachedTester,                &insert_goal_optimisation_guards_goal              },
  { STGoalMateReachedTester,            &insert_goal_optimisation_guards_goal_non_target   },
  { STGoalStalemateReachedTester,       &insert_goal_optimisation_guards_goal_non_target   },
  { STGoalDoubleStalemateReachedTester, &insert_goal_optimisation_guards_goal_non_target   },
  { STGoalTargetReachedTester,          &insert_goal_optimisation_guards_goal_target       },
  { STGoalCheckReachedTester,           &insert_goal_optimisation_guards_goal_non_target   },
  { STGoalCaptureReachedTester,         &insert_goal_optimisation_guards_goal_non_target   },
  { STGoalSteingewinnReachedTester,     &insert_goal_optimisation_guards_goal_non_target   },
  { STGoalEnpassantReachedTester,       &insert_goal_optimisation_guards_goal_non_target   },
  { STGoalDoubleMateReachedTester,      &insert_goal_optimisation_guards_goal_non_target   },
  { STGoalCounterMateReachedTester,     &insert_goal_optimisation_guards_goal_non_target   },
  { STGoalCastlingReachedTester,        &insert_goal_optimisation_guards_goal_non_target   },
  { STGoalAutoStalemateReachedTester,   &insert_goal_optimisation_guards_goal_non_target   },
  { STHelpFork,                         &insert_goal_optimisation_guards_help_fork         },
  { STHelpMove,                         &insert_goal_optimisation_guards_help_move         },
  { STHelpMoveToGoal,                   &insert_goal_optimisation_guards_help_move_to_goal },
  { STSeriesMoveToGoal,                 &insert_goal_optimisation_guards_series_move       }
};

enum
{
  nr_optimisation_guard_inserters
  = (sizeof optimisation_guard_inserters
     / sizeof optimisation_guard_inserters[0])
};

/* Instrument the stipulation structure with goal optimisation guards.
 * These guards stop solving if the following move has to reach a
 * goal, but the prerequisites for that goal (if any) aren't met
 * before the move.
 * @param si identifies slice where to start
 */
void stip_insert_goal_optimisation_guards(slice_index si)
{
  stip_moves_traversal st;
  optimisation_guards_insertion_state state = { { no_goal, initsquare },
                                                { false } };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_moves_traversal_init(&st,
                            optimisation_guard_inserters,
                            nr_optimisation_guard_inserters,
                            &state);
  stip_traverse_moves(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
