#include "solving/find_shortest.h"
#include "pydata.h"
#include "stipulation/proxy.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "solving/avoid_unsolvable.h"
#include "solving/find_by_increasing_length.h"
#include "solving/fork_on_remaining.h"
#include "debugging/trace.h"

#include <assert.h>

/* Allocate a STFindShortest slice.
 * @param length maximum number of half moves until end of slice
 * @param min_length minimum number of half moves until end of slice
 * @return index of allocated slice
 */
slice_index alloc_find_shortest_slice(stip_length_type length,
                                      stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STFindShortest,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type find_shortest_solve(slice_index si, stip_length_type n)
{
  stip_length_type result = n+2;
  slice_index const next = slices[si].next1;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  stip_length_type const n_min = (min_length>=(length-n)+slack_length
                                  ? min_length-(length-n)
                                  : min_length);
  stip_length_type n_current;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(length>=n);

  for (n_current = n_min+(n-n_min)%2; n_current<=n; n_current += 2)
  {
    result = solve(next,n_current);
    if (result<=n_current)
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_find_shortest_battle_adapter(slice_index si,
                                                stip_structure_traversal *st)
{
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (length>=min_length+2)
  {
    slice_index const defense = branch_find_slice(STReadyForDefense,si,st->context);
    slice_index const attack = branch_find_slice(STReadyForAttack,
                                                 defense,
                                                 stip_traversal_context_defense);
    slice_index const prototypes[] =
    {
        alloc_find_shortest_slice(length,min_length),
        alloc_learn_unsolvable_slice()
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    assert(defense!=no_slice);
    assert(attack!=no_slice);
    attack_branch_insert_slices(attack,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_index find_ready_for_move_in_loop(slice_index ready_root)
{
  slice_index result = ready_root;
  do
  {
    result = branch_find_slice(STReadyForHelpMove,
                               result,
                               stip_traversal_context_help);
  } while ((slices[result].u.branch.length-slack_length)%2
           !=(slices[ready_root].u.branch.length-slack_length)%2);
  return result;
}

static void insert_find_shortest_help_adapter(slice_index si,
                                              stip_structure_traversal *st)
{
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (length+2>=min_length)
  {
    if (st->level==structure_traversal_level_nested)
    {
      if (st->context==stip_traversal_context_intro)
      {
        slice_index const prototype = alloc_find_shortest_slice(length,min_length);
        branch_insert_slices(si,&prototype,1);
      }
    }
    else /* root or set play */
    {
      if (!OptFlag[restart] && length>=slack_length+2)
      {
        {
          slice_index const prototype =
              alloc_find_by_increasing_length_slice(length,min_length);
          branch_insert_slices(si,&prototype,1);
        }
        {
          slice_index const ready_root = branch_find_slice(STReadyForHelpMove,
                                                           si,
                                                           st->context);
          slice_index const ready_loop = find_ready_for_move_in_loop(ready_root);
          slice_index const proxy_root = alloc_proxy_slice();
          slice_index const proxy_loop = alloc_proxy_slice();
          pipe_set_successor(proxy_loop,ready_loop);
          pipe_link(slices[ready_root].prev,
                    alloc_fork_on_remaining_slice(proxy_root,proxy_loop,
                                                  length-1-slack_length));
          pipe_link(proxy_root,ready_root);
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor const find_shortest_inserters[] =
{
  { STAttackAdapter,  &insert_find_shortest_battle_adapter },
  { STDefenseAdapter, &insert_find_shortest_battle_adapter },
  { STHelpAdapter,    &insert_find_shortest_help_adapter   }
};

enum
{
  nr_find_shortest_inserters = sizeof find_shortest_inserters / sizeof find_shortest_inserters[0]
};

/* Instrument the stipulation with slices that attempt the shortest
 * solutions/variations
 * @param si root slice of stipulation
 */
void stip_insert_find_shortest_solvers(slice_index si)
{
  stip_structure_traversal st;
  output_mode mode = output_mode_none;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&mode);
  stip_structure_traversal_override(&st,find_shortest_inserters,nr_find_shortest_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
