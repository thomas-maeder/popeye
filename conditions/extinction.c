#include "conditions/extinction.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"
#include "stipulation/move_player.h"
#include "debugging/trace.h"

#include <assert.h>

static unsigned int prev_nbpiece[derbla];

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_extinction_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STExtinctionRememberThreatened);
  stip_instrument_moves(si,STExtinctionTester);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Detect whether the move just played has extincted a kind
 */
static boolean move_extincts_kind(Side starter)
{
  piece p;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,starter,"");
  TraceFunctionParamListEnd();

  for (p = roib; p<derbla; ++p)
    if (prev_nbpiece[p]>0
        && nbpiece[starter==White ? p : -p]==0)
    {
      result = true;
      break;
    }

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
stip_length_type extinction_remember_threatened_solve(slice_index si,
                                                       stip_length_type n)
{
  stip_length_type result;
  Side const starter = slices[si].starter;
  slice_index const next = slices[si].next1;
  piece p;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (p = roib; p<derbla; p++)
    prev_nbpiece[p] = nbpiece[starter==White ? p : -p];

  result = solve(next,n);

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
stip_length_type extinction_tester_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  Side const starter = slices[si].starter;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (move_extincts_kind(starter))
    result = slack_length-2;
  else
    result = solve(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
