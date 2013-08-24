#include "conditions/forced_squares.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include <assert.h>

/* Determine the length of a move in the presence of forced squares; the higher
 * the value the more likely the move is going to be played.
 * @return a value expressing the precedence of this move
 */
int forced_squares_measure_length(void)
{
  square const sq_arrival = move_generation_stack[current_move[nbply]-1].arrival;
  SquareFlags const flag = trait[nbply]==White ? WhForcedSq : BlForcedSq;
  int const result = TSTFLAG(sq_spec[sq_arrival],flag);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%x\n",sq_spec[sq_arrival]);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
