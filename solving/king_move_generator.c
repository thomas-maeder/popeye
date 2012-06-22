#include "solving/king_move_generator.h"
#include "pystip.h"
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

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type king_move_generator_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  Side const attacker = slices[si].starter;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_generation_mode = move_generation_not_optimized;
  nextply(nbply);
  trait[nbply] = attacker;
  if (TSTFLAG(PieSpExFlags,Neutral))
    initneutre(advers(attacker));
  generate_king_moves(attacker);
  result = attack(next,n);
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type king_move_generator_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  Side const defender = slices[si].starter;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_generation_mode = move_generation_not_optimized;
  nextply(nbply);
  trait[nbply] = defender;
  if (TSTFLAG(PieSpExFlags,Neutral))
    initneutre(advers(defender));
  generate_king_moves(defender);
  result = defend(next,n);
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
