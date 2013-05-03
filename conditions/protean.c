#include "conditions/protean.h"
#include "pydata.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

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
stip_length_type protean_pawn_adjuster_solve(slice_index si,
                                              stip_length_type n)
{
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;
  move_effect_journal_index_type const top = move_effect_journal_top[nbply-1];
  move_effect_journal_index_type const capture = top+move_effect_journal_index_offset_capture;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (move_effect_journal[capture].type==move_effect_piece_removal
      && (!rex_protean_ex || !TSTFLAG(spec[sq_arrival],Royal)))
  {
    move_effect_journal_index_type const movement = top+move_effect_journal_index_offset_movement;
    piece const pi_captured = move_effect_journal[capture].u.piece_removal.removed;
    Flags const spec_moving = move_effect_journal[movement].u.piece_movement.movingspec;
    piece substitute = -pi_captured;
    if (TSTFLAG(spec_moving,Black))
    {
      if (substitute==pn)
        substitute = reversepn;
      else if (substitute==reversepn)
        substitute = pn;
    }
    else
    {
      if (substitute==pb)
        substitute = reversepb;
      else if (substitute==reversepb)
        substitute = pb;
    }

    move_effect_journal_do_piece_change(move_effect_reason_protean_adjustment,
                                        sq_arrival,
                                        substitute);
  }

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with move tracers
 */
void stip_insert_protean_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STProteanPawnAdjuster);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
