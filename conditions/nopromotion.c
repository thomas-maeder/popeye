#include "conditions/nopromotion.h"
#include "conditions/conditions.h"
#include "solving/move_effect_journal.h"
#include "pieces/walks/pawns/promotee_sequence.h"
#include "position/effects/utils.h"
#include "solving/pipe.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void nopromotion_avoid_promotion_moving_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
    PieceIdType const moving_id = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
    square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                              moving_id,
                                                                              sq_arrival);
    pipe_this_move_illegal_if(si,is_square_occupied_by_promotable_pawn(pos,trait[nbply]));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void substitute_avoid_promotion_moving(slice_index si,
                                              stip_structure_traversal *st)
{
  boolean const (* const enabled)[nr_sides] = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if ((*enabled)[SLICE_STARTER(si)])
    pipe_substitute(si,alloc_pipe(STNoPromotionsRemovePromotionMoving));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solvers with Patrol Chess
 * @param si identifies the root slice of the stipulation
 */
void solving_insert_nopromotions(slice_index si)
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

  solving_impose_starter(si,SLICE_STARTER(si));

  stip_structure_traversal_init(&st,&enabled);
  stip_structure_traversal_override_single(&st,STPawnPromoter,&substitute_avoid_promotion_moving);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
