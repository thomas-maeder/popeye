#include "optimisations/killer_move/killer_move.h"
#include "pypipe.h"
#include "stipulation/proxy.h"
#include "stipulation/battle_play/branch.h"
#include "solving/fork_on_remaining.h"
#include "optimisations/killer_move/move_generator.h"
#include "optimisations/killer_move/collector.h"
#include "optimisations/killer_move/final_defense_move.h"
#include "trace.h"

#include <assert.h>


/* for which Side(s) is the optimisation currently enabled? */
static boolean enabled[nr_sides] = { false };

/* Reset the enabled state of the optimisation of final defense moves
 */
void reset_killer_move_optimisation(void)
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
void disable_killer_move_optimisation(Side side)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  enabled[side] = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_goal(slice_index si, stip_structure_traversal *st)
{
  Goal * const goal = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *goal = slices[si].u.goal_handler.goal;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
    boolean testing;
    boolean root;
} final_defense_moves_iteration_state;

static
void optimise_final_defense_moves_move_generator(slice_index si,
                                                 stip_structure_traversal *st)
{
  final_defense_moves_iteration_state const * const state = st->param;
  Side const defender = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(defender!=no_side);

  stip_traverse_structure_children(si,st);

  if (state->testing && !state->root
      && st->context==stip_traversal_context_defense
      && enabled[defender])
  {
    stip_structure_traversal st_nested;
    Goal goal = { no_goal, initsquare };

    stip_structure_traversal_init(&st_nested,&goal);
    st_nested.context = st->context;
    stip_structure_traversal_override_single(&st_nested,STGoalReachedTester,&remember_goal);
    stip_traverse_structure(si,&st_nested);

    if (goal.type!=no_goal)
      killer_move_optimise_final_defense_move(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_testing(slice_index si, stip_structure_traversal *st)
{
  final_defense_moves_iteration_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  if (state->testing)
    stip_traverse_structure_children(si,st);
  else
  {
    stip_traverse_structure_pipe(si,st);

    if (slices[si].u.fork.fork!=no_slice)
    {
      state->testing = true;
      stip_traverse_structure(slices[si].u.fork.fork,st);
      state->testing = false;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_end_of_root(slice_index si, stip_structure_traversal *st)
{
  final_defense_moves_iteration_state * const state = st->param;
  boolean const save_root = state->root;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  state->root = false;
  stip_traverse_structure_children(si,st);
  state->root = save_root;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors const final_defense_move_optimisers[] =
{
  { STSetplayFork,   &stip_traverse_structure_pipe                },
  { STEndOfRoot,     &remember_end_of_root                        },
  { STMoveGenerator, &optimise_final_defense_moves_move_generator }
};

enum
{
  nr_final_defense_move_optimisers = (sizeof final_defense_move_optimisers
                                      / sizeof final_defense_move_optimisers[0])
};

/* Instrument stipulation with killer move slices
 * @param si identifies slice where to start
 */
static void optimise_final_defense_move_with_killer_moves(slice_index si)
{
  stip_structure_traversal st;
  final_defense_moves_iteration_state state = { false, true };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_testing_pipe,
                                                &remember_testing);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_conditional_pipe,
                                                &remember_testing);
  stip_structure_traversal_override(&st,
                                    final_defense_move_optimisers,
                                    nr_final_defense_move_optimisers);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void substitute_killermove_machinery(slice_index si,
                                            stip_structure_traversal *st)
{
  stip_traversal_context_type context = st->context;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(slices[si].starter!=no_side);

  stip_traverse_structure_children(si,st);

  if ((context==stip_traversal_context_attack
      || context==stip_traversal_context_defense)
      && enabled[slices[si].starter])
  {
    slice_index const prototype = alloc_killer_move_collector_slice();
    if (st->context==stip_traversal_context_attack)
      attack_branch_insert_slices(si,&prototype,1);
    else
      defense_branch_insert_slices(si,&prototype,1);
    pipe_substitute(si,alloc_killer_move_move_generator_slice());
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors killer_move_collector_inserters[] =
{
  { STSetplayFork,   &stip_traverse_structure_pipe    },
  { STMoveGenerator, &substitute_killermove_machinery }
};

enum
{
  nr_killer_move_collector_inserters =
  (sizeof killer_move_collector_inserters
   / sizeof killer_move_collector_inserters[0])
};

static void optimise_move_generators(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    killer_move_collector_inserters,
                                    nr_killer_move_collector_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument stipulation with killer move slices
 * @param si identifies slice where to start
 */
void stip_optimise_with_killer_moves(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_impose_starter(si,slices[si].starter);
  optimise_final_defense_move_with_killer_moves(si);
  optimise_move_generators(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
