#include "conditions/koeko/contact_grid.h"
#include "position/position.h"
#include "conditions/circe/circe.h"
#include "conditions/grid.h"
#include "position/effects/utils.h"
#include "solving/observation.h"
#include "solving/pipe.h"
#include "stipulation/pipe.h"
#include "stipulation/move.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

static boolean nogridcontact(square sq_arrival)
{
  vec_index_type k;

  for (k = vec_queen_end; k>=vec_queen_start; k--)
  {
    square const sq_candidate = sq_arrival+vec[k];
    if (!is_square_empty(sq_candidate) && !is_square_blocked(sq_candidate)
        && GridLegal(sq_candidate,sq_arrival))
      return false;
  }

  return true;
}

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
void contact_grid_legality_tester_solve(slice_index si)
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
    pipe_this_move_illegal_if(si,nogridcontact(pos));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

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
void contact_grid_avoid_circe_rebirth(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (nogridcontact(circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square))
    circe_rebirth_context_stack[circe_rebirth_context_stack_pointer].rebirth_square = initsquare;

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise solving in Contact Grid Chess
 * @param si identifies root slice of stipulation
 */
void contact_grid_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  circe_instrument_solving(si,
                           STCirceConsideringRebirth,
                           STCirceDeterminedRebirth,
                           alloc_pipe(STContactGridAvoidCirceRebirth));

  stip_instrument_moves(si,STGridContactLegalityTester);

  observation_play_move_to_validate(si,nr_sides);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
