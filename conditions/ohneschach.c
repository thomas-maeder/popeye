#include "conditions/ohneschach.h"
#include "pydata.h"
#include "pymsg.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/temporary_hacks.h"
#include "debugging/trace.h"

#include <assert.h>

static void insert_stop(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STOhneschachStopIfCheckAndNotMate);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void ohneschach_insert_check_guards(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,STNotEndOfBranchGoal,&insert_stop);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean immobile(Side side)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  /* ohneschach_stop_if_check_solve() may invoke itself recursively. Protect
   * ourselves from infinite recursion. */
  if (nbply>250)
    FtlMsg(ChecklessUndecidable);

  result = solve(slices[temporary_hack_immobility_tester[side]].next2,length_unspecified)==has_solution;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type ohneschach_stop_if_check_and_not_mate_solve(slice_index si,
                                                             stip_length_type n)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (echecc(slices[si].starter) && !immobile(slices[si].starter))
    result = slack_length-2;
  else
    result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
