#include "options/no_short_variations/no_short_variations.h"
#include "options/no_short_variations/no_short_variations_attacker_filter.h"
#include "stipulation/battle_play/branch.h"
#include "pypipe.h"
#include "trace.h"

#include <assert.h>

static void append_no_short_variations(slice_index si,
                                       stip_structure_traversal *st)
{
  boolean const * const nested = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_no_short_variations_slice();
    battle_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation with STNo_short_variationsSeriesFilter slices
 * @param si identifies slice where to start
 */
void stip_insert_no_short_variations_filters(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STDefenseMovePlayed,
                                           append_no_short_variations);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
