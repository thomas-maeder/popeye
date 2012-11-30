#include "optimisations/count_nr_opponent_moves/move_generator.h"
#include "stipulation/stipulation.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/proxy.h"
#include "solving/fork_on_remaining.h"
#include "optimisations/count_nr_opponent_moves/prioriser.h"
#include "debugging/trace.h"

#include <assert.h>

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
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  enabled[side] = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_length(slice_index si,
                                            stip_structure_traversal *st)
{
  stip_length_type * const length = st->param;
  stip_length_type const save_length = *length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *length = slices[si].u.branch.length;
  stip_traverse_structure_children_pipe(si,st);
  *length = save_length;


  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void optimise_defense_move_generator(slice_index si,
                                            stip_structure_traversal *st)
{
  stip_length_type const * const length = st->param;
  Side const defender = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(defender!=no_side);

  stip_traverse_structure_children_pipe(si,st);

  if (st->context==stip_traversal_context_defense
      && enabled[defender]
      && *length>slack_length+2
      && st->activity==stip_traversal_activity_testing)
  {
    slice_index const proxy1 = alloc_proxy_slice();
    slice_index const proxy2 = alloc_proxy_slice();
    slice_index const copy = copy_slice(si);
    slice_index const fork = alloc_fork_on_remaining_slice(proxy1,proxy2,3);

    assert(slices[slices[si].next1].type==STDoneGeneratingMoves);

    pipe_link(slices[si].prev,fork);

    pipe_link(proxy1,si);

    pipe_link(proxy2,copy);
    pipe_set_successor(copy,slices[si].next1);

    slices[si].u.move_generator.mode = move_generation_optimised_by_nr_opponent_moves;
    pipe_append(si,alloc_opponent_moves_few_moves_prioriser_slice());
  }

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
void stip_optimise_with_countnropponentmoves(slice_index si)
{
  stip_structure_traversal st;
  stip_length_type length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&length);
  stip_structure_traversal_override(&st,
                                    countnropponentmoves_optimisers,
                                    nr_countnropponentmoves_optimisers);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
