#include "conditions/norsk.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "solving/castling.h"
#include "solving/moving_pawn_promotion.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type norsk_castling_rights_restorer_attack(slice_index si,
                                                       stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  restore_castling_rights(move_generation_stack[current_move[nbply]].arrival);
  result = attack(slices[si].next1,n);

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
stip_length_type norsk_castling_rights_restorer_defend(slice_index si,
                                                       stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  restore_castling_rights(move_generation_stack[current_move[nbply]].arrival);
  result = defend(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static piece norskpiece(piece p)
{
  if (CondFlag[leofamily]) {
    switch (p)
    {
      case leob:
        return maob;
      case leon:
        return maon;
      case maob:
        return leob;
      case maon:
        return leon;
      case vaob:
        return paob;
      case vaon:
        return paon;
      case paob:
        return vaob;
      case paon:
        return vaon;
      default:
        break;
    }
  }
  else if (CondFlag[cavaliermajeur])
  {
    switch (p)
    {
      case db:
        return nb;
      case dn:
        return nn;
      case nb:
        return db;
      case nn:
        return dn;
      case fb:
        return tb;
      case fn:
        return tn;
      case tb:
        return fb;
      case tn:
        return fn;
      default:
        break;
    }
  }
  else
  {
    switch (p)
    {
      case db:
        return cb;
      case dn:
        return cn;
      case cb:
        return db;
      case cn:
        return dn;
      case fb:
        return tb;
      case fn:
        return tn;
      case tb:
        return fb;
      case tn:
        return fn;
      default:
        break;
    }
  }

  return p;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type norsk_arriving_adjuster_attack(slice_index si,
                                                       stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (current_promotion_of_moving[nbply]==Empty)
    replace_arriving_piece(norskpiece(e[move_generation_stack[current_move[nbply]].arrival]));
  result = attack(slices[si].next1,n);

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
stip_length_type norsk_arriving_adjuster_defend(slice_index si,
                                                       stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (current_promotion_of_moving[nbply]==Empty)
    replace_arriving_piece(norskpiece(e[move_generation_stack[current_move[nbply]].arrival]));
  result = defend(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with move tracers
 */
void stip_insert_norsk_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STNorskArrivingAdjuster);
  if (castling_supported)
    stip_instrument_moves_no_replay(si,STNorskCastlingRightsRestorer);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
