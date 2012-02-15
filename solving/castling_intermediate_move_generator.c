#include "solving/castling_intermediate_move_generator.h"
#include "pydata.h"
#include "pyproc.h"
#include "pypipe.h"
#include "trace.h"

#include <assert.h>

static square square_departure;
static square square_arrival;

/* Allocate a STCastlingIntermediateMoveGenerator slice.
 * @return index of allocated slice
 */
slice_index alloc_castling_intermediate_move_generator_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STCastlingIntermediateMoveGenerator);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Initialise the next move generation
 * @param sq_departure departure square of move to be generated
 * @param sq_arrival arrival square of move to be generated
 */
void castling_intermediate_move_generator_init_next(square sq_departure,
                                                    square sq_arrival)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();

  /* avoid concurrent generations */
  assert(square_departure==initsquare);

  square_departure = sq_departure;
  square_arrival = sq_arrival;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type castling_intermediate_move_generator_help(slice_index si,
                                                           stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  numecoup const save_repere = repere[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  /* We work within a ply for which moves are being generated right now.
   * That's why we don't do nextply()/finply() - we just trick our successor
   * slices into believing that this intermediate move is the only one in the
   * ply.
   */
  repere[nbply] = nbcou;
  empile(square_departure,square_arrival,square_arrival);
  result = help(next,n);
  repere[nbply] = save_repere;

  /* clean up after ourselves */
  square_departure = initsquare;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
