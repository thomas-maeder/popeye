#include "conditions/nopromotion.h"
#include "conditions/circe/circe.h"
#include "pieces/walks/pawns/promotion.h"
#include "solving/move_effect_journal.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "debugging/trace.h"
#include "conditions/conditions.h"

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type nopromotion_avoid_promotion_moving_solve(slice_index si,
                                                          stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
    PieceIdType const moving_id = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
    square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                              moving_id,
                                                                              sq_arrival);
    if (is_square_occupied_by_promotable_pawn(pos)==no_side)
      result = solve(slices[si].next1,n);
    else
      result = previous_move_is_illegal;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type nopromotion_avoid_promotion_reborn_solve(slice_index si,
                                                          stip_length_type n)
{
  stip_length_type result;
  square const sq_rebirth = circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square;
  Side promotion_for_side;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  promotion_for_side = is_square_occupied_by_promotable_pawn(sq_rebirth);

  if ((promotion_for_side==White && CondFlag[nowhiteprom])
      || (promotion_for_side==Black && CondFlag[noblackprom]))
    result = previous_move_is_illegal;
  else
    result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void substitute_avoid_promotion_moving(slice_index si,
                                              stip_structure_traversal *st)
{
  boolean const (* const enabled)[nr_sides] = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if ((*enabled)[slices[si].starter])
    pipe_substitute(si,alloc_pipe(STNoPromotionsRemovePromotionMoving));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void substitute_avoid_promotion_reborn(slice_index si,
                                              stip_structure_traversal *st)
{
  boolean const (* const enabled)[nr_sides] = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if ((*enabled)[slices[si].starter])
    pipe_substitute(si,alloc_pipe(STNoPromotionsRemovePromotionMoving));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solvers with Patrol Chess
 * @param si identifies the root slice of the stipulation
 */
void stip_insert_nopromotions(slice_index si)
{
  stip_structure_traversal st;
  boolean enabled[nr_sides] =
  {
      CondFlag[nowhiteprom],
      CondFlag[noblackprom]
  };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_impose_starter(si,slices[si].starter);

  stip_structure_traversal_init(&st,&enabled);
  stip_structure_traversal_override_single(&st,STMovingPawnPromoter,&substitute_avoid_promotion_moving);
  stip_structure_traversal_override_single(&st,STCirceRebirthPromoter,&substitute_avoid_promotion_reborn);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
