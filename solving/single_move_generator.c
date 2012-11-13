#include "solving/single_move_generator.h"
#include "stipulation/stipulation.h"
#include "pydata.h"
#include "pyproc.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"

#include <assert.h>

/* In conditions such as Ohneschach, there may recursive infocations of
 * single_move_generator_attack in different plys */
static square square_departure[maxply+1];
static square square_arrival[maxply+1];
static square square_capture[maxply+1];

/* Allocate a STSingleMoveGenerator slice.
 * @return index of allocated slice
 */
slice_index alloc_single_move_generator_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STSingleMoveGenerator);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Initialise the next1 move generation
 * @param sq_departure departure square of move to be generated
 * @param sq_arrival arrival square of move to be generated
 * @param sq_capture capture square of move to be generated
 */
void init_single_move_generator(square sq_departure,
                                square sq_arrival,
                                square sq_capture)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceFunctionParamListEnd();

  /* avoid concurrent generations */
  assert(square_departure[nbply+1]==initsquare);

  square_departure[nbply+1] = sq_departure;
  square_arrival[nbply+1] = sq_arrival;
  square_capture[nbply+1] = sq_capture;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type single_move_generator_solve(slice_index si,
                                              stip_length_type n)
{
  stip_length_type result;
  Side const side_at_move = slices[si].starter;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  nextply();
  trait[nbply] = side_at_move;
  add_to_move_generation_stack(square_departure[nbply],
                               square_arrival[nbply],
                               square_capture[nbply]);
  result = solve(next,n);
  finply();

  square_departure[nbply+1] = initsquare;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
