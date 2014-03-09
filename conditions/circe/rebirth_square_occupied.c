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
void circe_no_rebirth_on_occupied_square(slice_index si,
                                         slice_type hook_type,
                                         slice_type nonempty_proxy_type,
                                         slice_type joint_type)
{
  circe_insert_rebirth_avoider(si,
                               hook_type,
                               STCirceTestRebirthSquareEmpty,
                               nonempty_proxy_type,
                               joint_type);
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
void circe_stop_rebirth_on_occupied_square(slice_index si,
                                           slice_type hook_type,
                                           slice_type nonempty_proxy_type,
                                           slice_type joint_type)
{
  stip_structure_traversal st;

  circe_insert_rebirth_avoider(si,
                               hook_type,
                               STCirceTestRebirthSquareEmpty,
                               nonempty_proxy_type,
                               joint_type);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           nonempty_proxy_type,
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
void circe_assassinate_on_occupied_square(slice_index si,
                                          slice_type hook_type,
                                          slice_type nonempty_proxy_type,
                                          slice_type joint_type)
{
  stip_structure_traversal st;

  circe_insert_rebirth_avoider(si,
                               hook_type,
                               STCirceTestRebirthSquareEmpty,
                               nonempty_proxy_type,
                               joint_type);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           nonempty_proxy_type,
                                           &append_assassin);
  stip_traverse_structure(si,&st);
}

static void append_parachuter(slice_index si, stip_structure_traversal*st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype =  alloc_pipe(STCirceParachuteRemember);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Cause moves with Circe rebirth on an occupied square to parachute
 * @param si entry slice into the solving machinery
 */
void circe_parachute_onto_occupied_square(slice_index si,
                                          slice_type hook_type,
                                          slice_type nonempty_proxy_type,
                                          slice_type joint_type)
{
  stip_structure_traversal st;

  circe_insert_rebirth_avoider(si,
                               hook_type,
                               STCirceTestRebirthSquareEmpty,
                               nonempty_proxy_type,
                               joint_type);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           nonempty_proxy_type,
                                           &append_parachuter);
  stip_traverse_structure(si,&st);
}

static void append_volcanic(slice_index si, stip_structure_traversal*st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype =  alloc_pipe(STCirceVolcanicRemember);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Cause moves with Circe rebirth on an occupied square to volcanic
 * @param si entry slice into the solving machinery
 */
void circe_volcanic_under_occupied_square(slice_index si,
                                          slice_type hook_type,
                                          slice_type nonempty_proxy_type,
                                          slice_type joint_type)
{
  stip_structure_traversal st;

  circe_insert_rebirth_avoider(si,
                               hook_type,
                               STCirceTestRebirthSquareEmpty,
                               nonempty_proxy_type,
                               joint_type);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           nonempty_proxy_type,
                                           &append_volcanic);
  stip_traverse_structure(si,&st);
}
