#include "optimisations/orthodox_mating_moves/orthodox_mating_moves_generation.h"
#include "conditions/conditions.h"
#include "stipulation/pipe.h"
#include "stipulation/proxy.h"
#include "solving/moves_traversal.h"
#include "solving/fork_on_remaining.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_move_generator.h"
#include "optimisations/orthodox_mating_moves/king_contact_move_generator.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* for which Side(s) is the optimisation currently enabled? */
static boolean enabled[nr_sides] = { false };

/* Reset the enabled state of the optimisation of final defense moves
 */
void reset_orthodox_mating_move_optimisation(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  enabled[White] = true;
  enabled[Black] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Disable the optimisation of final defense moves for defense by a side
 * @param side side for which to disable the optimisation
 */
void disable_orthodox_mating_move_optimisation(Side side)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceFunctionParamListEnd();

  if (side==nr_sides)
  {
    enabled[White] = false;
    enabled[Black] = false;
  }
  else
    enabled[side] = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Is a goal eligible for this optimisation?
 * @param goal type of goal
 * @return true iff the goal is eligible
 */
static boolean is_goal_eligible(goal_type goal)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  switch (goal)
  {
    case goal_mate:
    case goal_check:
    case goal_doublemate:
      result = true;
      break;

    default:
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

typedef struct
{
    Goal goal_to_be_reached;
    unsigned int nr_goals_to_be_reached;
    boolean notNecessarilyFinalMove;
} final_move_optimisation_state;

static final_move_optimisation_state const init_state = { { no_goal, initsquare }, 0, false };

/* Remember the goal imminent after a defense or solve move
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void optimise_final_moves_move_generator(slice_index si,
                                                stip_moves_traversal *st)
{
  final_move_optimisation_state * const state = st->param;
  final_move_optimisation_state const save_state = *state;
  Side const starter = SLICE_STARTER(si);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *state = init_state;

  stip_traverse_moves_children(si,st);

  if (st->context!=stip_traversal_context_defense
      && st->remaining==1
      && state->nr_goals_to_be_reached==1
      && is_goal_eligible(state->goal_to_be_reached.type)
      && !state->notNecessarilyFinalMove
      && enabled[starter])
  {
    slice_index const generator = alloc_orthodox_mating_move_generator_slice();
    if (st->full_length<=2)
    {
      pipe_substitute(si,generator);
      if (CondFlag[SLICE_STARTER(si)==Black ? whiteedge : blackedge]
          || state->goal_to_be_reached.type==goal_doublemate)
        pipe_append(si,alloc_orthodox_mating_king_contact_generator_generator_slice());
    }
    else
    {
      slice_index const proxy1 = alloc_proxy_slice();
      slice_index const proxy2 = alloc_proxy_slice();
      slice_index const fork = alloc_fork_on_remaining_slice(proxy1,proxy2,1);
      pipe_link(SLICE_PREV(si),fork);
      pipe_link(proxy1,si);
      pipe_link(proxy2,generator);
      pipe_link(generator,SLICE_NEXT1(si));
      if (CondFlag[SLICE_STARTER(si)==Black ? whiteedge : blackedge]
          || state->goal_to_be_reached.type==goal_doublemate)
        pipe_append(generator,alloc_orthodox_mating_king_contact_generator_generator_slice());
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
void solving_optimise_with_orthodox_mating_move_generators(slice_index si)
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
