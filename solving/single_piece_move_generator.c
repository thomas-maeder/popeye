#include "solving/single_piece_move_generator.h"
#include "stipulation/stipulation.h"
#include "pydata.h"
#include "pyproc.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"

#include <assert.h>

static square square_departure;
static piece piece_moving;

void init_single_piece_move_generator(square sq_departure, piece pi_moving)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TracePiece(pi_moving);
  TraceFunctionParamListEnd();

  assert(square_departure==initsquare);
  square_departure = sq_departure;
  piece_moving = pi_moving;

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

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type single_piece_move_generator_attack(slice_index si,
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

  TraceValue("%u\n",current_move[nbply]);
  if (side_at_move==White)
    gen_wh_piece(square_departure,piece_moving);
  else
    gen_bl_piece(square_departure,piece_moving);
  TraceValue("%u\n",current_move[nbply]);

  result = attack(next,n);

  finply();

  square_departure = initsquare;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
