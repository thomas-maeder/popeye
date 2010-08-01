#include "optimisations/goals/optimisation_guards.h"
#include "pystip.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "optimisations/goals/enpassant/attacker_filter.h"
#include "optimisations/goals/enpassant/defender_filter.h"
#include "optimisations/goals/castling/attacker_filter.h"

#include "trace.h"

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
      pipe_append(si,alloc_enpassant_defender_filter_slice(length,min_length));
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
void insert_goal_optimisation_guards_attack_move(slice_index si,
                                                 stip_moves_traversal *st)
{
  optimisation_guards_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!state->is_optimised[si])
  {
    Goal const save_goal = state->goal;

    stip_traverse_moves_branch_slice(si,st);

    if (st->remaining<=slack_length_battle+2)
    {
      if (state->goal.type!=no_goal)
        insert_goal_optimisation_attacker_filter(si,state->goal);
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
void insert_goal_optimisation_guards_defense_end(slice_index si,
                                                 stip_moves_traversal *st)
{
  optimisation_guards_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!state->is_optimised[si])
  {
    Goal const save_goal = state->goal;

    stip_traverse_moves_defense_end(si,st);

    if (st->remaining<=slack_length_battle+2)
    {
      if (state->goal.type!=no_goal)
        insert_goal_optimisation_defender_filter(si,state->goal);
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
void insert_goal_optimisation_guards_attack_root(slice_index si,
                                                 stip_moves_traversal *st)
{
  optimisation_guards_insertion_state * const state = st->param;

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
    insert_goal_optimisation_attacker_filter(si,state->goal);
    state->goal.type = no_goal;
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

static moves_traversers_visitors const optimisation_guard_inserters[] =
{
  { STAttackEnd,         &insert_goal_optimisation_guards_attack_move },
  { STGoalReachedTester, &insert_goal_optimisation_guards_goal },
  { STAttackRoot,        &insert_goal_optimisation_guards_attack_root },
  { STDefenseEnd,        &insert_goal_optimisation_guards_defense_end }
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
 */
void stip_insert_goal_optimisation_guards(void)
{
  stip_moves_traversal st;
  optimisation_guards_insertion_state state = { { no_goal, initsquare },
                                                { false } };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  stip_moves_traversal_init(&st,
                            optimisation_guard_inserters,
                            nr_optimisation_guard_inserters,
                            &state);
  stip_traverse_moves(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
