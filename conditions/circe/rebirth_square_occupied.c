#include "conditions/circe/rebirth_square_occupied.h"
#include "conditions/circe/rebirth_avoider.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "debugging/trace.h"

/* Cause moves with Circe rebirth on an occupied square to be played without
 * rebirth
 * @param si entry slice into the solving machinery
 */
void circe_no_rebirth_on_occupied_square(slice_index si)
{
  circe_insert_rebirth_avoider(si,
                               STCirceDeterminingRebornPiece,
                               STCirceTestRebirthSquareEmpty,
                               STCirceRebirthOnNonEmptySquare,
                               STLandingAfterCirceRebirthHandler);
}

static void append_stop(slice_index si, stip_structure_traversal*st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_pipe(STSupercircePreventRebirthOnNonEmptySquare);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Cause moves with Circe rebirth on an occupied square to not be played
 * @param si entry slice into the solving machinery
 */
void circe_stop_rebirth_on_occupied_square(slice_index si)
{
  stip_structure_traversal st;

  circe_insert_rebirth_avoider(si,
                               STCirceDeterminingRebornPiece,
                               STCirceTestRebirthSquareEmpty,
                               STCirceRebirthOnNonEmptySquare,
                               STLandingAfterCirceRebirthHandler);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STCirceRebirthOnNonEmptySquare,
                                           &append_stop);
  stip_traverse_structure(si,&st);
}

static void append_assassin(slice_index si, stip_structure_traversal*st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype =  alloc_pipe(STCirceAssassinAssassinate);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Cause moves with Circe rebirth on an occupied square to assassinate
 * @param si entry slice into the solving machinery
 */
void circe_assassinate_on_occupied_square(slice_index si)
{
  stip_structure_traversal st;

  circe_insert_rebirth_avoider(si,
                               STCirceDeterminingRebornPiece,
                               STCirceTestRebirthSquareEmpty,
                               STCirceRebirthOnNonEmptySquare,
                               STCircePlacingReborn);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STCirceRebirthOnNonEmptySquare,
                                           &append_assassin);
  stip_traverse_structure(si,&st);
}
