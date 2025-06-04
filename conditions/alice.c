#include "conditions/alice.h"
#include "stipulation/move.h"
#include "solving/pipe.h"
#include "solving/move_effect_journal.h"
#include "position/effects/flags_change.h"
#include "position/position.h"

#include "debugging/assert.h"
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
void alice_change_board_solve(slice_index si)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = top+move_effect_journal_index_offset_movement;
  square const pos = move_effect_journal[movement].u.piece_movement.to;
  Flags flags = being_solved.spec[pos];
  Flags const flag_new = TSTFLAG(flags,AliceBoardA) ? AliceBoardB : AliceBoardA;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(move_effect_journal[movement].type==move_effect_piece_movement);
  assert(TSTFLAG(flags,AliceBoardA)|TSTFLAG(flags,AliceBoardB));

  CLRFLAG(flags,AliceBoardA);
  CLRFLAG(flags,AliceBoardB);
  SETFLAG(flags,flag_new);
  move_effect_journal_do_flags_change(move_effect_reason_alice,pos,flags);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument slices with move tracers
 */
void solving_insert_alice(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STAliceBoardChanger);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
