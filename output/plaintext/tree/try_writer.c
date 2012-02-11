#include "output/plaintext/tree/try_writer.h"
#include "pydata.h"
#include "pymsg.h"
#include "pypipe.h"
#include "stipulation/battle_play/defense_play.h"
#include "solving/battle_play/try.h"
#include "solving/trivial_end_filter.h"
#include "output/plaintext/tree/key_writer.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STTryWriter defender slice.
 * @return index of allocated slice
 */
slice_index alloc_try_writer(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STTryWriter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type try_writer_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (table_length(refutations)>0)
  {
    StdString(" ?");
    result = defend(slices[si].u.pipe.next,n);
  }
  else
	  result = key_writer_defend(si,n);

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}
