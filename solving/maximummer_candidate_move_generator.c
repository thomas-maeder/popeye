#include "solving/maximummer_candidate_move_generator.h"
#include "pydata.h"
#include "pyproc.h"
#include "pypipe.h"
#include "trace.h"

#include <assert.h>

/* In conditions such as Ohneschach, there may recursive infocations of
 * maximummer_candidate_move_generator_can_help in different plys */
static square square_departure[maxply+1];
static square square_arrival[maxply+1];
static square square_capture[maxply+1];
static square square_mars_rebirth[maxply+1];

/* Allocate a STMaximummerCandidateMoveGenerator slice.
 * @return index of allocated slice
 */
slice_index alloc_maximummer_candidate_move_generator_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STMaximummerCandidateMoveGenerator);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Initialise the next move generation
 * @param sq_departure departure square of move to be generated
 * @param sq_arrival arrival square of move to be generated
 * @param sq_capture capture square of move to be generated
 * @param sq_mren Mars Circe rebirth square
 */
void maximummer_candidate_move_generator_init_next(square sq_departure,
                                                   square sq_arrival,
                                                   square sq_capture,
                                                   square sq_mren)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceSquare(sq_mren);
  TraceFunctionParamListEnd();

  /* avoid concurrent generations */
  assert(square_departure[nbply+1]==initsquare);

  square_departure[nbply+1] = sq_departure;
  square_arrival[nbply+1] = sq_arrival;
  square_capture[nbply+1] = sq_capture;
  square_mars_rebirth[nbply+1] = sq_mren;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether the slice has a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type
maximummer_candidate_move_generator_can_help(slice_index si,
                                             stip_length_type n)
{
  stip_length_type result;
  Side const side_at_move = slices[si].starter;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  nextply(nbply);
  trait[nbply] = side_at_move;
  add_to_move_generation_stack(square_departure[nbply],
                               square_arrival[nbply],
                               square_capture[nbply],
                               square_mars_rebirth[nbply]);
  result = can_help(next,n);
  finply();

  square_departure[nbply+1] = initsquare;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
