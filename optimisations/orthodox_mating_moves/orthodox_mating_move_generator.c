#include "optimisations/orthodox_mating_moves/orthodox_mating_move_generator.h"
#include "pydata.h"
#include "pyproc.h"
#include "pypipe.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_moves_generation.h"
#include "optimisations/optimisation_fork.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STOrthodoxMatingMoveGenerator slice.
 * @param goal goal to be reached
 * @return index of allocated slice
 */
static slice_index alloc_orthodox_mating_move_generator_slice(Goal goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal.type);
  TraceFunctionParamListEnd();

  result = alloc_branch(STOrthodoxMatingMoveGenerator,
                        slack_length_battle+1,slack_length_battle);
  slices[result].u.branch.imminent_goal = goal;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Optimise a STAttackMoveGenerator slice for attacking a mate goal
 * @param si identifies slice to be optimised
 * @param goal goal that slice si defends against
 */
static void attack_move_generator_optimise_orthodox_mating(slice_index si,
                                                           Goal goal)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal.type);
  TraceFunctionParamListEnd();

  {
    slice_index const to_goal = alloc_orthodox_mating_move_generator_slice(goal);
    slice_index const proxy = alloc_proxy_slice();
    slice_index const fork = alloc_optimisation_fork_slice(proxy,1);
    slice_index const proxy2 = alloc_proxy_slice();
    pipe_append(slices[si].prev,fork);
    pipe_append(si,proxy2);
    pipe_link(proxy,to_goal);
    pipe_set_successor(to_goal,proxy2);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
    Goal goal;
    boolean notNecessarilyFinalMove;
} final_move_optimisation_state;

/* Remember the goal imminent after a defense or attack move
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void optimise_final_moves_attack_move_generator(slice_index si,
                                                       stip_moves_traversal *st)
{
  final_move_optimisation_state * const state = st->param;
  Goal const save_goal = state->goal;
  boolean const save_notNecessarilyFinalMove = state->notNecessarilyFinalMove;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_children(si,st);

  if (st->remaining==1
      && state->goal.type!=no_goal
      && !state->notNecessarilyFinalMove)
    attack_move_generator_optimise_orthodox_mating(si,state->goal);

  state->goal = save_goal;
  state->notNecessarilyFinalMove = save_notNecessarilyFinalMove;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether there are more moves after this branch
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void optimise_final_moves_end_of_battle_branch(slice_index si,
                                                      stip_moves_traversal *st)
{
  final_move_optimisation_state * const state = st->param;
  Goal const save_goal = state->goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->goal.type = no_goal;

  stip_traverse_moves_children(si,st);

  if (state->goal.type==no_goal)
    state->notNecessarilyFinalMove = true;

  state->goal = save_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Forget a remembered goal because it is to be reached by a move
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void move_swallow_goal(slice_index si, stip_moves_traversal *st)
{
  final_move_optimisation_state * const state = st->param;
  Goal const save_goal = state->goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_move_slice(si,st);
  state->goal = save_goal;

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
  Goal const save_goal = state->goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_children(si,st);
  state->goal = save_goal;

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

  state->goal = slices[si].u.goal_writer.goal;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static moves_traversers_visitors const final_move_optimisers[] =
{
  { STAttackMoveGenerator,         &optimise_final_moves_attack_move_generator },
  { STOrthodoxMatingMoveGenerator, &generator_swallow_goal                     },
  { STDefenseMoveGenerator,        &generator_swallow_goal                     },
  { STReflexDefenderFilter,        &optimise_final_moves_end_of_battle_branch  },
  { STHelpMoveToGoal,              &move_swallow_goal                          },
  { STSeriesMoveToGoal,            &move_swallow_goal                          },
  { STGoalReachedTesting,          &optimise_final_moves_goal                  }
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
  final_move_optimisation_state state = { { no_goal, initsquare }, false };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_moves_traversal_init(&st,&state);
  stip_moves_traversal_override(&st,
                                final_move_optimisers,nr_final_move_optimisers);
  stip_traverse_moves(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
orthodox_mating_move_generator_can_attack(slice_index si,
                                          stip_length_type n,
                                          stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(slices[si].u.branch.imminent_goal.type!=no_goal);
  assert(n==slack_length_battle+1);

  move_generation_mode = move_generation_optimized_by_killer_move;
  TraceValue("->%u\n",move_generation_mode);
  empile_for_goal = slices[si].u.branch.imminent_goal;
  generate_move_reaching_goal(slices[si].starter);
  empile_for_goal.type = no_goal;
  result = can_attack(next,n,n_max_unsolvable);
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @note n==n_max_unsolvable means that we are solving refutations
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
orthodox_mating_move_generator_attack(slice_index si,
                                      stip_length_type n,
                                      stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  assert(n==slack_length_battle+1);
  assert(slices[si].u.branch.imminent_goal.type!=no_goal);

  move_generation_mode = move_generation_not_optimized;
  TraceValue("->%u\n",move_generation_mode);
  empile_for_goal = slices[si].u.branch.imminent_goal;
  generate_move_reaching_goal(slices[si].starter);
  empile_for_goal.type = no_goal;
  result = attack(next,n,n_max_unsolvable);
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
