#include "optimisations/count_nr_opponent_moves/prioriser.h"
#include "stipulation/stipulation.h"
#include "solving/machinery/slack_length.h"
#include "stipulation/binary.h"
#include "solving/testing_pipe.h"
#include "optimisations/count_nr_opponent_moves/opponent_moves_counter.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

#include <stdlib.h>

/* Allocate a STOpponentMovesFewMovesPrioriser slice.
 * @param operand2 entry point into counting machinery
 * @return index of allocated slice
 */
slice_index alloc_opponent_moves_few_moves_prioriser_slice(slice_index operand2)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_binary_slice(STOpponentMovesFewMovesPrioriser,no_slice,operand2);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static int compare_nr_opponent_moves(void const *a, void const *b)
{
  move_generation_elmt const * const elmt_a = a;
  move_generation_elmt const * const elmt_b = b;

  return (opponent_moves_few_moves_prioriser_table[elmt_b->id]
          -opponent_moves_few_moves_prioriser_table[elmt_a->id]);
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
void opponent_moves_few_moves_prioriser_solve(slice_index si)
{
  numecoup const base = MOVEBASE_OF_PLY(nbply)+1;
  numecoup const top = CURRMOVE_OF_PLY(nbply)+1;
  unsigned int const nr_moves = top-base;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* Until 4.63, moves with the same number of opponent moves were tried in
   * the inverse order than what we do since. If Popeye's behaviour pre and post
   * 4.63 is compared, consider activating the following statement.
   */
  /* move_generator_invert_move_order(nbply); */

  copyply();
  testing_pipe_solve_delegate(si,slack_length+2);
  fincopiedply();

  qsort(&move_generation_stack[base],
        nr_moves,
        sizeof move_generation_stack[0],
        &compare_nr_opponent_moves);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
