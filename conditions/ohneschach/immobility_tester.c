#include "conditions/ohneschach/immobility_tester.h"
#include "pydata.h"
#include "pyproc.h"
#include "pymsg.h"
#include "trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether a side is immobile
 */

static void substitute_optimiser(slice_index si, stip_structure_traversal *st)
{
  slices[si].type = STOhneschachImmobilityTester;
  stip_traverse_structure_children(si,st);
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
        && !echecc(nbply,ad) && !echecc(nbply,side))
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
has_solution_type ohneschach_immobility_tester_has_solution(slice_index si)
{
  has_solution_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* first try to find a move that doesn't deliver check ... */
  if (!ohneschach_find_nonchecking_move(slices[si].starter)
      /* ... then try to find a move that delivers mate. This is efficient
       * because determining whether ad is immobile is costly.
       */
       && !ohneschach_find_any_move(slices[si].starter))
    result = has_solution;
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
