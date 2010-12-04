#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "pypipe.h"
#include "trace.h"

#include <assert.h>

/* Order in which the slice types at the root appear
 */
static slice_index const root_slice_rank_order[] =
{
  STProxy,
  STSetplayFork,
  STMoveInverterRootSolvableFilter,
  STMoveInverterSolvableFilter,
  STReflexAttackerFilter,
  STReflexDefenderFilter,
  STReadyForAttack,
  STReadyForDefense,
  STReadyForHelpMove,
  STReadyForSeriesMove,
  STAmuMateFilter,
  STUltraschachzwangGoalFilter,
  STCirceSteingewinnFilter,
  STSelfCheckGuard,
  STAttackMoveLegalityChecked,
  STDefenseMoveLegalityChecked,
  STHelpMoveLegalityChecked,
  STSeriesMoveLegalityChecked
};

enum
{
  nr_root_slice_rank_order_elmts = (sizeof root_slice_rank_order
                                    / sizeof root_slice_rank_order[0])
};

/* Determine the rank of a slice type
 * @param type defense slice type
 * @return rank of type; nr_defense_slice_rank_order_elmts if the rank can't
 *         be determined
 */
static unsigned int get_root_slice_rank(SliceType type)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(SliceType,type,"");
  TraceFunctionParamListEnd();

  for (result = 0; result!=nr_root_slice_rank_order_elmts; ++result)
    if (root_slice_rank_order[result]==type)
      break;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine the position where to insert a slice from the root.
 * @param si entry slice of defense branch
 * @param type type of slice to be inserted
 * @return identifier of slice before which to insert; no_slice if no
 *         suitable position could be found
 */
slice_index find_root_slice_insertion_pos(slice_index si, SliceType type)
{
  slice_index result = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(SliceType,type,"");
  TraceFunctionParamListEnd();

  {
    unsigned int const rank_type = get_root_slice_rank(type);
    assert(rank_type!=nr_root_slice_rank_order_elmts);
    while (true)
    {
      unsigned int const rank = get_root_slice_rank(slices[si].type);
      if (rank==nr_root_slice_rank_order_elmts)
        break;
      else if (rank>rank_type)
      {
        result = si;
        break;
      }
      else
        si = slices[si].u.pipe.next;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Order in which the slice types dealing with goals appear
 */
static slice_index const leaf_slice_rank_order[] =
{
  STProxy,
  STGoalMateReachedTester,
  STGoalStalemateReachedTester,
  STGoalDoubleStalemateReachedTester,
  STGoalTargetReachedTester,
  STGoalCaptureReachedTester,
  STGoalSteingewinnReachedTester,
  STGoalEnpassantReachedTester,
  STGoalDoubleMateReachedTester,
  STGoalCounterMateReachedTester,
  STGoalCastlingReachedTester,
  STGoalAutoStalemateReachedTester,
  STGoalCircuitReachedTester,
  STGoalExchangeReachedTester,
  STGoalCircuitByRebirthReachedTester,
  STGoalExchangeByRebirthReachedTester,
  STGoalAnyReachedTester,
  STGoalProofgameReachedTester,
  STGoalAToBReachedTester,
  STGoalMateOrStalemateReachedTester,
  STGoalCheckReachedTester,
  STSelfCheckGuard,
  STGoalNotCheckReachedTester,
  STGoalImmobileReachedTester,
  STGoalReachedTested,
  STDefenseMoveLegalityChecked,
  STHelpMoveLegalityChecked,
  STSeriesMoveLegalityChecked,
  STLeaf
};

enum
{
  nr_leaf_slice_rank_order_elmts = (sizeof leaf_slice_rank_order
                                    / sizeof leaf_slice_rank_order[0])
};

/* Determine the rank of a slice type
 * @param type defense slice type
 * @return rank of type; nr_defense_slice_rank_order_elmts if the rank can't
 *         be determined
 */
static unsigned int get_leaf_slice_rank(SliceType type)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(SliceType,type,"");
  TraceFunctionParamListEnd();

  for (result = 0; result!=nr_leaf_slice_rank_order_elmts; ++result)
    if (leaf_slice_rank_order[result]==type)
      break;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine the position where to insert a slice into an defense branch.
 * @param si entry slice of defense branch
 * @param type type of slice to be inserted
 * @return identifier of slice before which to insert; no_slice if no
 *         suitable position could be found
 */
slice_index find_leaf_slice_insertion_pos(slice_index si, SliceType type)
{
  slice_index result = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(SliceType,type,"");
  TraceFunctionParamListEnd();

  {
    unsigned int const rank_type = get_leaf_slice_rank(type);
    assert(rank_type!=nr_leaf_slice_rank_order_elmts);
    while (true)
    {
      unsigned int const rank = get_leaf_slice_rank(slices[si].type);
      if (rank==nr_leaf_slice_rank_order_elmts)
        break;
      else if (rank>rank_type)
      {
        result = si;
        break;
      }
      else
        si = slices[si].u.pipe.next;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a new branch slice
 * @param type which slice type
 * @param length maximum number of half moves until end of slice
 * @param min_length minimum number of half moves until end of slice
 * @return newly allocated slice
 */
slice_index alloc_branch(SliceType type,
                         stip_length_type length,
                         stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(SliceType,type,"");
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_pipe(type);
  slices[result].u.branch.length = length;
  slices[result].u.branch.min_length = min_length;
  slices[result].u.branch.imminent_goal.type = no_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find the next slice with a specific type in a branch
 * @param type type of slice to be found
 * @param si identifies the slice where to start searching
 * @return identifier for slice with type type; no_slice if none is found
 */
slice_index branch_find_slice(SliceType type, slice_index si)
{
  slice_index result = si;
  boolean slices_visited[max_nr_slices] = { false };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  do
  {
    TraceValue("%u\n",slices[result].u.pipe.next);
    result = slices[result].u.pipe.next;
    if (result==no_slice || slices[result].type==type)
      break;
    else if (slices_visited[result]
             || !slice_has_structure(result,slice_structure_pipe))
    {
      result = no_slice;
      break;
    }
    else
      slices_visited[result] = true;
  } while (true);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Initialise the full_length and remaing fields of a
 * stip_moves_traversal struct from a branch slice if necessary
 * @param si identifies the branch slice
 * @param st refers to the struct to be initialised
 */
void stip_traverse_moves_branch_init_full_length(slice_index si,
                                                 stip_moves_traversal *st)
{
  if (st->remaining==0)
  {
    st->full_length = slices[si].u.branch.length;
    TraceValue("->%u\n",st->full_length);
    st->remaining = slices[si].u.branch.length;
  }
}

/* Traversal of the moves of some branch slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_branch_slice(slice_index si, stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_branch_init_full_length(si,st);

  stip_traverse_moves_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traversal of the moves of some branch slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_move_slice(slice_index si, stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_branch_init_full_length(si,st);

  --st->remaining;
  TraceValue("->%u\n",st->remaining);
  stip_traverse_moves_pipe(si,st);
  ++st->remaining;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traversal of the moves of a branch
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_branch(slice_index si, stip_moves_traversal *st)
{
  stip_length_type const save_remaining = st->remaining;
  stip_length_type const save_full_length = st->full_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  ++st->level;
  st->remaining = 0;

  stip_traverse_moves(si,st);

  st->full_length = save_full_length;
  st->remaining = save_remaining;
  TraceFunctionParam("->%u\n",st->remaining);
  --st->level;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Link a pipe slice to the entry slice of a branch
 * @param pipe identifies the pipe slice
 * @param entry identifies the entry slice of the branch
 */
void link_to_branch(slice_index pipe, slice_index entry)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",pipe);
  TraceFunctionParam("%u",entry);
  TraceFunctionParamListEnd();

  if (slices[entry].prev==no_slice)
    pipe_link(pipe,entry);
  else
    pipe_set_successor(pipe,entry);


  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
