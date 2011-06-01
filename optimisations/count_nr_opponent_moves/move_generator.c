#include "optimisations/count_nr_opponent_moves/move_generator.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/proxy.h"
#include "solving/fork_on_remaining.h"
#include "trace.h"

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

/* Allocate a STCountNrOpponentMovesMoveGenerator defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
static slice_index alloc_countnropponentmoves_move_generator_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STCountNrOpponentMovesMoveGenerator);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @note n==n_max_unsolvable means that we are solving refutations
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type
countnropponentmoves_move_generator_defend(slice_index si,
                                           stip_length_type n,
                                           stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  Side const defender = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  move_generation_mode = move_generation_optimized_by_nr_opponent_moves;
  genmove(defender);
  result = defend(next,n,n_max_unsolvable);
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type
countnropponentmoves_move_generator_can_defend(slice_index si,
                                               stip_length_type n,
                                               stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  Side const defender = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  move_generation_mode = move_generation_optimized_by_nr_opponent_moves;
  genmove(defender);
  result = can_defend(next,n,n_max_unsolvable);
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
  stip_traverse_structure_children(si,st);
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

  stip_traverse_structure_children(si,st);

  if (st->context==structure_traversal_context_defense
      && enabled[defender] && *length>slack_length_battle+2)
  {
    slice_index const proxy1 = alloc_proxy_slice();
    slice_index const copy = copy_slice(si);
    slice_index const fork = alloc_fork_on_remaining_slice(proxy1,3);
    slice_index const proxy2 = alloc_proxy_slice();

    pipe_append(slices[si].prev,fork);
    pipe_substitute(si,alloc_countnropponentmoves_move_generator_slice());
    pipe_append(si,proxy2);

    pipe_link(proxy1,copy);
    pipe_set_successor(copy,proxy2);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors const countnropponentmoves_optimisers[] =
{
  { STSetplayFork,          &stip_traverse_structure_pipe    },
  { STReadyForDefense,      &remember_length                 },
  { STAttackMoveGenerator,  &optimise_defense_move_generator },
  { STDefenseMoveGenerator, &optimise_defense_move_generator },
  { STHelpMoveGenerator,    &optimise_defense_move_generator },
  { STSeriesMoveGenerator,  &optimise_defense_move_generator }
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
  stip_length_type length = slack_length_battle;

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
