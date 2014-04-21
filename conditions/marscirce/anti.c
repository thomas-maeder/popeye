#include "conditions/marscirce/anti.h"
#include "conditions/marscirce/marscirce.h"
#include "solving/move_generator.h"
#include "solving/move_effect_journal.h"
#include "stipulation/stipulation.h"
#include "solving/has_solution_type.h"
#include "stipulation/move.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

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
                                             STGeneratingNoncapturesForPiece,
                                             &marscirce_instrument_movegenerator_rebirth);
    stip_structure_traversal_override_single(&st,
                                             STGeneratingCapturesForPiece,
                                             &marscirce_instrument_movegenerator_no_rebirth);
    stip_traverse_structure(si,&st);
  }

  stip_instrument_moves(si,STMarsCirceMoveToRebirthSquare);

  move_effect_journal_register_pre_capture_effect();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
