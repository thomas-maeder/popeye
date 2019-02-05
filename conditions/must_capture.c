#include "conditions/must_capture.h"
#include "position/position.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Determine the length of a move in Black/White must capture; the higher
 * the value the more likely the move is going to be played.
 * @return a value expressing the precedence of this move
 */
mummer_length_type must_capture_measure_length(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceSquare(sq_capture);TraceEOL();

  result = !is_no_capture(move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
