#include "conditions/degradierung.h"
#include "pieces/walks/classification.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/branch.h"
#include "stipulation/move.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"
#include "pydata.h"

#include <assert.h>

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
stip_length_type degradierung_degrader_solve(slice_index si,
                                              stip_length_type n)
{
  stip_length_type result;
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
  PieNam const pi_played = move_effect_journal[movement].u.piece_movement.moving;
  SquareFlags const double_step = slices[si].starter==White ? WhPawnDoublestepSq : BlPawnDoublestepSq;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(pi_played!=Empty);

  if (!is_pawn(pi_played)
      && !TSTFLAG(move_effect_journal[movement].u.piece_movement.movingspec,Royal)
      && TSTFLAG(sq_spec[sq_arrival],double_step))
    move_effect_journal_do_piece_change(move_effect_reason_degradierung,
                                        sq_arrival,
                                        Pawn);

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_degradierung(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STDegradierungDegrader);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
