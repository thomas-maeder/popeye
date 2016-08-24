#include "optimisations/killer_move/killer_move.h"
#include "stipulation/proxy.h"
#include "stipulation/battle_play/branch.h"
#include "solving/fork_on_remaining.h"
#include "solving/move_generator.h"
#include "optimisations/killer_move/prioriser.h"
#include "optimisations/killer_move/collector.h"
#include "optimisations/killer_move/final_defense_move.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

move_generation_elmt killer_moves[maxply+1];

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
  TraceEnumerator(Side,side);
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

  *goal = SLICE_U(si).goal_handler.goal;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
    boolean are_we_at_root_level;
    boolean are_we_generating;
    stip_length_type lower_limit;
} final_defense_moves_iteration_state;

static
void optimise_final_defense_moves_move_generator(slice_index si,
                                                 stip_structure_traversal *st)
{
  final_defense_moves_iteration_state const * const state = st->param;
  Side const defender = SLICE_STARTER(si);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(defender!=no_side);

  stip_traverse_structure_children_pipe(si,st);

  if (st->activity==stip_traversal_activity_testing
      && !state->are_we_at_root_level
      && state->lower_limit==0
      && st->context==stip_traversal_context_defense
      && enabled[defender])
  {
    stip_structure_traversal st_nested;
    Goal goal = { no_goal, initsquare };

    stip_structure_traversal_init_nested(&st_nested,st,&goal);
    stip_structure_traversal_override_single(&st_nested,
                                             STGoalReachedTester,
                                             &remember_goal);
    stip_traverse_structure(si,&st_nested);

    if (goal.type!=no_goal)
      killer_move_optimise_final_defense_move(si,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_end_of_root(slice_index si, stip_structure_traversal *st)
{
  final_defense_moves_iteration_state * const state = st->param;
  boolean const save_root = state->are_we_at_root_level;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  state->are_we_at_root_level = false;
  stip_traverse_structure_children_pipe(si,st);
  state->are_we_at_root_level = save_root;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_generating(slice_index si, stip_structure_traversal *st)
{
  final_defense_moves_iteration_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  state->are_we_generating = true;
  state->lower_limit = 0;
  stip_traverse_structure_children_pipe(si,st);
  state->are_we_generating = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_not_generating(slice_index si, stip_structure_traversal *st)
{
  final_defense_moves_iteration_state * const state = st->param;
  stip_length_type const save_lower_limit = state->lower_limit;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  state->are_we_generating = false;
  stip_traverse_structure_children_pipe(si,st);
  state->are_we_generating = true;
  state->lower_limit = save_lower_limit;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_lower_move_limit(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(SLICE_NEXT2(si),st);

  {
    final_defense_moves_iteration_state * const state = st->param;
    stip_length_type const save_lower_limit = state->lower_limit;
    state->lower_limit = SLICE_U(si).fork_on_remaining.threshold;
    stip_traverse_structure_children_pipe(SLICE_NEXT1(si),st);
    state->lower_limit = save_lower_limit;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor const final_defense_move_optimisers[] =
{
  { STEndOfRoot,           &remember_end_of_root                        },
  { STGeneratingMoves,     &remember_generating                         },
  { STDonePriorisingMoves, &remember_not_generating                     },
  { STForkOnRemaining,     &remember_lower_move_limit                   },
  { STMoveGenerator,       &optimise_final_defense_moves_move_generator }
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
  final_defense_moves_iteration_state state = { true, false, 0 };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
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
  slice_index * const found_optimiser = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(SLICE_STARTER(si)!=no_side);

  *found_optimiser = no_slice;

  stip_traverse_structure_children(si,st);

  if (enabled[SLICE_STARTER(si)]
      && st->activity==stip_traversal_activity_testing
      && *found_optimiser==no_slice)
  {
    if (context==stip_traversal_context_attack)
    {
      slice_index const prototypes[] =
      {
          alloc_killer_move_prioriser_slice(),
          alloc_killer_attack_collector_slice()
      };
      enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
      attack_branch_insert_slices(si,prototypes,nr_prototypes);
    }
    else if (context==stip_traversal_context_defense)
    {
      slice_index const prototypes[] =
      {
          alloc_killer_move_prioriser_slice(),
          alloc_killer_defense_collector_slice()
      };
      enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
      defense_branch_insert_slices(si,prototypes,nr_prototypes);
    }
  }

  *found_optimiser = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_optimiser(slice_index si, stip_structure_traversal *st)
{
  slice_index * const found_optimiser = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  /* if the optimiser is of a fork type, we don't want to traverse next2! */
  stip_traverse_structure_children_pipe(si,st);

  *found_optimiser = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forget_optimiser(slice_index si, stip_structure_traversal *st)
{
  slice_index * const found_optimiser = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  *found_optimiser = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor killer_move_collector_inserters[] =
{
  { STRefutationsSolver,        &stip_traverse_structure_children_pipe },
  { STOpponentMovesCounterFork, &stip_traverse_structure_children_pipe },
  { STBackHomeFinderFork,       &stip_traverse_structure_children_pipe },
  { STNot,                      &stip_structure_visitor_noop           },
  { STMoveGenerator,            &substitute_killermove_machinery       }
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
  slice_index found_optimiser = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&found_optimiser);
  /* avoid optimising move generators that are already optimised ... */
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_move_reordering_optimiser,
                                                &remember_optimiser);
  /* ... but don't allow an optimiser to influence an unrelated generator */
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_move_generator,
                                                &forget_optimiser);
  stip_structure_traversal_override(&st,
                                    killer_move_collector_inserters,
                                    nr_killer_move_collector_inserters);
  stip_traverse_structure(si,&st);

  {
    ply i;
    for (i= maxply; i > 0; i--)
    {
      killer_moves[i].departure = initsquare;
      killer_moves[i].arrival = initsquare;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument stipulation with killer move slices
 * @param si identifies slice where to start
 */
void solving_optimise_with_killer_moves(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  solving_impose_starter(si,SLICE_STARTER(si));
  optimise_final_defense_move_with_killer_moves(si);
  optimise_move_generators(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
