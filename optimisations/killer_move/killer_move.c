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

/* Remember the goal imminent after a defense or attack move
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static
void optimise_final_defense_moves_defense_move_generator(slice_index si,
                                                         stip_moves_traversal *st)
{
  Goal * const goal = st->param;
  Goal const save_goal = *goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_children(si,st);

  if (st->remaining==1
      && goal->type!=no_goal
      && enabled[slices[si].starter])
    killer_move_optimise_final_defense_move(si);

  *goal = save_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Remember the goal imminent after a defense or attack move
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void optimise_final_defense_moves_goal(slice_index si,
                                              stip_moves_traversal *st)
{
  Goal * const goal = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *goal = slices[si].u.goal_tester.goal;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static moves_traversers_visitors const final_defense_move_optimisers[] =
{
  { STSetplayFork,          &stip_traverse_moves_pipe                            },
  { STDefenseMoveGenerator, &optimise_final_defense_moves_defense_move_generator },
  { STGoalReachedTester,    &optimise_final_defense_moves_goal                   }
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
  stip_moves_traversal st;
  Goal goal = { no_goal, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_moves_traversal_init(&st,&goal);
  stip_moves_traversal_override(&st,
                                final_defense_move_optimisers,
                                nr_final_defense_move_optimisers);
  stip_traverse_moves(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void substitute_killermove_machinery(slice_index si,
                                            stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (enabled[slices[si].starter])
  {
    slice_index const prototype = alloc_killer_move_collector_slice();
    battle_branch_insert_slices(si,&prototype,1);
    pipe_substitute(si,alloc_killer_move_move_generator_slice());
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors killer_move_collector_inserters[] =
{
  { STSetplayFork,          &stip_traverse_structure_pipe    },
  { STAttackMoveGenerator,  &substitute_killermove_machinery },
  { STDefenseMoveGenerator, &substitute_killermove_machinery },
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

  optimise_final_defense_move_with_killer_moves(si);
  optimise_move_generators(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
