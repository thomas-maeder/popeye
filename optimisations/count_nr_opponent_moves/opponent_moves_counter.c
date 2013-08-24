#include "optimisations/count_nr_opponent_moves/opponent_moves_counter.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "solving/legal_move_counter.h"
#include "solving/post_move_iteration.h"
#include "stipulation/has_solution_type.h"
#include "debugging/trace.h"

#include <assert.h>
#include <limits.h>

int opponent_moves_few_moves_prioriser_table[toppile + 1];

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

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type opponent_moves_counter_solve(slice_index si,
                                              stip_length_type n)
{
  stip_length_type result;
  numecoup const move_id = move_generation_stack[current_move[nbply]-1].id;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(legal_move_counter_count[nbply]==0);
  legal_move_counter_interesting[nbply] = UINT_MAX;

  result = solve(slices[si].next1,n);

  if (result==previous_move_is_illegal)
    /* Defenses leading to self check get a big count.
     * But still make sure that we can correctly compute the difference of two
     * counts.
     */
    opponent_moves_few_moves_prioriser_table[move_id] = INT_MAX/2;
  else
    opponent_moves_few_moves_prioriser_table[move_id] = legal_move_counter_count[nbply];

  legal_move_counter_count[nbply] = 0;

  /* one promotion per pawn move is normally enough */
  post_move_iteration_locked[nbply] = false;

  result = n;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
