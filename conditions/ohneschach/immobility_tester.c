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

  /* temporarily deactivate ohneschach to avoid the expensive test for checkmate
   * after moves that deliver check */
  CondFlag[ohneschach] = false;

  while (!result && encore())
  {
    if (jouecoup(nbply,first_play) && TraceCurrentMove(nbply)
        && !echecc(nbply,ad) && !echecc(nbply,side))
      result = true;
    repcoup();
  }

  CondFlag[ohneschach] = true;

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

  if (ohneschach_immobile(slices[si].starter))
    result = slice_has_solution(slices[si].u.immobility_tester.next);
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
