#include "conditions/marscirce/anti.h"
#include "conditions/marscirce/marscirce.h"
#include "solving/move_generator.h"
#include "solving/move_effect_journal.h"
#include "stipulation/stipulation.h"
#include "solving/has_solution_type.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/move.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

circe_variant_type antimars_variant;

static void instrument_no_rebirth(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototypes[] =
    {
        alloc_pipe(STMarsCirceRememberNoRebirth),
        alloc_pipe(STMoveGeneratorRejectNoncaptures)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    branch_insert_slices_contextual(si,st->context,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_rebirth(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototypes[] = {
        alloc_pipe(STMarsCirceFixDeparture),
        alloc_pipe(STMarsCirceGenerateFromRebirthSquare),
        alloc_pipe(STMarsCirceRememberRebirth),
        alloc_pipe(STMoveGeneratorRejectCaptures)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    branch_insert_slices_contextual(si,st->context,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise the solving machinery with Anti-Mars Circe
 * @param si identifies root slice of solving machinery
 */
void solving_initialise_antimars(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,
                                             STMoveForPieceGeneratorStandardPath,
                                             &instrument_rebirth);
    stip_structure_traversal_override_single(&st,
                                             STMoveForPieceGeneratorAlternativePath,
                                             &instrument_no_rebirth);
    stip_traverse_structure(si,&st);
  }

  circe_initialise_solving(si,
                           &antimars_variant,
                           STMoveForPieceGeneratorStandardPath,
                           &branch_insert_slices_contextual,
                           STMarsCirceConsideringRebirth);
  circe_instrument_solving(si,
                           STMarsCirceConsideringRebirth,
                           STCirceDeterminedRebirth,
                           alloc_pipe(STMarscirceRemoveCapturer));

  if (antimars_variant.rebirth_reason==move_effect_reason_rebirth_choice)
    circe_instrument_solving(si,
                             STMarsCirceConsideringRebirth,
                             STMarsCirceConsideringRebirth,
                             alloc_pipe(STMoveGenerationPostMoveIterator));

  stip_instrument_moves(si,STMarsCirceMoveToRebirthSquare);
  move_effect_journal_register_pre_capture_effect();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
