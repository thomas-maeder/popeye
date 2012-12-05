#include "optimisations/goals/mate/neutralretractable.h"
#include "pydata.h"
#include "pyproc.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/goals/goals.h"
#include "stipulation/moves_traversal.h"
#include "solving/fork_on_remaining.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

typedef struct
{
    Goal goal_to_be_reached;
    unsigned int nr_goals_to_be_reached;
    boolean notNecessarilyFinalMove;
} final_move_optimisation_state;

static final_move_optimisation_state const init_state = { { no_goal, initsquare }, 0, false };

static boolean is_goal_eligible(Goal goal)
{
  boolean result;

  switch (goal.type)
  {
    case goal_mate:
    case goal_doublemate:
      result = true;
      break;

    default:
      result = false;
      break;
  }

  return result;
}

/* Remember the goal imminent after a defense or solve move
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void optimise_final_moves_move_generator(slice_index si,
                                                stip_moves_traversal *st)
{
  final_move_optimisation_state * const state = st->param;
  final_move_optimisation_state const save_state = *state;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *state = init_state;

  stip_traverse_moves_children(si,st);

  if (st->context!=stip_traversal_context_defense
      && st->remaining==1
      && state->nr_goals_to_be_reached==1
      && !state->notNecessarilyFinalMove
      && is_goal_eligible(state->goal_to_be_reached))
  {
    slice_index const remover = alloc_pipe(STMateRemoveRetractable);

    assert(slices[slices[si].next1].type==STDoneRemovingFutileMoves);

    if (st->full_length<=2)
      pipe_append(si,remover);
    else
    {
      slice_index const proxy1 = alloc_proxy_slice();
      slice_index const proxy2 = alloc_proxy_slice();
      slice_index const fork = alloc_fork_on_remaining_slice(proxy1,proxy2,1);
      pipe_link(proxy1,slices[si].next1);
      pipe_link(proxy2,remover);
      pipe_set_successor(remover,slices[si].next1);
      pipe_link(si,fork);
    }
  }

  *state = save_state;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether there are more moves after this branch
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static
void optimise_final_moves_end_of_branch_non_goal(slice_index si,
                                                 stip_moves_traversal *st)
{
  final_move_optimisation_state * const state = st->param;
  unsigned int const save_nr_imminent_goals = state->nr_goals_to_be_reached;
  Goal const save_imminent_goal = state->goal_to_be_reached;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->nr_goals_to_be_reached = 0;
  state->goal_to_be_reached.type = no_goal;

  stip_traverse_moves_children(si,st);

  if (state->nr_goals_to_be_reached==0)
    state->notNecessarilyFinalMove = true;

  state->nr_goals_to_be_reached = save_nr_imminent_goals;
  state->goal_to_be_reached = save_imminent_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Forget a remembered goal because it is to be reached by a move
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void generator_swallow_goal(slice_index si, stip_moves_traversal *st)
{
  final_move_optimisation_state * const state = st->param;
  unsigned int const save_nr_imminent_goals = state->nr_goals_to_be_reached;
  Goal const save_imminent_goal = state->goal_to_be_reached;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_children(si,st);

  state->nr_goals_to_be_reached = save_nr_imminent_goals;
  state->goal_to_be_reached = save_imminent_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Remember the goal to be reached
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void optimise_final_moves_goal(slice_index si, stip_moves_traversal *st)
{
  final_move_optimisation_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_children(si,st);

  if (!are_goals_equal(state->goal_to_be_reached,
                       slices[si].u.goal_handler.goal))
  {
    state->goal_to_be_reached = slices[si].u.goal_handler.goal;
    ++state->nr_goals_to_be_reached;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void optimise_final_moves_suppress(slice_index si, stip_moves_traversal *st)
{
  final_move_optimisation_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_children(si,st);
  state->nr_goals_to_be_reached = 2;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static moves_traversers_visitors const final_move_optimisers[] =
{
  { STDoneRemovingIllegalMoves, &optimise_final_moves_move_generator },
  { STGoalReachedTester,        &optimise_final_moves_goal           },
  { STNot,                      &optimise_final_moves_suppress       }
};

enum
{
  nr_final_move_optimisers
  = (sizeof final_move_optimisers / sizeof final_move_optimisers[0])
};

/* Optimise move generation by inserting orthodox mating move generators
 * @param si identifies the root slice of the stipulation
 */
void stip_optimise_by_omitting_retractable_neutral_moves(slice_index si)
{
  stip_moves_traversal st;
  final_move_optimisation_state state = { { no_goal, initsquare }, 2, false };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_moves_traversal_init(&st,&state);
  stip_moves_traversal_override_by_function(&st,
                                            slice_function_move_generator,
                                            &generator_swallow_goal);
  stip_moves_traversal_override_by_function(&st,
                                            slice_function_end_of_branch,
                                            &optimise_final_moves_end_of_branch_non_goal);
  stip_moves_traversal_override(&st,
                                final_move_optimisers,
                                nr_final_move_optimisers);
  stip_traverse_moves(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean is_move_retractable(numecoup i)
{
  boolean result = false;
  square const sq_departure = move_generation_stack[i].departure;
  square const sq_capture = move_generation_stack[i].capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (TSTFLAG(spec[sq_departure],Neutral))
  {
     /* TODO can we optimise like this when we are not generating goal
      * reaching moves?
      */
    if (king_square[White]==king_square[Black])
    {
      if (king_square[White]==sq_departure)
      {
        if (e[sq_capture]==vide)
          result = true;
      }
      else if (abs(e[sq_departure])!=Pawn || e[sq_capture]==vide)
        result = true;
    }
    else if (e[sq_capture]==vide && abs(e[sq_departure])!=Pawn)
      result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type mate_remove_retractable_solve(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;
  numecoup i;
  numecoup new_top = current_move[nbply-1];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (i = current_move[nbply-1]+1; i<=current_move[nbply]; ++i)
    if (!is_move_retractable(i))
    {
      ++new_top;
      move_generation_stack[new_top] = move_generation_stack[i];
    }

  current_move[nbply] = new_top;

  result = solve(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
