#include "solving/single_move_generator.h"
#include "solving/move_generator.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"
#include "pydata.h"
#include "pyproc.h"

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
  assert(square_departure[nbply]==initsquare);

  square_departure[nbply] = sq_departure;
  square_arrival[nbply] = sq_arrival;
  square_capture[nbply] = sq_capture;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
stip_length_type single_move_generator_solve(slice_index si,
                                              stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceValue("%u",current_move[nbply]);
  TraceValue("%u\n",current_move[nbply-1]);
  add_to_move_generation_stack(square_departure[nbply],square_arrival[nbply],square_capture[nbply]);
  result = solve(next,n);

  square_departure[nbply] = initsquare;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
