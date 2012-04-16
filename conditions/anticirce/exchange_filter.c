#include "conditions/anticirce/exchange_filter.h"
#include "pypipe.h"
#include "pydata.h"
#include "debugging/trace.h"

#include <assert.h>

/* This module provides slice type STAnticirceExchangeFilter
 */

/* Allocate a STAnticirceExchangeFilter slice.
 * @return index of allocated slice
 */
slice_index alloc_anticirce_exchange_filter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STAnticirceExchangeFilter);

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
stip_length_type anticirce_exchange_filter_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  square const sq_rebirth = sq_rebirth_capturing[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (sq_rebirth==initsquare)
    result = attack(next,n);
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
