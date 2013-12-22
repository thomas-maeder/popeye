#include "conditions/bicolores.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"
#include "stipulation/temporary_hacks.h"

static void remove_enforce_side(slice_index si, stip_structure_traversal *st)
{
  pipe_remove(si);
}

/* Instrument the solving machinery with Bicolores
 * @param si root slice of the solving machinery
 */
void bicolores_initalise_solving(slice_index si)
{
  stip_structure_traversal st;

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STEnforceObserverSide,
                                           &remove_enforce_side);
  stip_traverse_structure(slices[temporary_hack_check_validator[White]].next2,&st);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STEnforceObserverSide,
                                           &remove_enforce_side);
  stip_traverse_structure(slices[temporary_hack_check_validator[Black]].next2,&st);
}
