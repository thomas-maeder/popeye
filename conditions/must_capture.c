#include "conditions/must_capture.h"
#include "position/position.h"
#include "debugging/trace.h"

#include <assert.h>

/* Determine the length of a move in Black/White must capture; the higher
 * the value the more likely the move is going to be played.
 * @param sq_departure departure square
 * @param sq_arrival arrival square
 * @param sq_capture capture square
 * @return a value expressing the precedence of this move
 */
int must_capture_measure_length(square sq_departure,
                                square sq_arrival,
                                square sq_capture)
{
  return !is_square_empty(sq_capture);
}
