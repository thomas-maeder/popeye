#include "conditions/ghost_chess.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/move_player.h"
#include "conditions/haunted_chess.h"
#include "debugging/trace.h"

#include <assert.h>
#include <string.h>

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type ghost_chess_ghost_rememberer_attack(slice_index si,
                                                     stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (pprise[nbply]==vide)
    result = attack(slices[si].next1,n);
  else
  {
    haunted_chess_remember_ghost();
    SETFLAG(ghosts[nr_ghosts-1].ghost_flags,Uncapturable);
    result = attack(slices[si].next1,n);
    haunted_chess_forget_ghost();
  }

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
stip_length_type ghost_chess_ghost_rememberer_defend(slice_index si,
                                                     stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (pprise[nbply]==vide)
    result = defend(slices[si].next1,n);
  else
  {
    haunted_chess_remember_ghost();
    SETFLAG(ghosts[nr_ghosts-1].ghost_flags,Uncapturable);
    result = defend(slices[si].next1,n);
    haunted_chess_forget_ghost();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_ghost_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STGhostChessGhostRememberer);
  stip_instrument_moves(si,STHauntedChessGhostSummoner);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
