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

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type anticirce_exchange_special_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  square const sq_rebirth = sq_rebirth_capturing[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (sq_rebirth==initsquare)
    result = n+2;
  else
  {
    Side const just_moved = advers(slices[si].starter);
    square const sq_diagram = GetPositionInDiagram(spec[sq_rebirth]);
    if (GetPositionInDiagram(spec[sq_diagram])==sq_rebirth
        && (just_moved==White ? e[sq_diagram]>=roib : e[sq_diagram]<=roin)
        && sq_diagram!=sq_rebirth)
      result = attack(slices[si].u.pipe.next,n);
    else
      result = n+2;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
