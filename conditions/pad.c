#include "conditions/pad.h"
#include "stipulation/pipe.h"
#include "solving/has_solution_type.h"
#include "solving/observation.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "position/effects/piece_removal.h"
#include "position/effects/utils.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

boolean pad_is_rex_inclusive;

static boolean has_piece_captured[MaxPieceId+1];

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
void pad_bookkeeper_solve(slice_index si)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (move_effect_journal[capture].type==move_effect_piece_removal)
  {
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    Flags const moving_spec = move_effect_journal[movement].u.piece_movement.movingspec;
    PieceIdType const moving_id = GetPieceId(moving_spec);

    if (TSTFLAG(moving_spec,Royal))
    {
      if (pad_is_rex_inclusive)
      {
        if (has_piece_captured[moving_id])
          solve_result = this_move_is_illegal;
        else
        {
          has_piece_captured[moving_id] = true;
          pipe_solve_delegate(si);
          has_piece_captured[moving_id] = false;
        }
      }
      else
        pipe_solve_delegate(si);
    }
    else
    {
      if (has_piece_captured[moving_id])
      {
        square const to = move_effect_journal[movement].u.piece_movement.to;
        PieceIdType const moving_id = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
        square const final_destination_of_moving = move_effect_journal_follow_piece_through_other_effects(nbply,moving_id,to);

        move_effect_journal_do_piece_removal(move_effect_reason_pad,final_destination_of_moving);
        pipe_solve_delegate(si);
      }
      else
      {
        has_piece_captured[moving_id] = true;
        pipe_solve_delegate(si);
        has_piece_captured[moving_id] = false;
      }
    }
  }
  else
    pipe_solve_delegate(si);


  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument slices with move tracers
 */
void solving_insert_pad(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STPADBookKeeper);

  if (pad_is_rex_inclusive)
    stip_instrument_check_validation(si,
                                     nr_sides,
                                     STValidateCheckMoveByPlayingCapture);

  {
    PieceIdType p;
    for (p = MinPieceId; p<=MaxPieceId; ++p)
      has_piece_captured[p] = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
