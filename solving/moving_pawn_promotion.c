#include "solving/moving_pawn_promotion.h"
#include "stipulation/pipe.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "solving/post_move_iteration.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

PieNam current_promotion_of_moving[maxply+1];

static post_move_iteration_id_type prev_post_move_iteration_id[maxply+1];

/* Has a pawn reached a promotion square
 * @param side the pawn's side
 * @param square_reached square reached by the pawn
 * @return true iff square_reached is a promotion square
 */
boolean has_pawn_reached_promotion_square(Side side, square square_reached)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceSquare(square_reached);
  TraceFunctionParamListEnd();

  result = (is_pawn(e[square_reached])
           && PromSq(is_reversepawn(e[square_reached])^side,square_reached)
           && ((!CondFlag[protean] && !TSTFLAG(spec[square_reached],Protean))
               || pprise[nbply]==vide));

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
stip_length_type moving_pawn_promoter_attack(slice_index si, stip_length_type n)
{
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id[nbply])
  {
    boolean const is_prom_square = has_pawn_reached_promotion_square(slices[si].starter,
                                                                     sq_arrival);
    current_promotion_of_moving[nbply] = is_prom_square ? getprompiece[Empty] : Empty;
  }

  if (current_promotion_of_moving[nbply]==Empty)
    result = attack(slices[si].next1,n);
  else
  {
    piece const promotee = (e[sq_arrival]<vide
                            ? -current_promotion_of_moving[nbply]
                            : current_promotion_of_moving[nbply]);

    jouearr[nbply] = promotee;

    move_effect_journal_do_piece_change(move_effect_reason_pawn_promotion,
                                        sq_arrival,promotee);

    result = attack(slices[si].next1,n);

    if (!post_move_iteration_locked[nbply])
    {
      current_promotion_of_moving[nbply] = getprompiece[current_promotion_of_moving[nbply]];
      if (current_promotion_of_moving[nbply]!=Empty)
        lock_post_move_iterations();
    }
  }

  prev_post_move_iteration_id[nbply] = post_move_iteration_id[nbply];

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
stip_length_type moving_pawn_promoter_defend(slice_index si, stip_length_type n)
{
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id[nbply])
  {
    boolean const is_prom_square = has_pawn_reached_promotion_square(slices[si].starter,
                                                                     sq_arrival);
    current_promotion_of_moving[nbply] = is_prom_square ? getprompiece[Empty] : Empty;
  }

  if (current_promotion_of_moving[nbply]==Empty)
    result = defend(slices[si].next1,n);
  else
  {
    piece const promotee = (e[sq_arrival]<vide
                            ? -current_promotion_of_moving[nbply]
                            : current_promotion_of_moving[nbply]);

    jouearr[nbply] = promotee;

    move_effect_journal_do_piece_change(move_effect_reason_pawn_promotion,
                                        sq_arrival,promotee);

    result = defend(slices[si].next1,n);

    if (!post_move_iteration_locked[nbply])
    {
      current_promotion_of_moving[nbply] = getprompiece[current_promotion_of_moving[nbply]];
      if (current_promotion_of_moving[nbply]!=Empty)
        lock_post_move_iterations();
    }
  }

  prev_post_move_iteration_id[nbply] = post_move_iteration_id[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with promotee markers
 */
void stip_insert_moving_pawn_promoters(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves_no_replay(si,STMovingPawnPromoter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
