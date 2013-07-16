#include "conditions/alphabetic.h"
#include "pydata.h"

#include <assert.h>

/* Determine the length of a move for the Alphabetic Chess; the higher
 * the value the more likely the move is going to be played.
 * @param sq_departure departure square
 * @param sq_arrival arrival square
 * @param sq_capture capture square
 * @return a value expressing the precedence of this move
 */
int alphabetic_measure_length(square sq_departure,
                              square sq_arrival,
                              square sq_capture)
{
  return -((sq_departure/onerow) + onerow*(sq_departure%onerow));
}
