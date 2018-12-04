#include "conditions/koeko/anti.h"
#include "solving/observation.h"
#include "solving/move_effect_journal.h"
#include "position/effects/utils.h"
#include "solving/has_solution_type.h"
#include "solving/pipe.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/temporary_hacks.h"
#include "debugging/trace.h"

nocontactfunc_t antikoeko_nocontact;

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
void antikoeko_legality_tester_solve(slice_index si)
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
    pipe_this_move_illegal_if(si,!(*antikoeko_nocontact)(pos));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise solving in Anti-Koeko
 * @param si identifies the root slice of the stipulation
 */
void antikoeko_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_instrument_moves(si,STAntiKoekoLegalityTester);

  observation_play_move_to_validate(si,nr_sides);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
