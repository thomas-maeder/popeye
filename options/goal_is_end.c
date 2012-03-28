#include "options/goal_is_end.h"
#include "stipulation/proxy.h"
#include "stipulation/boolean/not.h"
#include "stipulation/battle_play/branch.h"
#include "trace.h"

#include <assert.h>

typedef struct
{
    goal_type the_goal;
    boolean goal_found[nr_goals];
    unsigned int nr_unique_goals_found;
} goal_is_end_one_insertion_state_type;

static goal_is_end_one_insertion_state_type const nil_state = { no_goal, { false }, 0 };

typedef struct
{
    boolean inserted;
    goal_is_end_one_insertion_state_type current;
} goal_is_end_insertion_state_type;

static void remember_goal(slice_index si, stip_structure_traversal *st)
{
  goal_is_end_insertion_state_type * const state = st->param;
  goal_type const type = slices[si].u.goal_handler.goal.type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (type==goal_negated)
    state->current.nr_unique_goals_found = 2;
  else if (!state->current.goal_found[type])
  {
    state->current.goal_found[type] = true;
    ++state->current.nr_unique_goals_found;
    state->current.the_goal = type;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_goal_is_end_tester(slice_index si, stip_structure_traversal *st)
{
  goal_is_end_insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure(slices[si].u.fork.fork,st);

  if (st->context==stip_traversal_context_defense
      && state->current.nr_unique_goals_found==1)
  {
    slice_index const goal_branch_copy = stip_deep_copy(slices[si].u.fork.fork);
    slice_index const not = alloc_not_slice();
    slice_index const proxy = alloc_proxy_slice();
    pipe_link(proxy,not);
    pipe_link(not,goal_branch_copy);
    battle_branch_insert_attack_constraint(si,proxy);
    state->inserted = true;
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
