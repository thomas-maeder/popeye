#include "solving/non_king_move_generator.h"
#include "stipulation/stipulation.h"
#include "pydata.h"
#include "pyproc.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/pipe.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

/* Allocate a STNonKingMoveGenerator slice.
 * @return index of allocated slice
 */
slice_index alloc_non_king_move_generator_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STNonKingMoveGenerator);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean advance_departure_square(Side side,
                                        square const **next_square_to_try)
{
  if (TSTFLAG(PieSpExFlags,Neutral))
    initialise_neutrals(advers(side));

  while (true)
  {
    square const sq_departure = **next_square_to_try;
    if (sq_departure==0)
      break;
    else
    {
      piece p = e[sq_departure];
      TraceSquare(sq_departure);
      TracePiece(abs(p));
      TraceText("\n");
      ++*next_square_to_try;
      if (p!=vide)
      {
        if (TSTFLAG(spec[sq_departure],Neutral))
          p = -p;

        TraceEnumerator(Side,side,"\n");
        if (side==White)
        {
          if (p>obs && sq_departure!=king_square[White])
          {
            gen_wh_piece(sq_departure,p);
            return true;
          }
        }
        else
        {
          if (p<vide && sq_departure!=king_square[Black])
          {
            gen_bl_piece(sq_departure,p);
            return true;
          }
        }
      }
    }
  }

  return false;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type non_king_move_generator_solve(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result = slack_length-2;
  Side const side_at_move = slices[si].starter;
  square const *next_square_to_try = boardnum;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  nextply();
  trait[nbply] = side_at_move;

  while (result<slack_length
         && advance_departure_square(side_at_move,&next_square_to_try))
    result = solve(slices[si].next1,n);

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
