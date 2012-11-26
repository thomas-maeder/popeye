#include "solving/single_piece_move_generator.h"
#include "stipulation/stipulation.h"
#include "pydata.h"
#include "pyproc.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"

#include <assert.h>

static square square_departure;

void init_single_piece_move_generator(square sq_departure)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  assert(square_departure==initsquare);
  square_departure = sq_departure;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a STSinglePieceMoveGenerator slice.
 * @return index of allocated slice
 */
slice_index alloc_single_piece_move_generator_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STSinglePieceMoveGenerator);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type single_piece_move_generator_solve(slice_index si,
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

  TraceValue("%u\n",current_move[nbply]);

  {
    piece p = e[square_departure];

    if (TSTFLAG(spec[square_departure],Neutral))
      p = -p;

    move_generation_mode = move_generation_not_optimized;
    if (side_at_move==White)
      gen_wh_piece(square_departure,p);
    else
      gen_bl_piece(square_departure,p);
  }

  square_departure = initsquare;

  TraceValue("%u\n",current_move[nbply]);

  result = solve(next,n);

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
