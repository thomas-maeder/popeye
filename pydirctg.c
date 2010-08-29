#include "pydirctg.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/attack_move_to_goal.h"
#include "stipulation/battle_play/attack_fork.h"
#include "trace.h"

#include <assert.h>


static void direct_guards_inserter_attack(slice_index si,
                                          stip_structure_traversal *st)
{
  slice_index const * const to_goal = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    slice_index const fork = alloc_attack_fork_slice(length,min_length,
                                                     *to_goal);
    pipe_append(si,fork);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors direct_guards_inserters[] =
{
  { STReadyForAttack, &direct_guards_inserter_attack }
};

enum
{
  nr_direct_guards_inserters = (sizeof direct_guards_inserters
                                / sizeof direct_guards_inserters[0])
};

/* Instrument a branch with slices dealing with direct play
 * @param si root of branch to be instrumented
 * @param proxy_to_goal identifies slice leading towards goal
 */
void slice_insert_direct_guards(slice_index si, slice_index proxy_to_goal)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  assert(slices[proxy_to_goal].type==STProxy);

  stip_structure_traversal_init(&st,&proxy_to_goal);
  stip_structure_traversal_override(&st,
                                    direct_guards_inserters,
                                    nr_direct_guards_inserters);
  stip_traverse_structure(si,&st);

  TraceStipulation(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a the appropriate proxy slices before each STLeaf slice
 * @param si identifies STLeaf slice
 * @param st address of structure representing the traversal
 */
static void instrument_tester(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prev = slices[si].prev;
    slice_index const next = slices[si].u.pipe.next;
    Goal const goal = slices[si].u.goal_reached_tester.goal;

    slice_index const move = alloc_attack_move_to_goal_slice(goal);
    slice_index const played = alloc_branch(STAttackMovePlayed,
                                            slack_length_battle,
                                            slack_length_battle-1);
    slice_index const shoehorned = alloc_branch(STAttackMoveShoeHorningDone,
                                                slack_length_battle,
                                                slack_length_battle-1);
    slice_index const checked = alloc_branch(STAttackMoveLegalityChecked,
                                             slack_length_battle,
                                             slack_length_battle-1);
    slice_index const filtered = alloc_branch(STAttackMoveFiltered,
                                              slack_length_battle,
                                              slack_length_battle-1);
    slice_index const dealt = alloc_branch(STAttackDealtWith,
                                           slack_length_battle,
                                           slack_length_battle-1);

    pipe_link(prev,move);
    pipe_link(move,played);
    pipe_link(played,shoehorned);
    pipe_link(shoehorned,si);
    pipe_link(si,checked);
    pipe_link(checked,filtered);
    pipe_link(filtered,dealt);
    pipe_link(dealt,next);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors direct_leaf_instrumenters[] =
{
  { STGoalReachedTester, &instrument_tester }
};

enum
{
  nr_direct_leaf_instrumenters = (sizeof direct_leaf_instrumenters
                                  / sizeof direct_leaf_instrumenters[0])
};

/* Instrument a branch leading to a goal to be a direct goal branch
 * @param si identifies entry slice of branch
 */
void slice_make_direct_goal_branch(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    direct_leaf_instrumenters,
                                    nr_direct_leaf_instrumenters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
