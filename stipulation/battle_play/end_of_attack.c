#include "stipulation/battle_play/end_of_attack.h"
#include "pypipe.h"
#include "pyreflxg.h"
#include "stipulation/battle_play/ready_for_defense.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STEndOfAttack slice.
 * @return index of allocated slice
 */
slice_index alloc_end_of_attack_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STEndOfAttack);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static structure_traversers_visitors to_postkey_play_makers[] =
{
  { STReflexDefenderFilter, &reflex_defender_filter_reduce_to_postkey_play },
  { STReadyForDefense,      &ready_for_defense_reduce_to_postkey_play      }
};

enum
{
  nr_to_postkey_play_makers = (sizeof to_postkey_play_makers
                               / sizeof to_postkey_play_makers[0])
};

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void end_of_attack_apply_postkeyplay(slice_index si,
                                     stip_structure_traversal *st)
{
  slice_index * const postkey_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st_nested;

    stip_structure_traversal_init(&st_nested,postkey_slice);
    stip_structure_traversal_override(&st_nested,
                                      to_postkey_play_makers,
                                      nr_to_postkey_play_makers);
    stip_traverse_structure(si,&st_nested);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
