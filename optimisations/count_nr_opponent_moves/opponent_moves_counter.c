#include "optimisations/count_nr_opponent_moves/opponent_moves_counter.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "solving/legal_move_counter.h"
#include "solving/has_solution_type.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

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

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void opponent_moves_counter_solve(slice_index si)
{
  numecoup const move_id = move_generation_stack[CURRMOVE_OF_PLY(nbply)].id;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  legal_move_count_init(UINT_MAX);

  pipe_solve_delegate(si);

  if (solve_result==this_move_is_illegal)
    /* Defenses leading to self check get a big count.
     * But still make sure that we can correctly compute the difference of two
     * counts.
     */
    opponent_moves_few_moves_prioriser_table[move_id] = INT_MAX/2;
  else
    opponent_moves_few_moves_prioriser_table[move_id] = legal_move_counter_count[nbply];

  legal_move_count_fini();

  solve_result = MOVE_HAS_SOLVED_LENGTH();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
