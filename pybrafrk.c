#include "pybrafrk.h"
#include "pystip.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "solving/solving.h"
#include "debugging/trace.h"

#include <assert.h>

/* **************** Initialisation ***************
 */

/* Allocate a new branch fork slice
 * @param type which slice type
 * @param fork identifies proxy slice that leads towards goal
 *                from the branch
 * @return newly allocated slice
 */
slice_index alloc_branch_fork(slice_type type, slice_index fork)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParam("%u",fork);
  TraceFunctionParamListEnd();

  result = alloc_pipe(type);
  slices[result].next2 = fork;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void branch_fork_detect_starter(slice_index si, stip_structure_traversal *st)
{
  slice_index const fork = slices[si].next2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_next_branch(si,st);

  if (slices[si].starter==no_side)
  {
    if (slices[fork].starter==no_side)
    {
      stip_traverse_structure_children_pipe(si,st);
      slices[si].starter = slices[slices[si].next1].starter;
    }
    else
      slices[si].starter = slices[fork].starter;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Callback to stip_spin_off_testers
 * Spin a tester slice off a fork slice
 * @param si identifies the testing pipe slice
 * @param st address of structure representing traversal
 */
void stip_spin_off_testers_fork(slice_index si, stip_structure_traversal *st)
{
  boolean const * const spinning_off = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (*spinning_off)
  {
    slices[si].tester = copy_slice(si);
    stip_traverse_structure_children(si,st);
    link_to_branch(slices[si].tester,slices[slices[si].next1].tester);
    slices[slices[si].tester].next2 = slices[slices[si].next2].tester;
  }
  else
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
