#include "conditions/anticirce/exchange_special.h"
#include "pypipe.h"
#include "pydata.h"
#include "trace.h"

#include <assert.h>

/* This module provides slice type STAnticirceExchangeSpecial
 */

/* Allocate a STAnticirceExchangeSpecial slice.
 * @return index of allocated slice
 */
slice_index alloc_anticirce_exchange_special_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STAnticirceExchangeSpecial);

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
has_solution_type anticirce_exchange_special_has_solution(slice_index si)
{
  has_solution_type result;
  square const sq_rebirth = sq_rebirth_capturing[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (sq_rebirth==initsquare)
    result = has_no_solution;
  else
  {
    Side const just_moved = advers(slices[si].starter);
    square const sq_diagram = GetPositionInDiagram(spec[sq_rebirth]);
    if (GetPositionInDiagram(spec[sq_diagram])==sq_rebirth
        && (just_moved==White ? e[sq_diagram]>=roib : e[sq_diagram]<=roin)
        && sq_diagram!=sq_rebirth)
      result = slice_has_solution(slices[si].u.pipe.next);
    else
      result = has_no_solution;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type anticirce_exchange_special_solve(slice_index si)
{
  has_solution_type result;
  square const sq_rebirth = sq_rebirth_capturing[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (sq_rebirth==initsquare)
    result = has_no_solution;
  else
  {
    Side const just_moved = advers(slices[si].starter);
    square const sq_diagram = GetPositionInDiagram(spec[sq_rebirth]);
    if (GetPositionInDiagram(spec[sq_diagram])==sq_rebirth
        && (just_moved==White ? e[sq_diagram]>=roib : e[sq_diagram]<=roin)
        && sq_diagram!=sq_rebirth)
      result = slice_solve(slices[si].u.pipe.next);
    else
      result = has_no_solution;
  }

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
