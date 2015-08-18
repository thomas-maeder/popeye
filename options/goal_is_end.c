#include "options/goal_is_end.h"
#include "solving/machinery/slack_length.h"
#include "stipulation/proxy.h"
#include "stipulation/boolean/not.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

typedef struct
{
    slice_index tester;
    boolean goal_found[nr_goals];
    unsigned int nr_unique_goals_found;
} goal_is_end_one_search_state_type;

static void remember_goal(slice_index si, stip_structure_traversal *st)
{
  goal_is_end_one_search_state_type * const state = st->param;
  Goal const goal = SLICE_U(si).goal_handler.goal;
  Side const side = SLICE_STARTER(si);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (goal.type==goal_negated)
  {
    TraceText("negated\n");
    state->nr_unique_goals_found = 2;
  }
  else if (!state->goal_found[goal.type])
  {
    TraceText("new goal\n");
    state->goal_found[goal.type] = true;
    ++state->nr_unique_goals_found;
    state->tester = si;
  }
  else if (goal.type==goal_target
           && goal.target!=SLICE_U(state->tester).goal_handler.goal.target)
  {
    TraceText("different target\n");
    state->nr_unique_goals_found = 2;
  }
  else if (SLICE_STARTER(state->tester)!=side)
  {
    TraceText("different sides\n");
    state->nr_unique_goals_found = 2;
  }
  else
  {
    TraceText("repeated goal\n");
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void find_ending_goal(slice_index root_slice,
                             goal_is_end_one_search_state_type *state)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root_slice);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,state);
  stip_structure_traversal_override_single(&st,STGoalReachedTester,&remember_goal);
  stip_structure_traversal_override_single(&st,STTemporaryHackFork,&stip_traverse_structure_children_pipe);
  stip_traverse_structure(root_slice,&st);

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

typedef struct
{
    slice_index const tester;
    boolean inserted;
} goal_is_end_tester_insertion_state_type;

static void insert_goal_is_end_tester_battle(slice_index adapter,
                                             Side attacker_side,
                                             stip_structure_traversal *st)
{
  goal_is_end_tester_insertion_state_type * const state = st->param;
  Side const tester_side = SLICE_STARTER(state->tester);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(adapter,st);

  {
    slice_index const tester = make_goal_is_end_tester(state->tester);
    if (attacker_side==tester_side)
      battle_branch_insert_defense_goal_constraint(adapter,tester);
    else
      battle_branch_insert_attack_goal_constraint(adapter,tester);
  }

  state->inserted = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_goal_is_end_tester_attack(slice_index adapter,
                                             stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  insert_goal_is_end_tester_battle(adapter,SLICE_STARTER(adapter),st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_goal_is_end_tester_defense(slice_index adapter,
                                              stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  insert_goal_is_end_tester_battle(adapter,advers(SLICE_STARTER(adapter)),st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_goal_is_end_tester_help(slice_index adapter,
                                           stip_structure_traversal *st)
{
  goal_is_end_tester_insertion_state_type * const state = st->param;
  unsigned int const adapter_parity = (SLICE_U(adapter).branch.length-slack_length)%2;
  Side const adapter_side = SLICE_STARTER(adapter);
  Side const tester_side = SLICE_STARTER(state->tester);
  unsigned int const parity = adapter_side==tester_side ? adapter_parity : 1-adapter_parity;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(adapter,st);

  if (help_branch_insert_constraint(adapter,
                                    make_goal_is_end_tester(state->tester),
                                    parity))
    state->inserted = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation for option GoalIsEnd
 * @param root_slice identifies root slice of stipulation
 * @param tester identifies goal tester slice
 * @return true iff option GoalIsEnd is applicable
 */
static boolean insert_goal_is_end_testers(slice_index root_slice, slice_index tester)
{
  goal_is_end_tester_insertion_state_type state = { tester, false };
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root_slice);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_single(&st,STAttackAdapter,&insert_goal_is_end_tester_attack);
  stip_structure_traversal_override_single(&st,STDefenseAdapter,&insert_goal_is_end_tester_defense);
  stip_structure_traversal_override_single(&st,STHelpAdapter,&insert_goal_is_end_tester_help);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",state.inserted);
  TraceFunctionResultEnd();
  return state.inserted;
}

/* Instrument a stipulation for option GoalIsEnd
 * @param root_slice identifies root slice of stipulation
 * @return true iff option GoalIsEnd is applicable
 */
boolean stip_insert_goal_is_end_testers(slice_index root_slice)
{
  goal_is_end_one_search_state_type state = { no_slice, { false }, 0 };
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root_slice);
  TraceFunctionParamListEnd();

  stip_detect_starter(root_slice);
  solving_impose_starter(root_slice,SLICE_STARTER(root_slice));

  find_ending_goal(root_slice,&state);

  if (state.nr_unique_goals_found>1
      || SLICE_U(state.tester).goal_handler.goal.type==no_goal)
    result = false;
  else
    result = insert_goal_is_end_testers(root_slice,state.tester);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
