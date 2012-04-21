#include "solving/single_move_generator_with_king_capture.h"
#include "pystip.h"
#include "pydata.h"
#include "pyproc.h"
#include "pypipe.h"
#include "debugging/trace.h"

#include <assert.h>

static square square_departure[maxply];
static square square_arrival[maxply];
static square square_capture[maxply];

/* Allocate a STSingleMoveGeneratorWithKingCapture slice.
 * @return index of allocated slice
 */
slice_index alloc_single_move_generator_with_king_capture_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STSingleMoveGeneratorWithKingCapture);

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
void single_move_generator_with_king_capture_init_next(square sq_departure,
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

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type single_move_generator_with_king_capture_attack(slice_index si,
                                                                stip_length_type n)
{
  stip_length_type result;
  Side const side_at_move = slices[si].starter;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  nextply(nbply);
  trait[nbply] = side_at_move;
  init_move_generation_optimizer();
  k_cap = true;         /* allow K capture */
  empile(square_departure[nbply],square_arrival[nbply],square_capture[nbply]);
  k_cap = false;
  finish_move_generation_optimizer();
  result = attack(next,n);
  finply();

  square_departure[nbply+1] = initsquare;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
