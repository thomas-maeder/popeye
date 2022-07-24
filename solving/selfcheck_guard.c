#include "solving/selfcheck_guard.h"
#include "stipulation/pipe.h"
#include "solving/has_solution_type.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/proxy.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/boolean/and.h"
#include "stipulation/boolean/true.h"
#include "stipulation/goals/goals.h"
#include "stipulation/goals/doublestalemate/reached_tester.h"
#include "stipulation/goals/slice_insertion.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "solving/check.h"
#include "solving/pipe.h"
#include "conditions/conditions.h"
#include "debugging/trace.h"
#include "debugging/assert.h"


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

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void selfcheck_guard_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (is_in_check(advers(SLICE_STARTER(si))))
    solve_result = previous_move_is_illegal;
  else
    pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
    SLICE_STARTER(prototype) = SLICE_STARTER(si);
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
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
  slice_index const tester = SLICE_NEXT2(si);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->last_checked!=SLICE_STARTER(si)
      && get_stip_structure_traversal_state(tester,st)==slice_not_traversed)
  {
    boolean const save_is_instrumented = state->is_branch_instrumented;

    state->in_goal_tester = SLICE_U(si).goal_handler.goal.type;
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
    /* make sure that not_slice doesn't convert previous_move_has_not_solved into
     * previous_move_has_solved if the last move caused a self-check
     */
    slice_index const proxy_not = alloc_proxy_slice();
    slice_index const proxy_selfcheck = alloc_proxy_slice();
    slice_index const guard = alloc_selfcheck_guard_slice();
    slice_index const leaf_selfcheck = alloc_true_slice();
    if (is_goal_move_oriented(SLICE_NEXT1(si),st))
      pipe_link(SLICE_PREV(si),alloc_and_slice(proxy_not,proxy_selfcheck));
    else
      pipe_link(SLICE_PREV(si),alloc_and_slice(proxy_selfcheck,proxy_not));
    pipe_link(proxy_not,si);
    pipe_link(proxy_selfcheck,guard);
    pipe_link(guard,leaf_selfcheck);
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
    goal_branch_insert_slices(SLICE_NEXT1(si),&prototype,1);
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

  if (SLICE_U(si).goal_filter.applies_to_who==goal_applies_to_adversary)
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

  assert(SLICE_STARTER(si)!=no_side);
  state->last_checked = SLICE_STARTER(si);
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
  { STKingCaptureLegalityTester,       &stip_traverse_structure_children_pipe    },
  { STMoveLegalityTester,              &stip_traverse_structure_children_pipe    },
  { STCageCirceNonCapturingMoveFinder, &suspend_insertion                        },
  { STAnd,                             &instrument_doublestalemate_tester        },
  { STNot,                             &instrument_negated_tester                },
  { STGoalCheckReachedTester,          &dont_instrument_selfcheck_ignoring_goals },
  { STSelfCheckGuard,                  &remember_last_checked                    },
  { STMoveInverter,                    &invert_last_checked                      },
  { STMoveInverterSetPlay,             &invert_last_checked                      },
  { STAttackPlayed,                    &forget_last_checked                      },
  { STDefensePlayed,                   &forget_last_checked                      },
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

  insert_in_branch_guards(SLICE_NEXT2(si));

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
    slice_insertion_insert(si,&prototype,1);
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
  state->guard_needed = state->last_guarded_side!=no_side && SLICE_STARTER(si)!=state->last_guarded_side;

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

  *side = SLICE_STARTER(si);
  stip_traverse_structure_children_pipe(si,st);
  *side = save_side;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor adapters_guards_inserters[] =
{
  { STAttackAdapter,       &determine_need_for_move_inverter_instrumentation },
  { STDefenseAdapter,      &determine_need_for_move_inverter_instrumentation },
  { STHelpAdapter,         &determine_need_for_move_inverter_instrumentation },
  { STSelfCheckGuard,      &remember_checked_side                            },
  { STMoveInverter,        &instrument_move_inverter                         },
  { STMoveInverterSetPlay, &instrument_move_inverter                         },
  { STTemporaryHackFork,   &stip_traverse_structure_children_pipe            }
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

  if (!CondFlag[lesemajeste])
  {
    slice_index const prototype = alloc_selfcheck_guard_slice();
    slice_insertion_insert(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with slices dealing with selfcheck detection
 * @param si identifies the root of the solving machinery
 */
void solving_insert_selfcheck_guards(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  solving_impose_starter(si,SLICE_STARTER(si));
  insert_in_branch_guards(si);
  solving_impose_starter(si,SLICE_STARTER(si));
  insert_guards_in_immobility_testers(si);
  solving_impose_starter(si,SLICE_STARTER(si));
  instrument_move_inverters(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
