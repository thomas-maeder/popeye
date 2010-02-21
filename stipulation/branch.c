#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "pypipe.h"
#include "trace.h"

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
  slices[result].u.shortcut.length = length;
  slices[result].u.shortcut.min_length = min_length;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Continue deallocating a branch
 * @param si identifies branch_fork slice
 * @param st structure representing the traversal
 * @return true iff si and its children have been successfully
 *         deallocated
 */
static boolean traverse_and_deallocate(slice_index si, slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);
  dealloc_slice(si);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Continue deallocating a branch
 * @param si identifies branch_fork slice
 * @param st structure representing the traversal
 * @return true iff si and its children have been successfully
 *         deallocated
 */
static boolean traverse_and_deallocate_proxy(slice_index si, slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);
  dealloc_proxy_slice(si);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Store slice representing play after branch in object representing
 * traversal, then continue deallocating the branch
 * @param si identifies branch_fork slice
 * @param st structure representing the traversal
 */
static boolean traverse_and_deallocate_branch_fork(slice_index si,
                                                   slice_traversal *st)
{
  boolean const result = true;
  slice_index * const to_be_found = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *to_be_found = slices[si].u.branch_fork.towards_goal;

  traverse_slices(slices[si].u.pipe.next,st);
  dealloc_slice(si);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean traverse_and_deallocate_leaf(slice_index si,
                                            slice_traversal *st)
{
  boolean const result = true;
  slice_index * const to_be_found = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *to_be_found = si;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const slice_deallocators[] =
{
  &traverse_and_deallocate_proxy,       /* STProxy */
  &traverse_and_deallocate,             /* STAttackMove */
  &traverse_and_deallocate,             /* STBranchDirectDefender */
  &traverse_and_deallocate,             /* STHelpMove */
  &traverse_and_deallocate_branch_fork, /* STHelpFork */
  &traverse_and_deallocate,             /* STSeriesMove */
  &traverse_and_deallocate_branch_fork, /* STSeriesFork */
  &traverse_and_deallocate_leaf,        /* STLeafDirect */
  &traverse_and_deallocate_leaf,        /* STLeafHelp */
  &traverse_and_deallocate_leaf,        /* STLeafForced */
  &slice_traverse_children,             /* STReciprocal */
  &slice_traverse_children,             /* STQuodlibet */
  &slice_traverse_children,             /* STNot */
  &slice_traverse_children,             /* STMoveInverterRootSolvableFilter */
  &slice_traverse_children,             /* STMoveInverterSolvableFilter */
  &slice_traverse_children,             /* STMoveInverterSeriesFilter */
  &traverse_and_deallocate,             /* STAttackRoot */
  &traverse_and_deallocate,             /* STDefenseRoot */
  &traverse_and_deallocate,             /* STDirectHashed */
  &traverse_and_deallocate,             /* STHelpRoot */
  &traverse_and_deallocate,             /* STHelpShortcut */
  &traverse_and_deallocate,             /* STHelpHashed */
  &traverse_and_deallocate,             /* STSeriesRoot */
  &traverse_and_deallocate,             /* STSeriesShortcut */
  &traverse_and_deallocate,             /* STParryFork */
  &traverse_and_deallocate,             /* STSeriesHashed */
  &traverse_and_deallocate,             /* STSelfCheckGuardRootSolvableFilter */
  &traverse_and_deallocate,             /* STSelfCheckGuardSolvableFilter */
  &traverse_and_deallocate,             /* STSelfCheckGuardRootDefenderFilter */
  &traverse_and_deallocate,             /* STSelfCheckGuardAttackerFilter */
  &traverse_and_deallocate,             /* STSelfCheckGuardDefenderFilter */
  &traverse_and_deallocate,             /* STSelfCheckGuardHelpFilter */
  &traverse_and_deallocate,             /* STSelfCheckGuardSeriesFilter */
  &traverse_and_deallocate,             /* STDirectDefense */
  &slice_traverse_children,             /* STReflexHelpFilter */
  &slice_traverse_children,             /* STReflexSeriesFilter */
  &slice_traverse_children,             /* STReflexAttackerFilter */
  &slice_traverse_children,             /* STReflexDefenderFilter */
  &traverse_and_deallocate,             /* STSelfAttack */
  &traverse_and_deallocate,             /* STSelfDefense */
  &slice_traverse_children,             /* STRestartGuardRootDefenderFilter */
  &slice_traverse_children,             /* STRestartGuardHelpFilter */
  &slice_traverse_children,             /* STRestartGuardSeriesFilter */
  &slice_traverse_children,             /* STIntelligentHelpFilter */
  &slice_traverse_children,             /* STIntelligentSeriesFilter */
  &slice_traverse_children,             /* STGoalReachableGuardHelpFilter */
  &slice_traverse_children,             /* STGoalReachableGuardSeriesFilter */
  &traverse_and_deallocate,             /* STKeepMatingGuardRootDefenderFilter */
  &traverse_and_deallocate,             /* STKeepMatingGuardAttackerFilter */
  &traverse_and_deallocate,             /* STKeepMatingGuardDefenderFilter */
  &traverse_and_deallocate,             /* STKeepMatingGuardHelpFilter */
  &traverse_and_deallocate,             /* STKeepMatingGuardSeriesFilter */
  &traverse_and_deallocate,             /* STMaxFlightsquares */
  &traverse_and_deallocate,             /* STDegenerateTree */
  &traverse_and_deallocate,             /* STMaxNrNonTrivial */
  &traverse_and_deallocate,             /* STMaxThreatLength */
  &traverse_and_deallocate,             /* STMaxTimeRootDefenderFilter */
  &traverse_and_deallocate,             /* STMaxTimeDefenderFilter */
  &traverse_and_deallocate,             /* STMaxTimeHelpFilter */
  &traverse_and_deallocate,             /* STMaxTimeSeriesFilter */
  &traverse_and_deallocate,             /* STMaxSolutionsRootSolvableFilter */
  &traverse_and_deallocate,             /* STMaxSolutionsRootDefenderFilter */
  &traverse_and_deallocate,             /* STMaxSolutionsHelpFilter */
  &traverse_and_deallocate,             /* STMaxSolutionsSeriesFilter */
  &traverse_and_deallocate,             /* STStopOnShortSolutionsRootSolvableFilter */
  &traverse_and_deallocate,             /* STStopOnShortSolutionsHelpFilter */
  &traverse_and_deallocate              /* STStopOnShortSolutionsSeriesFilter */
};

/* Deallocate a branch
 * @param branch identifies branch
 * @return index of slice representing the play after the branch
 */
slice_index branch_deallocate(slice_index branch)
{
  slice_index result;
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",branch);
  TraceFunctionParamListEnd();

  slice_traversal_init(&st,&slice_deallocators,&result);
  traverse_slices(branch,&st);
  
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
    if (slices[result].type==type)
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
