#include "conditions/marscirce/anti.h"
#include "conditions/marscirce/marscirce.h"
#include "solving/move_generator.h"
#include "solving/move_effect_journal.h"
#include "stipulation/stipulation.h"
#include "solving/has_solution_type.h"
#include "solving/pipe.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/move.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

circe_variant_type antimars_variant;

/* Prevent Anti-Marscirce from generating null moves
 * @param si identifies the slice
 */
void anti_mars_circe_reject_null_moves(slice_index si)
{
  numecoup const base = CURRMOVE_OF_PLY(nbply);
  numecoup i;
  numecoup new_top = base;

  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  pipe_move_generation_delegate(si);

  for (i = base+1; i<=CURRMOVE_OF_PLY(nbply); ++i)
  {
    TraceSquare(move_generation_stack[i].departure);
    TraceSquare(move_generation_stack[i].arrival);

    if (move_generation_stack[i].arrival==move_generation_stack[i].departure)
    {
      TraceText("rejecting");
    }
    else
    {
      TraceText("accepting");
      ++new_top;
      move_generation_stack[new_top] = move_generation_stack[i];
    }

    TraceEOL();
  }

  SET_CURRMOVE(nbply,new_top);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

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
    slice_insertion_insert_contextually(si,st->context,prototypes,nr_prototypes);
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
        alloc_pipe(STAntiMarsCirceRejectNullMoves),
        alloc_pipe(STMarsCirceFixDeparture),
        alloc_pipe(STMarsCirceGenerateFromRebirthSquare),
        alloc_pipe(STMarsCirceRememberRebirth),
        alloc_pipe(STMoveGeneratorRejectCaptures)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert_contextually(si,st->context,prototypes,nr_prototypes);
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
                           &slice_insertion_insert_contextually,
                           STAntimarsCirceConsideringRebirth);
  circe_instrument_solving(si,
                           STAntimarsCirceConsideringRebirth,
                           STCirceDeterminedRebirth,
                           alloc_pipe(STMarscirceRemoveCapturer));

  if (antimars_variant.rebirth_reason==move_effect_reason_rebirth_choice)
    circe_instrument_solving(si,
                             STAntimarsCirceConsideringRebirth,
                             STAntimarsCirceConsideringRebirth,
                             alloc_pipe(STMoveGenerationPostMoveIterator));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
