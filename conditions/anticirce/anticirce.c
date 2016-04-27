#include "conditions/anticirce/anticirce.h"
#include "conditions/anticirce/cheylan.h"
#include "solving/observation.h"
#include "solving/move_generator.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/temporary_hacks.h"
#include "pieces/pieces.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

static circe_variant_type const circe_variant_default = {
    .is_rex_inclusive = true,
    .on_occupied_rebirth_square_default = circe_on_occupied_rebirth_square_strict,
    .do_place_reborn = true,
    .default_relevant_piece = circe_relevant_piece_capturer,
    .actual_relevant_piece = circe_relevant_piece_capturer,
    .rebirth_reason = move_effect_reason_rebirth_no_choice,
    .anticirce_type = anticirce_type_count
};

circe_variant_type anticirce_variant;

/* Reset a circe_variant object to the default values
 * @param variant address of the variant object to be reset
 */
void anticirce_reset_variant(circe_variant_type *variant)
{
  *variant = circe_variant_default;
}

/* Instrument the solving machinery with AntiCirce
 * @param si identifies the root slice of the solving machinery
 */
void anticirce_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  circe_initialise_solving(si,&anticirce_variant,STMove,&move_insert_slices,STAnticirceConsideringRebirth);

  if (anticirce_variant.do_place_reborn)
  {
    circe_instrument_solving(si,
                             STAnticirceConsideringRebirth,
                             STCirceDeterminedRebirth,
                             alloc_pipe(STAnticirceRemoveCapturer));
    if (anticirce_variant.anticirce_type==anticirce_type_cheylan)
      anticirce_cheylan_initialise_solving(si);

    if (anticirce_variant.determine_rebirth_square!=circe_determine_rebirth_square_super)
      stip_instrument_check_validation(si,
                                       nr_sides,
                                       STValidateCheckMoveByPlayingCapture);
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
void anticirce_remove_capturer_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];
    square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                              GetPieceId(context->relevant_spec),
                                                                              context->rebirth_from);
    move_effect_journal_do_piece_removal(move_effect_reason_transfer_no_choice,
                                         pos);
  }

  pipe_dispatch_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
