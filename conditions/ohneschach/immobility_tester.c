#include "conditions/ohneschach/immobility_tester.h"
#include "pydata.h"
#include "pyproc.h"
#include "pymsg.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/boolean/and.h"
#include "solving/legal_move_counter.h"
#include "trace.h"

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
    slice_index const proxy1 = alloc_proxy_slice();
    slice_index const proxy2 = alloc_proxy_slice();
    slice_index const next = slices[si].u.pipe.next;
    slice_index const testerNonchecking = alloc_pipe(STOhneschachImmobilityTesterNonchecking);
    slice_index const testerAny = alloc_pipe(STOhneschachImmobilityTesterAny);

    pipe_link(si,alloc_and_slice(proxy1,proxy2));

    pipe_link(proxy1,testerNonchecking);
    pipe_link(testerNonchecking,next);
    {
      slice_index const prototypes[] =
      {
          alloc_pipe(STOhneschachSuspender),
          alloc_pipe(STOhneschachCheckGuard)
      };
      enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
      branch_insert_slices(testerNonchecking,prototypes,nr_prototypes);
    }

    pipe_link(proxy2,testerAny);
    pipe_link(testerAny,stip_deep_copy(next));

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

static boolean ohneschach_find_any_move(Side side)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  move_generation_mode= move_generation_optimized_by_killer_move;
  TraceValue("->%u\n",move_generation_mode);
  genmove(side);

  while (!result && encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply))
      /* no test for self-check necessary
       * was already done by ohneschach_pos_legal() */
      result = true;

    repcoup();
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_ohneschach_suspended;

static boolean ohneschach_find_nonchecking_move(Side side)
{
  boolean result = false;
  Side const ad = advers(side);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  move_generation_mode= move_generation_optimized_by_killer_move;
  TraceValue("->%u\n",move_generation_mode);
  genmove(side);

  /* temporarily suspend Ohneschach to avoid the expensive test for checkmate
   * in ohneschach_pos_legal()
   */
  is_ohneschach_suspended = true;

  while (!result && encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !echecc(nbply,side) && !echecc(nbply,ad))
      result = true;
    repcoup();
  }

  is_ohneschach_suspended = false;

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a side is immobile in Ohneschach
 * @return true iff side is immobile
 */
boolean ohneschach_immobile(Side side)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  /* ohneschach_immobile() may invoke itself recursively. Protect ourselves from
   * infinite recursion. */
  if (nbply>maxply-2)
    FtlMsg(ChecklessUndecidable);

  /* first try to find a move that doesn't deliver check ... */
  if (ohneschach_find_nonchecking_move(side))
    result = false;

  /* ... then try to find a move that delivers mate. This is efficient
   * because determining whether ad is immobile is costly.
   */
  else if (ohneschach_find_any_move(side))
    result = false;

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

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type ohneschach_immobility_tester_nonchecking_has_solution(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* avoid concurrent counts */
  assert(legal_move_counter_count==0);

  /* stop counting once we have >0 legal moves */
  legal_move_counter_interesting = 0;

  slice_has_solution(slices[si].u.pipe.next);

  result = legal_move_counter_count==1 ? has_no_solution : has_solution;

  /* clean up after ourselves */
  legal_move_counter_count = 0;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type ohneschach_immobility_tester_any_has_solution(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (ohneschach_find_any_move(slices[si].starter))
    result = has_no_solution;
  else
    result = has_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type ohneschach_suspender_can_help(slice_index si,
                                               stip_length_type n)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  is_ohneschach_suspended = true;
  result = can_help(slices[si].u.pipe.next,n);
  is_ohneschach_suspended = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type ohneschach_check_guard_can_help(slice_index si,
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
    result = can_help(slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
