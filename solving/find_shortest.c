#include "solving/find_shortest.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STFindShortest slice.
 * @return index of allocated slice
 */
slice_index alloc_find_shortest_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STFindShortest);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type find_shortest_attack(slice_index si, stip_length_type n)
{
  stip_length_type result = n+2;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type const n_min = (max_unsolvable<slack_length
                                  ? slack_length+1
                                  : max_unsolvable+1);
  stip_length_type n_current;
  stip_length_type const save_max_unsolvable = max_unsolvable;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (n_current = n_min+(n-n_min)%2; n_current<=n; n_current += 2)
  {
    result = attack(next,n_current);
    if (result<=n_current)
      break;
    else
      max_unsolvable = n_current;
  }

  max_unsolvable = save_max_unsolvable;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index
 * @param n exact number of moves to reach the end state
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type find_shortest_help(slice_index si, stip_length_type n)
{
  stip_length_type result = slack_length+1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result += (n-result)%2;

  while (result<=n)
    if (help(slices[si].u.pipe.next,result)==result)
      break;
    else
      result += 2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void battle_insert_find_shortest(slice_index si)
{
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (length>=min_length+2)
  {
    slice_index const defense = branch_find_slice(STReadyForDefense,si);
    slice_index const attack = branch_find_slice(STReadyForAttack,defense);
    slice_index const proto = alloc_find_shortest_slice();
    assert(defense!=no_slice);
    assert(attack!=no_slice);
    battle_branch_insert_slices(attack,&proto,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_find_shortest_attack_adapter(slice_index si,
                                                stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  battle_insert_find_shortest(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_find_shortest_defense_adapter(slice_index si,
                                                 stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  battle_insert_find_shortest(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors const find_shortest_inserters[] =
{
  { STAttackAdapter,  &insert_find_shortest_attack_adapter  },
  { STDefenseAdapter, &insert_find_shortest_defense_adapter }
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
