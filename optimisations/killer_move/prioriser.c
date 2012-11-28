#include "optimisations/killer_move/prioriser.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* Allocate a STKillerMovePrioriser slice.
 * @return index of allocated slice
 */
slice_index alloc_killer_move_prioriser_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STKillerMovePrioriser);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_killer_move(numecoup i)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",i);
  TraceFunctionParamListEnd();

  result = kpilcd[nbply]==move_generation_stack[i].departure
           && kpilca[nbply]==move_generation_stack[i].arrival;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static numecoup find_killer_move(void)
{
  numecoup result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (result = current_move[nbply]; result>current_move[nbply-1]; --result)
    if (is_killer_move(result))
      break;

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
stip_length_type killer_move_prioriser_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  numecoup killer_index;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  killer_index = find_killer_move();

  if (killer_index>current_move[nbply-1])
  {
    move_generation_stack[current_move[nbply]+1] = move_generation_stack[killer_index];
    memmove(&move_generation_stack[killer_index],
            &move_generation_stack[killer_index+1],
            (current_move[nbply]+1-killer_index)
            * sizeof move_generation_stack[killer_index]);
  }

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
