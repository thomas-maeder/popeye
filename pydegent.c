#include "pydegent.h"
#include "pydata.h"
#include "pypipe.h"
#include "pypipe.h"
#include "pyoutput.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/attack_play.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static stip_length_type max_length_short_solutions;

/* Reset the max threats setting to off
 */
void reset_max_nr_nontrivial_length(void)
{
  max_length_short_solutions = no_stip_length;
}

/* Read the requested max threat length setting from a text token
 * entered by the user
 * @param textToken text token from which to read
 * @return true iff max threat setting was successfully read
 */
void init_degenerate_tree(stip_length_type max_length_short)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",max_length_short);
  TraceFunctionParamListEnd();

  max_length_short_solutions = max_length_short+1;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* **************** Initialisation ***************
 */

/* Allocate a STDegenerateTree slice
 * @return allocated slice
 */
static slice_index alloc_degenerate_tree_guard_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STDegenerateTree);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Delegate finding a solution to the next slice, gradually increasing
 * the number of allowed half-moves
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimum number of half-moves to try
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
static stip_length_type
delegate_has_solution_in_n(slice_index si,
                           stip_length_type n,
                           stip_length_type n_min,
                           stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  for (result = n_min+(n-n_min)%2; result<=n; result += 2)
    if (attack_has_solution_in_n(next,result,n_max_unsolvable)<=result)
      break;
    else
      n_max_unsolvable = result;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
degenerate_tree_direct_has_solution_in_n(slice_index si,
                                         stip_length_type n,
                                         stip_length_type n_max_unsolvable)
{
  stip_length_type result = n+2;
  stip_length_type const parity = n%2;
  stip_length_type n_min = n_max_unsolvable+1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  if (n>max_length_short_solutions+2-parity)
  {
    if (max_length_short_solutions>=slack_length_battle+2)
    {
      stip_length_type const n_interm = max_length_short_solutions-parity;
      result = delegate_has_solution_in_n(si,n_interm,n_min,n_max_unsolvable);
      if (result>n_interm)
        result = delegate_has_solution_in_n(si,n,n,n_interm);
    }
    else
      result = delegate_has_solution_in_n(si,n,n,n_max_unsolvable);
  }
  else
    result = delegate_has_solution_in_n(si,n,n_min,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* **************** Stipulation instrumentation ***************
 */

static void degenerate_tree_inserter_attack_move(slice_index si,
                                                 stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.branch.length>=slack_length_battle+2)
    pipe_replace(si,alloc_degenerate_tree_guard_slice());
  else
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument stipulation with STDegenerateTree slices
 * @param si identifies slice where to start
 */
void stip_insert_degenerate_tree_guards(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STAttackFindShortest,
                                           &degenerate_tree_inserter_attack_move);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
