#include "conditions/circe/promotion.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "solving/moving_pawn_promotion.h"
#include "solving/post_move_iteration.h"
#include "debugging/trace.h"

#include <assert.h>

static post_move_iteration_id_type prev_post_move_iteration_id[maxply+1];

static void place_promotee(void)
{
  square const sq_rebirth = current_circe_rebirth_square[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  --nbpiece[e[sq_rebirth]];
  e[sq_rebirth] = e[sq_rebirth]<vide ? -current_promotion_of_capturee[nbply] : current_promotion_of_capturee[nbply];
  ++nbpiece[e[sq_rebirth]];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type circe_promoter_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id[nbply])
  {
    square const sq_rebirth = current_circe_rebirth_square[nbply];

    /* check for both sides - this has to work for Circe Parrain and neutrals as well! */
    if ((TSTFLAG(spec[sq_rebirth],White) && has_pawn_reached_promotion_square(White,sq_rebirth))
        || (TSTFLAG(spec[sq_rebirth],Black) && has_pawn_reached_promotion_square(Black,sq_rebirth)))
      current_promotion_of_capturee[nbply] = getprompiece[Empty];
    else
      current_promotion_of_capturee[nbply] = Empty;
  }

  if (current_promotion_of_capturee[nbply]==Empty)
    result = attack(slices[si].next1,n);
  else
  {
    place_promotee();

    result = attack(slices[si].next1,n);

    if (!post_move_iteration_locked[nbply])
    {
      current_promotion_of_capturee[nbply] = getprompiece[current_promotion_of_capturee[nbply]];
      TracePiece(current_promotion_of_capturee[nbply]);TraceText("\n");

      if (current_promotion_of_capturee[nbply]!=Empty)
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
stip_length_type circe_promoter_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id[nbply])
  {
    square const sq_rebirth = current_circe_rebirth_square[nbply];

    /* check for both sides - this has to work for Circe Parrain and neutrals as well! */
    if ((TSTFLAG(spec[sq_rebirth],White) && has_pawn_reached_promotion_square(White,sq_rebirth))
        || (TSTFLAG(spec[sq_rebirth],Black) && has_pawn_reached_promotion_square(Black,sq_rebirth)))
      current_promotion_of_capturee[nbply] = getprompiece[Empty];
    else
      current_promotion_of_capturee[nbply] = Empty;
  }

  if (current_promotion_of_capturee[nbply]==Empty)
    result = defend(slices[si].next1,n);
  else
  {
    place_promotee();

    result = defend(slices[si].next1,n);

    if (!post_move_iteration_locked[nbply])
    {
      current_promotion_of_capturee[nbply] = getprompiece[current_promotion_of_capturee[nbply]];
      TracePiece(current_promotion_of_capturee[nbply]);TraceText("\n");

      if (current_promotion_of_capturee[nbply]!=Empty)
        lock_post_move_iterations();
    }
  }

  prev_post_move_iteration_id[nbply] = post_move_iteration_id[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void instrument_rebirth(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STCirceRebirthPromoter);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_circe_promoters(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STCirceRebirthHandler,
                                           &instrument_rebirth);
  stip_structure_traversal_override_single(&st,
                                           STSuperCirceRebirthHandler,
                                           &instrument_rebirth);
  stip_structure_traversal_override_single(&st,
                                           STCirceParrainRebirthHandler,
                                           &instrument_rebirth);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
