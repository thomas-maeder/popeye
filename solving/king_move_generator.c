#include "solving/king_move_generator.h"
#include "stipulation/stipulation.h"
#include "pydata.h"
#include "pyproc.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

/* Allocate a STKingMoveGenerator slice.
 * @return index of allocated slice
 */
slice_index alloc_king_move_generator_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STKingMoveGenerator);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Generate moves for the king (if any) of a side
 * @param side side for which to generate king moves
 */
void generate_king_moves(Side side)
{
  if (king_square[side]!=initsquare)
  {
    if (side==White)
      gen_wh_piece(king_square[White],abs(e[king_square[White]]));
    else
      gen_bl_piece(king_square[Black],-abs(e[king_square[Black]]));
  }
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type king_move_generator_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  Side const attacker = slices[si].starter;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  nextply();
  trait[nbply] = attacker;
  generate_king_moves(attacker);
  result = solve(next,n);
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
