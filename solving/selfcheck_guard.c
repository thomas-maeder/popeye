#include "solving/selfcheck_guard.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/boolean/and.h"
#include "stipulation/boolean/true.h"
#include "stipulation/goals/goals.h"
#include "stipulation/goals/doublestalemate/reached_tester.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "pyproc.h"
#include "pydata.h"
#include "debugging/trace.h"

#include <assert.h>


/* Allocate a STSelfCheckGuard slice
 * @return allocated slice
 */
slice_index alloc_selfcheck_guard_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STSelfCheckGuard);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type selfcheck_guard_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (echecc(nbply,advers(slices[si].starter)))
    result = slack_length-2;
  else
    result = attack(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type selfcheck_guard_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (echecc(nbply,advers(slices[si].starter)))
    result = n+2;
  else
    result = defend(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

typedef struct
{
  goal_type in_goal_tester;
  boolean is_branch_instrumented;
  Side last_checked;
} in_branch_insertion_state_type;

static void insert_selfcheck_guard_branch(slice_index si,
                                          stip_structure_traversal *st)
{
  in_branch_insertion_state_type const * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!state->is_branch_instrumented)
  {
    slice_index const prototype = alloc_selfcheck_guard_slice();
    slices[prototype].starter = slices[si].starter;
    switch (st->context)
    {
      case stip_traversal_context_attack:
        attack_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_defense:
        defense_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_help:
        help_branch_insert_slices(si,&prototype,1);
        break;

      default:
        assert(0);
        break;
    }
  }

  stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_goal_not_move_oriented(slice_index si,
                                            stip_structure_traversal *st)
{
  boolean * const result = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor goal_move_oriented_testers[] =
{
  { STGoalCheckReachedTester,    &remember_goal_not_move_oriented },
  { STGoalImmobileReachedTester, &remember_goal_not_move_oriented },
  { STGoalNotCheckReachedTester, &remember_goal_not_move_oriented }
};

enum
{
  nr_goal_move_oriented_testers = (sizeof goal_move_oriented_testers
                                   / sizeof goal_move_oriented_testers[0])
};

static boolean is_goal_move_oriented(slice_index goal_reached_tester,
                                     stip_structure_traversal *st)
{
  boolean result = true;
  stip_structure_traversal st_nested;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal_reached_tester);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init_nested(&st_nested,st,&result);
  stip_structure_traversal_override(&st_nested,
                                    goal_move_oriented_testers,
                                    nr_goal_move_oriented_testers);
  stip_traverse_structure(goal_reached_tester,&st_nested);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_selfcheck_guard_goal(slice_index si,
                                        stip_structure_traversal *st)
{
  in_branch_insertion_state_type * const state = st->param;
  slice_index const tester = slices[si].next2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->last_checked!=slices[si].starter
      && get_stip_structure_traversal_state(tester,st)==slice_not_traversed)
  {
    boolean const save_is_instrumented = state->is_branch_instrumented;

    state->in_goal_tester = slices[si].u.goal_handler.goal.type;
    state->is_branch_instrumented = false;

    stip_traverse_structure_next_branch(si,st);

    if (!state->is_branch_instrumented)
    {
      slice_index const prototype = alloc_selfcheck_guard_slice();
      goal_branch_insert_slices(tester,&prototype,1);
    }

    state->is_branch_instrumented = save_is_instrumented;
    state->in_goal_tester = no_goal;
  }

  stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_negated_tester(slice_index si,
                                      stip_structure_traversal *st)
{
  in_branch_insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->in_goal_tester!=no_goal)
  {
    /* make sure that not_slice doesn't convert has_no_solution into
     * has_solution if the last move caused a self-check
     */
    slice_index const proxy_not = alloc_proxy_slice();
    slice_index const proxy_selfcheck = alloc_proxy_slice();
    slice_index const guard = alloc_selfcheck_guard_slice();
    slice_index const leaf_selfcheck = alloc_true_slice();
    if (is_goal_move_oriented(slices[si].next1,st))
      pipe_link(slices[si].prev,alloc_and_slice(proxy_not,proxy_selfcheck));
    else
      pipe_link(slices[si].prev,alloc_and_slice(proxy_selfcheck,proxy_not));
    pipe_link(proxy_not,si);
    pipe_link(proxy_selfcheck,guard);
    pipe_link(guard,leaf_selfcheck);

    state->is_branch_instrumented = true;
  }
  else
    stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_doublestalemate_tester(slice_index si,
                                              stip_structure_traversal *st)
{
  in_branch_insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (state->in_goal_tester==goal_dblstale)
  {
    slice_index const prototype = alloc_selfcheck_guard_slice();
    /* no need to instrument the operand that tests for stalemate of the
     * starting side */
    goal_branch_insert_slices(slices[si].next1,&prototype,1);
    state->is_branch_instrumented = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void dont_instrument_selfcheck_ignoring_goals(slice_index si,
                                                     stip_structure_traversal *st)
{
  in_branch_insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.goal_filter.applies_to_who==goal_applies_to_adversary)
    state->is_branch_instrumented = true;
  else
    stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void suspend_insertion(slice_index si, stip_structure_traversal *st)
{
  in_branch_insertion_state_type * const state = st->param;
  boolean const save_is_instrumented = state->is_branch_instrumented;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* prevent instrumentation of next1 branch */
  state->is_branch_instrumented = true;
  stip_traverse_structure_conditional_pipe_tester(si,st);
  state->is_branch_instrumented = save_is_instrumented;

  stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_last_checked(slice_index si, stip_structure_traversal *st)
{
  in_branch_insertion_state_type * const state = st->param;
  Side const save_last_checked = state->last_checked;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(slices[si].starter!=no_side);
  state->last_checked = slices[si].starter;
  stip_traverse_structure_children_pipe(si,st);
  state->last_checked = save_last_checked;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void invert_last_checked(slice_index si, stip_structure_traversal *st)
{
  in_branch_insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->last_checked==no_side)
    stip_traverse_structure_children_pipe(si,st);
  else
  {
    state->last_checked = advers(state->last_checked);
    stip_traverse_structure_children_pipe(si,st);
    state->last_checked = advers(state->last_checked);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forget_last_checked(slice_index si, stip_structure_traversal *st)
{
  in_branch_insertion_state_type * const state = st->param;
  Side const save_last_checked = state->last_checked;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->last_checked = no_side;
  stip_traverse_structure_children_pipe(si,st);
  state->last_checked = save_last_checked;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor in_branch_guards_inserters[] =
{
  { STNotEndOfBranchGoal,              &insert_selfcheck_guard_branch            },
  { STGoalReachedTester,               &insert_selfcheck_guard_goal              },
  { STCounterMateFilter,               &stip_traverse_structure_children_pipe    },
  { STIsardamDefenderFinder,           &stip_traverse_structure_children_pipe    },
  { STCageCirceNonCapturingMoveFinder, &suspend_insertion                        },
  { STAnd,                             &instrument_doublestalemate_tester        },
  { STNot,                             &instrument_negated_tester                },
  { STGoalCheckReachedTester,          &dont_instrument_selfcheck_ignoring_goals },
  { STSelfCheckGuard,                  &remember_last_checked                    },
  { STMoveInverter,                    &invert_last_checked                      },
  { STMovePlayed,                      &forget_last_checked                      },
  { STHelpMovePlayed,                  &forget_last_checked                      }
};

enum
{
  nr_in_branch_guards_inserters = (sizeof in_branch_guards_inserters
                                   / sizeof in_branch_guards_inserters[0])
};

static void insert_in_branch_guards(slice_index si)
{
  stip_structure_traversal st;
  in_branch_insertion_state_type state = { no_goal, false, no_side };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override(&st,
                                    in_branch_guards_inserters,
                                    nr_in_branch_guards_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_immobile_reached_tester(slice_index si,
                                               stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  insert_in_branch_guards(slices[si].next2);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_guards_in_immobility_testers(slice_index si)
{
  stip_structure_traversal st;
  boolean in_constraint = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&in_constraint);
  stip_structure_traversal_override_single(&st,
                                           STGoalImmobileReachedTester,
                                           &instrument_immobile_reached_tester);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
  Side last_guarded_side;
  boolean guard_needed;
} move_inverter_instrumentation_state_type;

static void instrument_move_inverter(slice_index si,
                                     stip_structure_traversal *st)
{
  move_inverter_instrumentation_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->guard_needed = false;

  stip_traverse_structure_children_pipe(si,st);

  if (state->guard_needed)
  {
    slice_index const prototype = alloc_selfcheck_guard_slice();
    branch_insert_slices(si,&prototype,1);
    state->guard_needed = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void determine_need_for_move_inverter_instrumentation(slice_index si,
                                                             stip_structure_traversal *st)
{
  move_inverter_instrumentation_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);
  state->guard_needed = state->last_guarded_side!=no_side && slices[si].starter!=state->last_guarded_side;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_checked_side(slice_index si,
                                        stip_structure_traversal *st)
{
  Side * const side = st->param;
  Side const save_side = *side;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *side = slices[si].starter;
  stip_traverse_structure_children_pipe(si,st);
  *side = save_side;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor adapters_guards_inserters[] =
{
  { STAttackAdapter,     &determine_need_for_move_inverter_instrumentation },
  { STDefenseAdapter,    &determine_need_for_move_inverter_instrumentation },
  { STHelpAdapter,       &determine_need_for_move_inverter_instrumentation },
  { STSelfCheckGuard,    &remember_checked_side                            },
  { STMoveInverter,      &instrument_move_inverter                         },
  { STTemporaryHackFork, &stip_traverse_structure_children_pipe            }
};

enum
{
  nr_adapters_guards_inserters = (sizeof adapters_guards_inserters
                                  / sizeof adapters_guards_inserters[0])
};

static void instrument_move_inverters(slice_index si)
{
  stip_structure_traversal st;
  move_inverter_instrumentation_state_type state = { no_side, false };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override(&st,
                                    adapters_guards_inserters,
                                    nr_adapters_guards_inserters);
  stip_traverse_structure(si,&st);

  {
    slice_index const prototype = alloc_selfcheck_guard_slice();
    branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation with slices dealing with selfcheck detection
 * @param si root of branch to be instrumented
 */
void stip_insert_selfcheck_guards(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_impose_starter(si,slices[si].starter);
  insert_in_branch_guards(si);
  insert_guards_in_immobility_testers(si);
  stip_impose_starter(si,slices[si].starter);
  instrument_move_inverters(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
