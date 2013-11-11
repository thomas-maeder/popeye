#include "optimisations/count_nr_opponent_moves/prioriser.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/pipe.h"
#include "stipulation/temporary_hacks.h"
#include "optimisations/count_nr_opponent_moves/opponent_moves_counter.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

/* Allocate a STOpponentMovesFewMovesPrioriser slice.
 * @return index of allocated slice
 */
slice_index alloc_opponent_moves_few_moves_prioriser_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STOpponentMovesFewMovesPrioriser);

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
stip_length_type opponent_moves_few_moves_prioriser_solve(slice_index si,
                                                          stip_length_type n)
{
  stip_length_type result;
  numecoup const base = MOVEBASE_OF_PLY(nbply)+1;
  numecoup const top = CURRMOVE_OF_PLY(nbply)+1;
  unsigned int const nr_moves = top-base;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  copyply();

  solve(slices[temporary_hack_opponent_moves_counter[trait[nbply]]].next2,
        length_unspecified);

  finply();

  qsort(&move_generation_stack[base],
        nr_moves,
        sizeof move_generation_stack[0],
        &compare_nr_opponent_moves);

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
