#include "solving/for_each_move.h"
#include "pydata.h"
#include "pyproc.h"
#include "pypipe.h"
#include "pybrafrk.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "solving/solving.h"
#include "solving/find_move.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STForEachMove slice.
 * @return index of allocated slice
 */
slice_index alloc_for_each_move_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STForEachMove);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_move_iterator_move(slice_index si,
                                      stip_structure_traversal *st)
{
  boolean const * const testing = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = (*testing
                                   ? alloc_find_move_slice()
                                   : alloc_for_each_move_slice());
    battle_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_move_iterator_move_conditional_pipe(slice_index si,
                                                       stip_structure_traversal *st)
{
  boolean * const testing = st->param;
  boolean const save_testing = *testing;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  *testing = true;
  stip_traverse_structure_next_branch(si,st);
  *testing = save_testing;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors const move_iterator_inserters[] =
{
  { STMove, &insert_move_iterator_move },
};

enum
{
  nr_move_iterator_inserters = (sizeof move_iterator_inserters
                                / sizeof move_iterator_inserters[0])
};

/* Instrument the stipulation with move iterator slices
 * @param root_slice identifies root slice of stipulation
 */
void stip_insert_move_iterators(slice_index root_slice)
{
  boolean testing = false;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root_slice);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&testing);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_conditional_pipe,
                                                insert_move_iterator_move_conditional_pipe);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_testing_pipe,
                                                insert_move_iterator_move_conditional_pipe);
  stip_structure_traversal_override(&st,
                                    move_iterator_inserters,
                                    nr_move_iterator_inserters);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type for_each_move_attack(slice_index si, stip_length_type n)
{
  stip_length_type result = n+2;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  while (encore())
  {
    if (jouecoup(nbply,first_play))
    {
      stip_length_type const length_sol = attack(next,n);
      if (length_sol<result)
        result = length_sol;
    }

    repcoup();
  }

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
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type for_each_move_defend(slice_index si, stip_length_type n)
{
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type result = slack_length_battle-1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  while(encore())
  {
    if (jouecoup(nbply,first_play))
    {
      stip_length_type const length_sol = defend(next,n);
      if (result<length_sol)
        result = length_sol;
    }

    repcoup();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write the solution(s) in a help stipulation
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type for_each_move_help(slice_index si, stip_length_type n)
{
  stip_length_type result = n+2;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  while (encore())
  {
    if (jouecoup(nbply,first_play) && help(next,n)==n)
      result = n;

    repcoup();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
