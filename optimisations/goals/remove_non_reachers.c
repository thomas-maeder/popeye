#include "optimisations/goals/remove_non_reachers.h"
#include "conditions/conditions.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "solving/has_solution_type.h"
#include "stipulation/proxy.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/goals/goals.h"
#include "solving/moves_traversal.h"
#include "solving/fork_on_remaining.h"
#include "optimisations/goals/enpassant/remove_non_reachers.h"
#include "optimisations/goals/castling/remove_non_reachers.h"
#include "optimisations/goals/chess81/remove_non_reachers.h"
#include "optimisations/goals/capture/remove_non_reachers.h"
#include "optimisations/goals/target/remove_non_reachers.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

typedef struct
{
    Goal goal_to_be_reached;
    unsigned int nr_goals_to_be_reached;
    boolean notNecessarilyFinalMove;
} final_move_optimisation_state;

static final_move_optimisation_state const init_state = { { no_goal, initsquare }, 0, false };

static slice_index make_remover(Goal goal)
{
  switch (goal.type)
  {
    case goal_ep:
      return alloc_enpassant_remove_non_reachers_slice();

    case goal_castling:
      return alloc_castling_remove_non_reachers_slice();

    case goal_chess81:
      return alloc_chess81_remove_non_reachers_slice();

    case goal_capture:
    case goal_steingewinn:
      return alloc_capture_remove_non_reachers_slice();

    case goal_target:
      if (CondFlag[actrevolving]
          || CondFlag[arc]
          || CondFlag[wormholes]
          || CondFlag[anticirce]
          || CondFlag[series_capture])
        return no_slice;
      else
        return alloc_target_remove_non_reachers_slice(goal.target);

    default:
      return no_slice;
  }
}

static void stop_copying(slice_index si, stip_structure_traversal *st)
{
  stip_deep_copies_type * const copies = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  (*copies)[si] = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void optimise_last_move_generation(slice_index si,
                                          stip_traversal_context_type parent_context,
                                          slice_index remover)
{
  slice_index const proxy1 = alloc_proxy_slice();
  slice_index const proxy2 = alloc_proxy_slice();
  slice_index const fork = alloc_fork_on_remaining_slice(proxy1,proxy2,1);
  stip_structure_traversal st;
  stip_deep_copies_type copies;

  init_deep_copy(&st,0,&copies);
  stip_structure_traversal_override_single(&st,
                                           STDoneRemovingFutileMoves,
                                           &stop_copying);
  stip_traverse_structure(si,&st);

  pipe_link(SLICE_PREV(si),fork);
  pipe_link(proxy1,si);
  pipe_link(proxy2,copies[si]);

  slice_insertion_insert_contextually(copies[si],parent_context,&remover,1);
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
      && !state->notNecessarilyFinalMove)
  {
    slice_index const remover = make_remover(state->goal_to_be_reached);
    if (remover!=no_slice)
    {
      assert(SLICE_TYPE(SLICE_NEXT1(si))==STDoneGeneratingMoves);

      if (st->full_length<=2)
        slice_insertion_insert_contextually(si,st->context,&remover,1);
      else
        optimise_last_move_generation(si,st->context,remover);
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
                       SLICE_U(si).goal_handler.goal))
  {
    state->goal_to_be_reached = SLICE_U(si).goal_handler.goal;
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
  { STMoveGenerator,     &optimise_final_moves_move_generator },
  { STGoalReachedTester, &optimise_final_moves_goal           },
  { STNot,               &optimise_final_moves_suppress       }
};

enum
{
  nr_final_move_optimisers
  = (sizeof final_move_optimisers / sizeof final_move_optimisers[0])
};

/* Optimise move generation by inserting orthodox mating move generators
 * @param si identifies the root slice of the stipulation
 */
void solving_optimise_with_goal_non_reacher_removers(slice_index si)
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
  stip_moves_traversal_override_by_contextual(&st,
                                              slice_contextual_end_of_branch,
                                              &optimise_final_moves_end_of_branch_non_goal);
  stip_moves_traversal_override(&st,
                                final_move_optimisers,
                                nr_final_move_optimisers);
  stip_traverse_moves(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
