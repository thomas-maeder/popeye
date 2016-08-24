#include "solving/dead_end.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "solving/has_solution_type.h"
#include "solving/avoid_unsolvable.h"
#include "solving/moves_traversal.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

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
void dead_end_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(solve_nr_remaining>=previous_move_has_solved);

  TraceValue("%u",max_unsolvable);
  TraceEOL();
  pipe_this_move_doesnt_solve_if(si,
                                 solve_nr_remaining<=max_unsolvable
                                 || solve_nr_remaining<next_move_has_solution);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
    slice_index end_of_branch_goal;
    slice_index optimisable_deadend;
} optimisation_state;

static optimisation_state const null_optimisation_state = { no_slice, no_slice };

static void optimise_deadend_ready(slice_index si, stip_moves_traversal *st)
{
  optimisation_state * const state = st->param;
  optimisation_state const save_state = *state;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *state = null_optimisation_state;
  stip_traverse_moves_children(si,st);
  *state = save_state;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void substitute_deadend_goal(slice_index si, stip_moves_traversal *st)
{
  optimisation_state * const state = st->param;
  slice_index const save_optimisable_deadend = state->optimisable_deadend;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_children(si,st);

  TraceValue("%u",state->optimisable_deadend);
  TraceValue("%u",state->end_of_branch_goal);
  TraceValue("%u",st->context);
  TraceEOL();
  if (state->optimisable_deadend!=no_slice
      && state->end_of_branch_goal!=no_slice
      && st->context!=stip_traversal_context_attack)
  {
    slice_index const prototype = alloc_pipe(STDeadEndGoal);
    defense_branch_insert_slices(si,&prototype,1);
    pipe_remove(state->optimisable_deadend);
  }

  state->optimisable_deadend = save_optimisable_deadend;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_end_of_branch(slice_index si, stip_moves_traversal *st)
{
  optimisation_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->remaining==0)
    state->end_of_branch_goal = si;

  stip_traverse_moves_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_deadend(slice_index si, stip_moves_traversal *st)
{
  optimisation_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->remaining==0)
    state->optimisable_deadend = si;
  else
    stip_traverse_moves_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forget_deadend(slice_index si, stip_moves_traversal *st)
{
  optimisation_state * const state = st->param;
  slice_index const save_optimisable_deadend = state->optimisable_deadend;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",state->optimisable_deadend);
  TraceEOL();
  stip_traverse_moves_children(si,st);
  TraceValue("%u",state->optimisable_deadend);
  TraceEOL();
  state->optimisable_deadend = save_optimisable_deadend;
  TraceValue("->%u",state->optimisable_deadend);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static moves_traversers_visitors const dead_end_optimisers[] =
{
  { STReadyForAttack,          &optimise_deadend_ready  },
  { STReadyForHelpMove,        &optimise_deadend_ready  },
  { STEndOfBranchGoal,         &remember_end_of_branch  },
  { STEndOfBranchGoalImmobile, &remember_end_of_branch  },
  { STNotEndOfBranchGoal,      &substitute_deadend_goal },
  { STDeadEnd,                 &remember_deadend        },
  { STDefenseAdapter,          &forget_deadend          }
};

enum
{
  nr_dead_end_optimisers =
  (sizeof dead_end_optimisers / sizeof dead_end_optimisers[0])
};

/* Optimise away redundant deadend slices
 * @param si identifies the entry slice
 */
void solving_optimise_dead_end_slices(slice_index si)
{
  stip_moves_traversal mt;
  optimisation_state state = null_optimisation_state;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_moves_traversal_init(&mt,&state);
  stip_moves_traversal_override_by_contextual(&mt,
                                              slice_contextual_end_of_branch,
                                              &forget_deadend);
  stip_moves_traversal_override(&mt,dead_end_optimisers,nr_dead_end_optimisers);
  stip_traverse_moves(si,&mt);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
