#include "conditions/degradierung.h"
#include "pieces/walks/classification.h"
#include "stipulation/stipulation.h"
#include "solving/has_solution_type.h"
#include "stipulation/branch.h"
#include "stipulation/move.h"
#include "position/walk_change.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include "debugging/assert.h"

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
void degradierung_degrader_solve(slice_index si)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
  piece_walk_type const pi_played = move_effect_journal[movement].u.piece_movement.moving;
  SquareFlags const double_step = SLICE_STARTER(si)==White ? WhPawnDoublestepSq : BlPawnDoublestepSq;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(pi_played!=Empty);

  if (!is_pawn(pi_played)
      && !TSTFLAG(move_effect_journal[movement].u.piece_movement.movingspec,Royal)
      && TSTFLAG(sq_spec[sq_arrival],double_step))
    move_effect_journal_do_walk_change(move_effect_reason_degradierung,
                                        sq_arrival,
                                        Pawn);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void solving_insert_degradierung(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STDegradierungDegrader);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
