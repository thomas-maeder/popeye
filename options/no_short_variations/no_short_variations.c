#include "options/no_short_variations/no_short_variations.h"
#include "options/no_short_variations/no_short_variations_attacker_filter.h"
#include "pypipe.h"
#include "trace.h"

#include <assert.h>

static void append_no_short_variations(slice_index si,
                                       stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_append(si,alloc_no_short_variations_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors no_short_variations_filter_inserters[] =
{
  { STDefenseMoveLegalityChecked, &append_no_short_variations  }
};

enum
{
  nr_no_short_variations_filter_inserters =
  (sizeof no_short_variations_filter_inserters
   / sizeof no_short_variations_filter_inserters[0])
};

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
  stip_structure_traversal_override(&st,
                                    no_short_variations_filter_inserters,
                                    nr_no_short_variations_filter_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
