#include "conditions/protean.h"
#include "pieces/pieces.h"
#include "position/position.h"
#include "stipulation/pipe.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "position/effects/walk_change.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

boolean protean_is_rex_inclusive;

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
void protean_walk_adjuster_solve(slice_index si)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (move_effect_journal[capture].type==move_effect_piece_removal
      && (protean_is_rex_inclusive || !TSTFLAG(being_solved.spec[sq_arrival],Royal)))
  {
    piece_walk_type substitute = move_effect_journal[capture].u.piece_removal.walk;
    if (substitute==Pawn)
      substitute = ReversePawn;
    else if (substitute==ReversePawn)
      substitute = Pawn;

    move_effect_journal_do_walk_change(move_effect_reason_protean_adjustment,
                                       sq_arrival,
                                       substitute);
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument slices with move tracers
 */
void solving_insert_protean_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STProteanWalkAdjuster);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
