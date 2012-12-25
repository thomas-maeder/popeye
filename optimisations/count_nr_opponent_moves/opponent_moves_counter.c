#include "optimisations/count_nr_opponent_moves/opponent_moves_counter.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "pydata.h"
#include "solving/legal_move_counter.h"
#include "stipulation/has_solution_type.h"
#include "debugging/trace.h"

#include <assert.h>
#include <limits.h>

/* current value of the count */
static int opponent_moves_counter_count;

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

  assert(legal_move_counter_count[nbply+1]==0);
  legal_move_counter_interesting[nbply+1] = UINT_MAX;

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

  legal_move_counter_count[nbply+1] = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",result);
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
stip_length_type opponent_moves_counter_solve(slice_index si,
                                              stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = solve(slices[si].next1,n);

  if (result==opponent_self_check)
    /* Defenses leading to self check get a big count.
     * But still make sure that we can correctly compute the difference of two
     * counts.
     */
    opponent_moves_counter_count = INT_MAX/2;
  else
    opponent_moves_counter_count = legal_move_counter_count[nbply];

  TraceValue("%d\n",opponent_moves_counter_count);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
