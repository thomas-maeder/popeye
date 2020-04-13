#include "conditions/forced_squares.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include "debugging/assert.h"

/* Determine the length of a move in the presence of forced squares; the higher
 * the value the more likely the move is going to be played.
 * @return a value expressing the precedence of this move
 */
mummer_length_type forced_squares_measure_length(void)
{
  square const sq_arrival = move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival;
  SquareFlags const flag = trait[nbply]==White ? WhForcedSq : BlForcedSq;

  /* +1 so that consequent forced squares work correctly */
  mummer_length_type const result = TSTFLAG(sq_spec(sq_arrival),flag)+1;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%x",sq_spec(sq_arrival));
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
