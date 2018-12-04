#include "conditions/volage.h"
#include "position/effects/flags_change.h"
#include "position/effects/side_change.h"
#include "position/effects/piece_movement.h"
#include "stipulation/pipe.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"
#include "conditions/conditions.h"

#include "debugging/assert.h"

static void change_side(void)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  square const sq_departure = move_effect_journal_get_departure_square(nbply);
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
  Flags const movingspec = move_effect_journal[movement].u.piece_movement.movingspec;
  PieceIdType const moving_id = GetPieceId(movingspec);
  square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                            moving_id,
                                                                            sq_arrival);
  Side const to_side = advers(trait[nbply]);

  if (TSTFLAG(movingspec,Volage)
      && SquareCol(sq_departure)!=SquareCol(pos)
      && !TSTFLAG(being_solved.spec[pos],to_side))
  {
    move_effect_journal_do_side_change(move_effect_reason_volage_side_change,
                                       pos);

    if (!CondFlag[hypervolage])
    {
      Flags flags = being_solved.spec[pos];
      CLRFLAG(flags,Volage);
      move_effect_journal_do_flags_change(move_effect_reason_volage_side_change,
                                          pos,
                                          flags);
    }
  }
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
void volage_side_changer_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  change_side();
  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument slices with move tracers
 */
void solving_insert_volage_side_changers(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STVolageSideChanger);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
