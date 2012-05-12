#include "conditions/ohneschach/immobility_tester.h"
#include "pystip.h"
#include "pydata.h"
#include "pyproc.h"
#include "pymsg.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/boolean/and.h"
#include "stipulation/temporary_hacks.h"
#include "stipulation/battle_play/attack_play.h"
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

  stip_traverse_structure_children(si,st);

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

static boolean is_ohneschach_suspended;

/* Determine whether a side is immobile in Ohneschach
 * @return true iff side is immobile
 */
static boolean ohneschach_immobile(Side side)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  /* ohneschach_immobile() may invoke itself recursively. Protect ourselves from
   * infinite recursion. */
  if (nbply>maxply-2)
    FtlMsg(ChecklessUndecidable);

  result = attack(slices[temporary_hack_immobility_tester[side]].next2,length_unspecified)==has_solution;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the move just played has led to a legal position according
 * to condition Ohneschach
 * @param just_moved identifies the side that has just moved
 * @return true iff the position reached is legal according to Ohneschach
 */
boolean ohneschach_pos_legal(Side just_moved)
{
  boolean result = true;
  Side const ad = advers(just_moved);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,just_moved,"");
  TraceFunctionParamListEnd();

  if (is_ohneschach_suspended)
    result = true;
  else if (echecc(nbply,just_moved))
    result = false;
  else if (echecc(nbply,ad) && !ohneschach_immobile(ad))
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type ohneschach_suspender_attack(slice_index si,
                                             stip_length_type n)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  is_ohneschach_suspended = true;
  result = attack(slices[si].next1,n);
  is_ohneschach_suspended = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type ohneschach_check_guard_attack(slice_index si,
                                               stip_length_type n)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (echecc(nbply,slices[si].starter))
    result = n+2;
  else
    result = attack(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
