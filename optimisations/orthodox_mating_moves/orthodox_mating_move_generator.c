#include "optimisations/orthodox_mating_moves/orthodox_mating_move_generator.h"
#include "pystip.h"
#include "pydata.h"
#include "pyproc.h"
#include "pypipe.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_moves_generation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/proxy.h"
#include "stipulation/conditional_pipe.h"
#include "stipulation/branch.h"
#include "stipulation/goals/goals.h"
#include "stipulation/moves_traversal.h"
#include "solving/fork_on_remaining.h"
#include "debugging/trace.h"

#include <assert.h>

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
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  enabled[side] = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a STOrthodoxMatingMoveGenerator slice.
 * @param goal goal to be reached
 * @return index of allocated slice
 */
static
slice_index
alloc_orthodox_mating_move_generator_slice(Goal goal,
                                           move_generation_mode_type mode)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal.type);
  TraceFunctionParam("%u",mode);
  TraceFunctionParamListEnd();

  assert(goal.type!=no_goal);

  result = alloc_pipe(STOrthodoxMatingMoveGenerator);
  slices[result].u.move_generator.goal = goal;
  slices[result].u.move_generator.mode = mode;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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

    case goal_target:
    case goal_chess81:
      result = true;
      break;

    case goal_capture:
    case goal_steingewinn:
      result = true;
      break;

    case goal_ep:
      /* TODO only generate pawn moves? */
      result = true;
      break;

    case goal_castling:
      result = true;
      /* TODO only generate king moves? */
      break;

    case goal_countermate:
      /* TODO only generate king and ortho moves if there are no
       * obstacles?
       */
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

/* Remember the goal imminent after a defense or attack move
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void optimise_final_moves_move_generator(slice_index si,
                                                stip_moves_traversal *st)
{
  final_move_optimisation_state * const state = st->param;
  final_move_optimisation_state const save_state = *state;
  Side const starter = slices[si].starter;

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
    move_generation_mode_type mode = st->context==stip_traversal_context_attack ? move_generation_optimized_by_killer_move : move_generation_not_optimized;
    slice_index const generator
      = alloc_orthodox_mating_move_generator_slice(state->goal_to_be_reached,mode);
    if (st->full_length<=2)
      pipe_substitute(si,generator);
    else
    {
      slice_index const proxy1 = alloc_proxy_slice();
      slice_index const proxy2 = alloc_proxy_slice();
      slice_index const fork = alloc_fork_on_remaining_slice(proxy1,proxy2,1);
      slice_index const proxy3 = alloc_proxy_slice();
      pipe_link(slices[si].prev,fork);
      pipe_link(proxy1,si);
      pipe_append(si,proxy3);
      pipe_link(proxy2,generator);
      pipe_set_successor(generator,proxy3);
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
  { STMoveGenerator,     &optimise_final_moves_move_generator         },
  { STEndOfBranch,       &optimise_final_moves_end_of_branch_non_goal },
  { STEndOfBranchForced, &optimise_final_moves_end_of_branch_non_goal },
  { STGoalReachedTester, &optimise_final_moves_goal                   },
  { STNot,               &optimise_final_moves_suppress               }
};

enum
{
  nr_final_move_optimisers
  = (sizeof final_move_optimisers / sizeof final_move_optimisers[0])
};

/* Optimise move generation by inserting orthodox mating move generators
 * @param si identifies the root slice of the stipulation
 */
void stip_optimise_with_orthodox_mating_move_generators(slice_index si)
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
  stip_moves_traversal_override(&st,
                                final_move_optimisers,
                                nr_final_move_optimisers);
  stip_traverse_moves(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
orthodox_mating_move_generator_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n==slack_length+1);

  move_generation_mode = slices[si].u.move_generator.mode;
  TraceValue("->%u\n",move_generation_mode);
  empile_for_goal = slices[si].u.move_generator.goal;
  generate_move_reaching_goal(slices[si].starter);
  empile_for_goal.type = no_goal;
  result = attack(slices[si].next1,n);
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
