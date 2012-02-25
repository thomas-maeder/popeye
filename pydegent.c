#include "pydegent.h"
#include "pydata.h"
#include "pypipe.h"
#include "pybrafrk.h"
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

  max_length_short_solutions = 2*max_length_short+slack_length;
  TraceValue("%u\n",max_length_short_solutions);

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
 * @return length of solution found, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
static stip_length_type delegate_has_solution_in_n(slice_index si,
                                                   stip_length_type n,
                                                   stip_length_type n_min)
{
  stip_length_type result = n+2;
  stip_length_type n_current;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  for (n_current = n_min+(n-n_min)%2; n_current<=n; n_current += 2)
  {
    result = attack(slices[si].u.pipe.next,n_current);
    if (result<=n_current)
      break;
    else
      max_unsolvable = n_current-1;
  }

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
stip_length_type degenerate_tree_attack(slice_index si, stip_length_type n)
{
  stip_length_type result = n+2;
  stip_length_type const parity = (n-slack_length)%2;
  stip_length_type const n_min = max_unsolvable+2;
  stip_length_type const save_max_unsolvable = max_unsolvable;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (n>max_length_short_solutions)
  {
    if (max_length_short_solutions>=slack_length+2)
    {
      stip_length_type const n_interm = max_length_short_solutions-parity;
      result = delegate_has_solution_in_n(si,n_interm,n_min);
      if (result>n_interm)
      {
        max_unsolvable = n_interm-1;
        result = delegate_has_solution_in_n(si,n,n);
      }
    }
    else
      result = delegate_has_solution_in_n(si,n,n);
  }
  else
    result = delegate_has_solution_in_n(si,n,n_min);

  max_unsolvable = save_max_unsolvable;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void degenerate_tree_inserter_attack(slice_index si,
                                            stip_structure_traversal *st)
{
  boolean const * const testing = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (*testing && slices[si].u.branch.length>=slack_length+2)
  {
    slice_index const finder = branch_find_slice(STFindShortest,si);
    if (finder!=no_slice) /* slice may already have been replaced */
      pipe_substitute(finder,alloc_degenerate_tree_guard_slice());
  }
  else
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_testing_pipe(slice_index si, stip_structure_traversal *st)
{
  boolean * const testing = st->param;
  boolean const save_testing = *testing;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  *testing = true;
  stip_traverse_structure(slices[si].u.fork.fork,st );
  *testing = save_testing;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_conditional_pipe(slice_index si,
                                      stip_structure_traversal *st)
{
  boolean * const testing = st->param;
  boolean const save_testing = *testing;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  *testing = true;
  stip_traverse_structure_next_branch(si,st );
  *testing = save_testing;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument stipulation with STDegenerateTree slices
 * @param si identifies slice where to start
 */
void stip_insert_degenerate_tree_guards(slice_index si)
{
  stip_structure_traversal st;
  boolean testing = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&testing);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_testing_pipe,
                                                &remember_testing_pipe);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_conditional_pipe,
                                                &remember_conditional_pipe);
  stip_structure_traversal_override_single(&st,
                                           STReadyForAttack,
                                           &degenerate_tree_inserter_attack);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
