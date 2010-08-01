#include "pydirctg.h"
#include "pybrafrk.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/defense_play.h"
#include "trace.h"

#include <assert.h>


/* **************** Initialisation ***************
 */

/* Allocate a STDirectDefenderFilter slice
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_direct_defender_filter_slice(stip_length_type length,
                                               stip_length_type min_length,
                                               slice_index proxy_to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  result = alloc_branch_fork(STDirectDefenderFilter,
                             length,min_length,
                             proxy_to_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* **************** Implementation of interface Slice **********
 */

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - more refutations found than acceptable
 */
stip_length_type
direct_defender_filter_defend_in_n(slice_index si,
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

  assert(n>=slack_length_battle);

  if (n_max_unsolvable<slack_length_battle)
  {
    slice_index const length = slices[si].u.branch_fork.length;
    slice_index const min_length = slices[si].u.branch_fork.min_length;
    slice_index const to_goal = slices[si].u.branch_fork.towards_goal;

    if (n-slack_length_battle<=length-min_length)
    {
      if (defense_defend(to_goal))
      {
        n_max_unsolvable = slack_length_battle;
        result = defense_defend_in_n(next,n,n_max_unsolvable);
      }
      else
        result = n;
    }
    else
    {
      if (defense_can_defend(to_goal))
      {
        n_max_unsolvable = slack_length_battle;
        result = defense_defend_in_n(next,n,n_max_unsolvable);
      }
      else
        /* we have reached the goal earlier than allowed */
        result = n+4;
    }
  }
  else
    result = defense_defend_in_n(next,n,n_max_unsolvable);

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
 * @param max_nr_refutations how many refutations should we look for
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=max_nr_refutations refutations found
 *         n+4 refuted - >max_nr_refutations refutations found
 */
stip_length_type
direct_defender_filter_can_defend_in_n(slice_index si,
                                       stip_length_type n,
                                       stip_length_type n_max_unsolvable,
                                       unsigned int max_nr_refutations)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParam("%u",max_nr_refutations);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_battle);

  if (n_max_unsolvable<slack_length_battle)
  {
    if (defense_can_defend(slices[si].u.branch_fork.towards_goal))
    {
      n_max_unsolvable = slack_length_battle;
      result = defense_can_defend_in_n(next,
                                       n,n_max_unsolvable,
                                       max_nr_refutations);
    }
    else
    {
      slice_index const length = slices[si].u.branch_fork.length;
      slice_index const min_length = slices[si].u.branch_fork.min_length;

      if (n-slack_length_battle<=length-min_length)
        result = n;
      else
        /* we have reached the goal earlier than allowed */
        result = n+4;
    }
  }
  else
    result = defense_can_defend_in_n(next,
                                     n,n_max_unsolvable,
                                     max_nr_refutations);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void direct_defender_filter_make_root(slice_index si,
                                      stip_structure_traversal *st)
{
  slice_index * const root = st->param;
  slice_index root_filter;
  stip_length_type const length = slices[si].u.branch_fork.length;
  stip_length_type const min_length = slices[si].u.branch_fork.min_length;
  slice_index const to_goal = slices[si].u.branch_fork.towards_goal;
  slice_index root_to_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  root_to_goal = stip_make_root_slices(to_goal);
  root_filter = alloc_direct_defender_filter_slice(length,min_length,
                                                   root_to_goal);

  stip_traverse_structure_pipe(si,st);

  pipe_link(root_filter,*root);
  *root = root_filter;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void
direct_defender_filter_reduce_to_postkey_play(slice_index si,
                                              stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);
  dealloc_slices(slices[si].u.branch_fork.towards_goal);
  dealloc_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traversal of the moves beyond a series fork slice 
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_direct_defender_filter(slice_index si,
                                                stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_branch_init_full_length(si,st);

  if (st->remaining<=slack_length_battle+1)
    stip_traverse_moves_branch(slices[si].u.branch_fork.towards_goal,st);

  stip_traverse_moves_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* **************** Stipulation instrumentation ***************
 */

/* Insert a STDirectDefenderFilter slice after each STAttackMove slice
 * @param si identifies slice before which to insert a *
 *           STDirectDefenderFilter slice
 * @param st address of structure representing the traversal
 */
static void direct_guards_inserter_attack(slice_index si,
                                          stip_structure_traversal *st)
{
  slice_index * const to_goal = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    stip_length_type const length = slices[si].u.branch.length;
    stip_length_type const min_length = slices[si].u.branch.min_length;
    slice_index const
        filter = alloc_direct_defender_filter_slice(length-1,min_length-1,
                                                    *to_goal); 
    pipe_append(si,filter);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors direct_guards_inserters[] =
{
  { STAttackMove, &direct_guards_inserter_attack }
};

enum
{
  nr_direct_guards_inserters = (sizeof direct_guards_inserters
                                / sizeof direct_guards_inserters[0])
};

/* Instrument a branch with STDirectDefenderFilter slices
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
