#include "solving/avoid_unsolvable.h"
#include "pypipe.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/boolean/binary.h"
#include "trace.h"

#include <assert.h>

/* **************** Initialisation ***************
 */

/* Allocate a STEndOfBranch slice.
 * @param to_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_avoid_unsolvable_slice(slice_index proxy_op1,
                                         slice_index proxy_op2)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy_op1);
  TraceFunctionParam("%u",proxy_op2);
  TraceFunctionParamListEnd();

  result = alloc_binary_slice(STAvoidUnsolvable,proxy_op1,proxy_op2);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


static void avoid_unusable_inserter_end_of_branch(slice_index si,
                                                  stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (st->context==stip_traversal_context_attack
      || st->context==stip_traversal_context_defense)
  {
    slice_index const proxy1 = alloc_proxy_slice();
    slice_index const proxy2 = alloc_proxy_slice();
    slice_index const fork = alloc_avoid_unsolvable_slice(proxy1,proxy2);
    pipe_link(slices[si].prev,fork);
    pipe_append(si,proxy2);
    pipe_link(proxy1,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors const avoid_unusable_inserters[] =
{
  { STEndOfBranch,       &avoid_unusable_inserter_end_of_branch },
  { STEndOfBranchGoal,   &avoid_unusable_inserter_end_of_branch },
  { STEndOfBranchForced, &avoid_unusable_inserter_end_of_branch }
};

enum
{
  nr_avoid_unusable_inserters = (sizeof avoid_unusable_inserters
                                 / sizeof avoid_unusable_inserters[0])
};

/* Instrument STEndOfBranch* slices with the necessary STAvoidUnusable slices
 * @param root_slice identifes root slice of stipulation
 */
void stip_insert_avoid_unsolvable_forks(slice_index root_slice)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root_slice);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    avoid_unusable_inserters,
                                    nr_avoid_unusable_inserters);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type avoid_unsolvable_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length);

  if (max_unsolvable<=slack_length)
    result = attack(slices[si].u.binary.op1,n);
  else
    result = attack(slices[si].u.binary.op2,n);

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
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type avoid_unsolvable_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length);

  if (max_unsolvable<=slack_length+1)
    result = defend(slices[si].u.binary.op1,n);
  else
    result = defend(slices[si].u.binary.op2,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
