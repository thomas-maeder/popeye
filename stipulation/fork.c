#include "stipulation/fork.h"
#include "stipulation/stipulation.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

#include <stdio.h>  /* included for fprintf(FILE *, char const *, ...) */
#include <stdlib.h> /* included for exit(int) */

/* **************** Initialisation ***************
 */

/* Allocate a new branch fork slice
 * @param type which slice type
 * @param fork identifies proxy slice that leads towards goal
 *                from the branch
 * @return newly allocated slice
 */
slice_index alloc_fork_slice(slice_type type, slice_index fork)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type);
  TraceFunctionParam("%u",fork);
  TraceFunctionParamListEnd();

  result = alloc_pipe(type);
  SLICE_NEXT2(result) = fork;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void fork_detect_starter(slice_index si, stip_structure_traversal *st)
{
  slice_index const fork = SLICE_NEXT2(si);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_next_branch(si,st);

  if (SLICE_STARTER(si)==no_side)
  {
    if (SLICE_STARTER(fork)==no_side)
    {
      stip_traverse_structure_children_pipe(si,st);
      SLICE_STARTER(si) = SLICE_STARTER(SLICE_NEXT1(si));
    }
    else
      SLICE_STARTER(si) = SLICE_STARTER(fork);
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
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  SLICE_TESTER(si) = copy_slice(si);
  if (SLICE_TESTER(si)==no_slice)
  {
    fprintf(stderr, "\nOUT OF SPACE: Unable to copy slice in %s in %s -- aborting.\n", __func__, __FILE__);
    exit(2); /* TODO: Do we have to exit here? */
  }
  stip_traverse_structure_children(si,st);
  link_to_branch(SLICE_TESTER(si),SLICE_TESTER(SLICE_NEXT1(si)));
  SLICE_NEXT2(SLICE_TESTER(si)) = SLICE_TESTER(SLICE_NEXT2(si));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
