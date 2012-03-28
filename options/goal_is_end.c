#include "options/goal_is_end.h"
#include "stipulation/proxy.h"
#include "stipulation/boolean/not.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "trace.h"

#include <assert.h>

typedef struct
{
    Goal the_goal;
    boolean goal_found[nr_goals];
    unsigned int nr_unique_goals_found;
} goal_is_end_one_insertion_state_type;

static goal_is_end_one_insertion_state_type const nil_state = { { no_goal, initsquare }, { false }, 0 };

typedef struct
{
    boolean inserted;
    goal_is_end_one_insertion_state_type current;
} goal_is_end_insertion_state_type;

static void remember_goal(slice_index si, stip_structure_traversal *st)
{
  goal_is_end_insertion_state_type * const state = st->param;
  Goal const goal = slices[si].u.goal_handler.goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (goal.type==goal_negated)
    state->current.nr_unique_goals_found = 2;
  else if (!state->current.goal_found[goal.type])
  {
    state->current.goal_found[goal.type] = true;
    ++state->current.nr_unique_goals_found;
    state->current.the_goal = goal;
  }
  else if (goal.type==goal_target && goal.target!=state->current.the_goal.target)
    state->current.nr_unique_goals_found = 2;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_index make_goal_is_end_tester(slice_index si)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const not = alloc_not_slice();
    result = alloc_proxy_slice();
    pipe_link(result,not);
    pipe_link(not,stip_deep_copy(si));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_goal_is_end_tester(slice_index si, stip_structure_traversal *st)
{
  goal_is_end_insertion_state_type * const state = st->param;
  slice_index const fork = slices[si].u.fork.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure(fork,st);

  if (state->current.nr_unique_goals_found==1)
    switch (st->context)
    {
      case stip_traversal_context_attack:
        battle_branch_insert_defense_constraint(si,make_goal_is_end_tester(fork));
        state->inserted = true;
        break;

      case stip_traversal_context_defense:
        battle_branch_insert_attack_constraint(si,make_goal_is_end_tester(fork));
        state->inserted = true;
        break;

      case stip_traversal_context_help:
        if (help_branch_insert_constraint(si,make_goal_is_end_tester(fork),0))
          state->inserted = true;
        break;

      default:
        /* nothing */
        break;
    }

  state->current = nil_state;

  stip_traverse_structure_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation for option GoalIsEnd
 * @param root_slice identifies root slice of stipulation
 * @return true iff option GoalIsEnd is applicable
 */
boolean stip_insert_goal_is_end_testers(slice_index root_slice)
{
  goal_is_end_insertion_state_type state = { false, nil_state };
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root_slice);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_single(&st,STGoalReachedTester,&remember_goal);
  stip_structure_traversal_override_single(&st,STEndOfBranchGoal,&insert_goal_is_end_tester);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",state.inserted);
  TraceFunctionResultEnd();
  return state.inserted;
}
