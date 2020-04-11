#include "optimisations/count_nr_opponent_moves/move_generator.h"
#include "optimisations/count_nr_opponent_moves/prioriser.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "solving/machinery/slack_length.h"
#include "solving/fork_on_remaining.h"
#include "solving/temporary_hacks.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* for which Side(s) is the optimisation currently enabled? */
static boolean enabled[nr_sides] =  { false };

/* Reset the enabled state
 */
void reset_countnropponentmoves_defense_move_optimisation(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  enabled[White] = true;
  enabled[Black] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Disable the optimisation for defenses by a side
 * @param side side for which to disable the optimisation
 */
void disable_countnropponentmoves_defense_move_optimisation(Side side)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceFunctionParamListEnd();

  enabled[side] = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether the optimisation is enabled for a side
 * @param side
 * @return true iff the optimisation is enabled for side
 */
boolean is_countnropponentmoves_defense_move_optimisation_enabled(Side side)
{
  return enabled[side];
}

typedef struct
{
    slice_index found_optimiser;
    stip_length_type length;
} instrumentation_state_type;

static void remember_length(slice_index si,
                                            stip_structure_traversal *st)
{
  instrumentation_state_type * const state = st->param;
  stip_length_type const save_length = state->length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->length = SLICE_U(si).branch.length;
  stip_traverse_structure_children_pipe(si,st);
  state->length = save_length;


  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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

static void optimise_defense_move_generator(slice_index si,
                                            stip_structure_traversal *st)
{
  instrumentation_state_type const * const state = st->param;
  Side const defender = SLICE_STARTER(si);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(defender!=no_side);

  stip_traverse_structure_children_pipe(si,st);

  if (st->context==stip_traversal_context_defense
      && enabled[defender]
      && state->length>slack_length+2
      && st->activity==stip_traversal_activity_testing
      && state->found_optimiser==no_slice)
  {
    slice_index const proxy1 = alloc_proxy_slice();
    slice_index const proxy2 = alloc_proxy_slice();
    slice_index const fork = alloc_fork_on_remaining_slice(proxy1,proxy2,3);
    slice_index const hack = temporary_hack_opponent_moves_counter[defender];
    slice_index const operand2 = branch_find_slice(STDonePriorisingMoves,
                                                   SLICE_NEXT2(hack),
                                                   stip_traversal_context_intro);
    slice_index const proxy_operand2 = alloc_proxy_slice();
    slice_index const prototype = alloc_opponent_moves_few_moves_prioriser_slice(proxy_operand2);

    stip_structure_traversal st_nested;
    stip_deep_copies_type copies;

    assert(SLICE_TYPE(hack)==STOpponentMovesCounterFork);

    init_deep_copy(&st_nested,st,&copies);
    stip_structure_traversal_override_single(&st_nested,
                                             STDonePriorisingMoves,
                                             &stop_copying);
    stip_traverse_structure(si,&st_nested);

    pipe_link(SLICE_PREV(si),fork);
    pipe_link(proxy1,si);
    pipe_link(proxy2,copies[si]);

    link_to_branch(proxy_operand2,operand2);

    defense_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_optimiser(slice_index si, stip_structure_traversal *st)
{
  instrumentation_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",(void *)st);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  state->found_optimiser = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forget_optimiser(slice_index si, stip_structure_traversal *st)
{
  instrumentation_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",(void *)st);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  state->found_optimiser = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor const countnropponentmoves_optimisers[] =
{
  { STSetplayFork,       &stip_traverse_structure_children_pipe },
  { STRefutationsSolver, &stip_traverse_structure_children_pipe },
  { STReadyForDefense,   &remember_length                       },
  { STMoveGenerator,     &optimise_defense_move_generator       }
};

enum
{
  nr_countnropponentmoves_optimisers =
  (sizeof countnropponentmoves_optimisers
   / sizeof countnropponentmoves_optimisers[0])
};

/* Instrument stipulation with optimised move generation based on the number of
 * opponent moves
 * @param si identifies slice where to start
 */
void solving_optimise_with_countnropponentmoves(slice_index si)
{
  stip_structure_traversal st;
  instrumentation_state_type state = { no_slice, 0 };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&state);
  /* avoid optimising move generators that are already optimised ... */
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_move_reordering_optimiser,
                                                &remember_optimiser);
  /* ... but don't allow an optimiser to influence an unrelated generator */
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_move_generator,
                                                &forget_optimiser);
  stip_structure_traversal_override(&st,
                                    countnropponentmoves_optimisers,
                                    nr_countnropponentmoves_optimisers);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
