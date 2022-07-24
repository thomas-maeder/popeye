#include "conditions/circe/kamikaze.h"
#include "conditions/circe/rebirth_avoider.h"
#include "conditions/anticirce/anticirce.h"
#include "solving/move_effect_journal.h"
#include "stipulation/pipe.h"
#include "stipulation/fork.h"
#include "stipulation/move.h"
#include "solving/binary.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Instrument the solving machinery with Circe Kamikaze
 * @param si identifies the root slice of the solving machinery
 */
void circe_kamikaze_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  anticirce_variant.on_occupied_rebirth_square_default = circe_on_occupied_rebirth_square_relaxed;
  anticirce_variant.relevant_side_overrider = circe_variant.relevant_side_overrider;
  anticirce_variant.rebirth_square_adapter = circe_variant.rebirth_square_adapter;
  anticirce_variant.reborn_walk_adapter = circe_variant.reborn_walk_adapter;
  anticirce_variant.is_turncoat = circe_variant.is_turncoat;
  anticirce_variant.determine_rebirth_square = circe_variant.determine_rebirth_square;
  anticirce_variant.is_promotion_possible = circe_variant.is_promotion_possible;

  circe_initialise_solving(si,&anticirce_variant,STMove,&move_insert_slices,STAnticirceConsideringRebirth);
  circe_instrument_solving(si,
                           STAnticirceConsideringRebirth,
                           STCirceDeterminedRebirth,
                           alloc_pipe(STAnticirceRemoveCapturer));
  circe_insert_rebirth_avoider(si,
                               STAnticirceConsideringRebirth,
                               STAnticirceConsideringRebirth,
                               alloc_fork_slice(STCirceKamikazeCaptureFork,
                                                no_slice),
                               STCirceRebirthAvoided,
                               STCirceDoneWithRebirth);

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
void circe_kamikaze_capture_fork_solve(slice_index si)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  binary_solve_if_then_else(si,!TSTFLAG(move_effect_journal[movement].u.piece_movement.movingspec,Kamikaze));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
