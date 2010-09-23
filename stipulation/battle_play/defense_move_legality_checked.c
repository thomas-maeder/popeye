#include "stipulation/battle_play/defense_move_legality_checked.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STDefenseMoveLegalityChecked defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index
alloc_defense_move_legality_checked_slice(stip_length_type length,
                                          stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STDefenseMoveLegalityChecked,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Shorten the branches by the move that is represented by the root
 * slices
 */
static void battle_branch_post_root_shorten_end(slice_index si,
                                                stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  battle_branch_shorten_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Shorten the branches by the move that is represented by the root
 * slices
 */
static void battle_branch_post_root_shorten(slice_index si,
                                            stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);
  battle_branch_shorten_slice(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors post_root_shorteners[] =
{
  { STAttackMove,                   &battle_branch_post_root_shorten     },
  { STAttackFindShortest,           &battle_branch_post_root_shorten     },
  { STDefenseMove,                  &battle_branch_post_root_shorten     },
  { STSelfCheckGuardAttackerFilter, &battle_branch_post_root_shorten     },
  { STSelfCheckGuardDefenderFilter, &battle_branch_post_root_shorten     },
  { STSelfDefense,                  &battle_branch_post_root_shorten     },
  { STReflexAttackerFilter,         &battle_branch_post_root_shorten     },
  { STReflexDefenderFilter,         &battle_branch_post_root_shorten     },
  { STDefenseMoveShoeHorningDone,   &battle_branch_post_root_shorten_end },
  { STDefenseMoveLegalityChecked,   &battle_branch_post_root_shorten     },
  { STDefenseMoveFiltered,          &battle_branch_post_root_shorten     },
  { STDefenseDealtWith,             &battle_branch_post_root_shorten     },
  { STReadyForAttack,               &battle_branch_post_root_shorten     },
  { STAttackFork,                   &battle_branch_post_root_shorten     },
  { STAttackMovePlayed,             &battle_branch_post_root_shorten     },
  { STAttackMoveShoeHorningDone,    &battle_branch_post_root_shorten     },
  { STAttackMoveLegalityChecked,    &battle_branch_post_root_shorten     },
  { STAttackMoveFiltered,           &battle_branch_post_root_shorten     },
  { STAttackDealtWith,              &battle_branch_post_root_shorten     },
  { STReadyForDefense,              &battle_branch_post_root_shorten     },
  { STDefenseMovePlayed,            &battle_branch_post_root_shorten     },
  { STDefenseFork,                  &battle_branch_post_root_shorten     }
};

enum
{
  nr_post_root_shorteners = (sizeof post_root_shorteners
                             / sizeof post_root_shorteners[0])
};

/* Create the root slices sequence for a battle play branch; shorten
 * the non-root slices by the moves represented by the root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void defense_move_legality_checked_make_root(slice_index si,
                                             stip_structure_traversal *st)
{
  root_insertion_state_type * const state = st->param;
  slice_index copy;
  stip_structure_traversal st_nested;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  copy = copy_slice(si);
  pipe_link(copy,state->result);
  state->result = copy;

  stip_structure_traversal_init(&st_nested,0);
  stip_structure_traversal_override(&st_nested,
                                    post_root_shorteners,
                                    nr_post_root_shorteners);
  stip_traverse_structure(si,&st_nested);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
