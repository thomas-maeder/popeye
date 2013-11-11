#include "optimisations/killer_move/prioriser.h"
#include "solving/move_generator.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/pipe.h"
#include "optimisations/killer_move/killer_move.h"
#include "debugging/trace.h"

#include <assert.h>
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

  result = killer_moves[nbply].departure==move_generation_stack[i].departure
           && killer_moves[nbply].arrival==move_generation_stack[i].arrival;

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

  for (result = CURRMOVE_OF_PLY(nbply); result>MOVEBASE_OF_PLY(nbply); --result)
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
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
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

  if (killer_index>MOVEBASE_OF_PLY(nbply))
    move_generator_priorise(killer_index);

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
