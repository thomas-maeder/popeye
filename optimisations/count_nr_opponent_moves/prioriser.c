#include "optimisations/count_nr_opponent_moves/prioriser.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/pipe.h"
#include "stipulation/temporary_hacks.h"
#include "solving/single_move_generator.h"
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

static int count_opponent_moves(numecoup i)
{
  int result;

  TraceFunctionEntry(__func__);
  TraceValue("%u",i);
  TraceFunctionParamListEnd();

  init_opponent_moves_counter();

  TraceSquare(move_generation_stack[i].departure);
  TraceSquare(move_generation_stack[i].arrival);
  TraceSquare(move_generation_stack[i].capture);
  TraceText("\n");

  init_single_move_generator(move_generation_stack[i].departure,
                             move_generation_stack[i].arrival,
                             move_generation_stack[i].capture);

  solve(slices[temporary_hack_opponent_moves_counter[trait[nbply]]].next2,
        length_unspecified);

  result = fini_opponent_moves_counter();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

typedef struct
{
  move_generation_elmt move;
  int nr_opponent_moves;
} table_elmt;

static int compare_nr_opponent_moves(const void *a, const void *b)
{
  return  (((table_elmt *)a)->nr_opponent_moves
           - ((table_elmt *)b)->nr_opponent_moves);
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
  table_elmt table[100];
  unsigned int const nr_moves = current_move[nbply]-current_move[nbply-1];
  unsigned int table_index;
  numecoup i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (i = current_move[nbply-1]+1, table_index = 0; i<=current_move[nbply]; ++i, ++table_index)
  {
    nextply(slices[si].starter);
    table[table_index].move = move_generation_stack[i];
    table[table_index].nr_opponent_moves = count_opponent_moves(i);
    finply();
  }

  qsort(table, nr_moves, sizeof table[0], &compare_nr_opponent_moves);

  for (i = current_move[nbply-1]+1, table_index = nr_moves; i<=current_move[nbply]; ++i, --table_index)
    move_generation_stack[i] = table[table_index-1].move;

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
