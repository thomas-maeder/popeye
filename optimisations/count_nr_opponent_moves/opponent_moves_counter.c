#include "optimisations/count_nr_opponent_moves/opponent_moves_counter.h"
#include "pypipe.h"
#include "pydata.h"
#include "solving/legal_move_counter.h"
#include "trace.h"

#include <assert.h>

/* current value of the count */
int opponent_moves_counter_count;

enum
{
  count_for_selfcheck = INT_MAX
};

/* Allocate a STOpponentMovesCounter slice.
 * @return index of allocated slice
 */
slice_index alloc_opponent_moves_counter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STOpponentMovesCounter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Initialise counting the opponent's moves after the move just generated */
void init_opponent_moves_counter()
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(legal_move_counter_count[nbply+2]==0);
  legal_move_counter_interesting[nbply+2] = UINT_MAX;

  /* moves leading to self check get maximum count */
  opponent_moves_counter_count = count_for_selfcheck;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Retrieve the number opponent's moves after the move just generated
 * @return number of opponent's moves
 */
int fini_opponent_moves_counter()
{
  int const result = opponent_moves_counter_count;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  legal_move_counter_count[nbply+2] = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",result);
  TraceFunctionResultEnd();
  return result;
}


/* Solve in a number of half-moves
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move leading to the current position has
 *                           turned out to be illegal
 *            n   solution found
 *            n+2 no solution found
 */
stip_length_type opponent_moves_counter_help(slice_index si,
                                             stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  /* make sure that the iteration stops when there is no self check */
  assert(opponent_moves_counter_count==count_for_selfcheck);

  result = help(slices[si].u.pipe.next,n);
  if (slack_length<=result && result<n+4)
  {
    opponent_moves_counter_count = legal_move_counter_count[nbply+1];
    result = n;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
