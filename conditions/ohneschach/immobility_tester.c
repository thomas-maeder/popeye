#include "conditions/ohneschach/immobility_tester.h"
#include "stipulation/stipulation.h"
#include "pydata.h"
#include "pyproc.h"
#include "pymsg.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/boolean/and.h"
#include "stipulation/temporary_hacks.h"
#include "solving/solve.h"
#include "debugging/trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether a side is immobile
 */

static void substitute_optimiser(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const proxy_nonchecking = alloc_proxy_slice();
    slice_index const proxy_any = alloc_proxy_slice();
    slice_index const next = slices[si].next1;
    slice_index const tester_nonchecking = alloc_pipe(STImmobilityTester);
    slice_index const tester_any = alloc_pipe(STImmobilityTester);

    pipe_link(si,alloc_and_slice(proxy_nonchecking,proxy_any));

    pipe_link(proxy_nonchecking,tester_nonchecking);
    pipe_link(tester_nonchecking,next);

    pipe_link(proxy_any,tester_any);
    pipe_link(tester_any,stip_deep_copy(next));

    {
      slice_index const prototypes[] =
      {
          alloc_pipe(STOhneschachSuspender),
          alloc_pipe(STOhneschachCheckGuard)
      };
      enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
      branch_insert_slices(tester_nonchecking,prototypes,nr_prototypes);
    }

    {
      /* no test for self-check necessary
       * already done by ohneschach_pos_legal() */
      slice_index const selfcheckguard = branch_find_slice(STSelfCheckGuard,
                                                           tester_any,
                                                           stip_traversal_context_intro);
      assert(selfcheckguard!=no_slice);
      pipe_remove(selfcheckguard);
    }

    pipe_remove(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Replace immobility tester slices to cope with condition Ohneschach
 * @param si where to start (entry slice into stipulation)
 */
void ohneschach_replace_immobility_testers(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STImmobilityTester,
                                           &substitute_optimiser);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type ohneschach_check_guard_solve(slice_index si,
                                               stip_length_type n)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  /* ohneschach_check_guard_solve() may invoke itself recursively. Protect
   * ourselves from infinite recursion. */
  if (nbply>500)
    FtlMsg(ChecklessUndecidable);

  if (echecc(slices[si].starter))
    result = n+2;
  else
    result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
