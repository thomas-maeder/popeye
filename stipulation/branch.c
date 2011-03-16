#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/series_play/branch.h"
#include "pypipe.h"
#include "trace.h"

#include <assert.h>

/* Order in which the slice types at the root appear
 */
static slice_index const root_slice_rank_order[] =
{
  STProxy,
  STSetplayFork,
  STMoveInverter,
  STEndOfPhaseWriter,
  STOutputPlaintextTreeMoveInversionCounter,
  STOutputPlaintextLineMoveInversionCounter,
  /* in hXN.5 with set play, there are 2 move inversions in a row! */
  STMoveInverter,
  STOutputPlaintextLineMoveInversionCounter,
  STIllegalSelfcheckWriter,
  STSelfCheckGuard,
  STEndOfPhaseWriter,
  STMaxSolutionsRootSolvableFilter,
  STStopOnShortSolutionsInitialiser,
  STReciprocal,
  STQuodlibet,
  STStipulationReflexAttackSolver,
  STDefenseAdapter,
  STAttackAdapter,
  STHelpAdapter,
  STSeriesAdapter
};

enum
{
  nr_root_slice_rank_order_elmts = (sizeof root_slice_rank_order
                                    / sizeof root_slice_rank_order[0]),
                                    no_root_branch_slice_type = INT_MAX
};

/* Determine the rank of a slice type
 * @param type defense slice type
 * @return rank of type; nr_defense_slice_rank_order_elmts if the rank can't
 *         be determined
 */
static unsigned int get_root_slice_rank(SliceType type, unsigned int base)
{
  unsigned int result = no_root_branch_slice_type;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceEnumerator(SliceType,type,"");
  TraceFunctionParam("%u",base);
  TraceFunctionParamListEnd();

  for (i = base; i!=nr_root_slice_rank_order_elmts; ++i)
    if (root_slice_rank_order[i]==type)
    {
      result = i;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void root_branch_insert_slices_recursive(slice_index si,
                                                slice_index const prototypes[],
                                                unsigned int nr_prototypes,
                                                unsigned int base)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParam("%u",base);
  TraceFunctionParamListEnd();

  {
    SliceType const prototype_type = slices[prototypes[0]].type;
    unsigned int prototype_rank = get_root_slice_rank(prototype_type,base);

    do
    {
      slice_index const next = slices[si].u.pipe.next;
      if (slices[next].type==STProxy)
        si = next;
      else if (slices[next].type==STSetplayFork)
      {
        root_branch_insert_slices_recursive(slices[next].u.branch_fork.towards_goal,
                                            prototypes,nr_prototypes,
                                            base);
        si = next;
      }
      else
      {
        unsigned int const rank_next = get_root_slice_rank(slices[next].type,
                                                           base);
        if (rank_next==no_root_branch_slice_type)
          break;
        else if (rank_next>prototype_rank)
        {
          slice_index const copy = copy_slice(prototypes[0]);
          pipe_append(si,copy);
          if (nr_prototypes>1)
            root_branch_insert_slices_recursive(copy,
                                                prototypes+1,nr_prototypes-1,
                                                prototype_rank+1);
          break;
        }
        else if (slices[next].type==STQuodlibet || slices[next].type==STReciprocal)
        {
          root_branch_insert_slices_recursive(slices[next].u.binary.op1,
                                              prototypes,nr_prototypes,
                                              base);
          root_branch_insert_slices_recursive(slices[next].u.binary.op2,
                                              prototypes,nr_prototypes,
                                              base);
          break;
        }
        else if (slices[next].type==STGoalReachedTesting)
        {
          leaf_branch_insert_slices_nested(next,prototypes,nr_prototypes);
          break;
        }
        else if (slices[next].type==STHelpAdapter)
        {
          help_branch_insert_slices_nested(next,prototypes,nr_prototypes);
          break;
        }
        else if (slices[next].type==STSeriesAdapter)
        {
          series_branch_insert_slices_nested(next,prototypes,nr_prototypes);
          break;
        }
        else if (slices[next].type==STAttackAdapter
                 || slices[next].type==STDefenseAdapter)
        {
          battle_branch_insert_slices_nested(next,prototypes,nr_prototypes);
          break;
        }
        else
        {
          base = rank_next;
          si = next;
        }
      }
    } while (prototype_type!=slices[si].type);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert slices into a root branch.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by root_branch_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void root_branch_insert_slices(slice_index si,
                               slice_index const prototypes[],
                               unsigned int nr_prototypes)
{
  unsigned int i;
  unsigned int base;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  base = get_root_slice_rank(slices[si].type,0);
  root_branch_insert_slices_recursive(si,prototypes,nr_prototypes,base);

  for (i = 0; i!=nr_prototypes; ++i)
    dealloc_slice(prototypes[i]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Order in which the slice types dealing with goals appear
 */
static slice_index const leaf_slice_rank_order[] =
{
  STAttackAdapter,
  STReadyForAttack,
  STOrthodoxMatingMoveGenerator,
  STGoalReachableGuardHelpFilter,
  STGoalReachableGuardSeriesFilter,
  STGoalReachedTesting,
  STAmuMateFilter,
  STUltraschachzwangGoalFilter,
  STGoalMateReachedTester,
  STGoalStalemateReachedTester,
  STGoalDoubleStalemateReachedTester,
  STAnticirceTargetSquareFilter,
  STGoalTargetReachedTester,
  STGoalCaptureReachedTester,
  STCirceSteingewinnFilter,
  STGoalSteingewinnReachedTester,
  STGoalEnpassantReachedTester,
  STGoalDoubleMateReachedTester,
  STGoalCounterMateReachedTester,
  STGoalCastlingReachedTester,
  STGoalAutoStalemateReachedTester,
  STGoalCircuitReachedTester,
  STAnticirceExchangeFilter,
  STGoalExchangeReachedTester,
  STAnticirceCircuitSpecial,
  STCirceCircuitSpecial,
  STGoalCircuitByRebirthReachedTester,
  STCirceExchangeSpecial,
  STAnticirceExchangeSpecial,
  STGoalExchangeByRebirthReachedTester,
  STGoalAnyReachedTester,
  STGoalProofgameReachedTester,
  STGoalAToBReachedTester,
  STGoalMateOrStalemateReachedTester,
  STGoalCheckReachedTester,
  STSelfCheckGuard,
  STGoalNotCheckReachedTester,
  STGoalImmobileReachedTester,
  STPiecesParalysingMateFilter,
  STIntelligentDuplicateAvoider,
  STGoalReachedTested,
  STAttackAdapter,
  STDefenseAdapter,
  STReadyForDefense,
  STKeyWriter,
  STContinuationWriter,
  STVariationWriter,
  STRefutingVariationWriter,
  STCheckDetector,
  STMaxSolutionsSolvableFilter,
  STOutputPlaintextTreeCheckWriter,
  STOutputPlaintextTreeGoalWriter,
  STOutputPlaintextTreeDecorationWriter,
  STOutputPlaintextLineLineWriter,
  STLeaf
};

enum
{
  nr_leaf_slice_rank_order_elmts = (sizeof leaf_slice_rank_order
                                    / sizeof leaf_slice_rank_order[0]),
  no_leaf_slice_type = INT_MAX
};

/* Determine the rank of a slice type
 * @param type defense slice type
 * @return rank of type; nr_defense_slice_rank_order_elmts if the rank can't
 *         be determined
 */
static unsigned int get_leaf_slice_rank(SliceType type)
{
  unsigned int result = no_leaf_slice_type;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceEnumerator(SliceType,type,"");
  TraceFunctionParamListEnd();

  for (i = 0; i!=nr_leaf_slice_rank_order_elmts; ++i)
    if (leaf_slice_rank_order[i]==type)
    {
      result = i;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert slices into a leaf branch.
 * The inserted slices are copies of the elements of prototypes).
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void leaf_branch_insert_slices_nested(slice_index si,
                                         slice_index const prototypes[],
                                         unsigned int nr_prototypes)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  {
    SliceType const prototype_type = slices[prototypes[0]].type;
    unsigned int prototype_rank = get_leaf_slice_rank(prototype_type);

    if (prototype_rank==no_leaf_slice_type)
    {
      if (nr_prototypes>1)
        leaf_branch_insert_slices_nested(si,prototypes+1,nr_prototypes-1);
    }
    else
      do
      {
        slice_index const next = slices[si].u.pipe.next;
        if (slices[next].type==STProxy)
          si = next;
        else if (slices[next].type==STQuodlibet
                 || slices[next].type==STReciprocal)
        {
          leaf_branch_insert_slices_nested(slices[next].u.binary.op1,
                                           prototypes,nr_prototypes);
          leaf_branch_insert_slices_nested(slices[next].u.binary.op2,
                                           prototypes,nr_prototypes);
          break;
        }
        else
        {
          unsigned int const rank_next = get_leaf_slice_rank(slices[next].type);
          if (rank_next==no_leaf_slice_type)
            break;
          else if (rank_next>prototype_rank)
          {
            slice_index const copy = copy_slice(prototypes[0]);
            pipe_append(si,copy);
            if (nr_prototypes>1)
              leaf_branch_insert_slices_nested(copy,prototypes+1,nr_prototypes-1);
            break;
          }
          else
            si = next;
        }
      } while (prototype_type!=slices[si].type);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert slices into a leaf branch.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by leaf_branch_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void leaf_branch_insert_slices(slice_index si,
                               slice_index const prototypes[],
                               unsigned int nr_prototypes)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  leaf_branch_insert_slices_nested(si,prototypes,nr_prototypes);

  for (i = 0; i!=nr_prototypes; ++i)
    dealloc_slice(prototypes[i]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
  TraceEnumerator(SliceType,type,"");
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

/* Traversal of the moves of some branch slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_move_slice(slice_index si, stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(st->remaining>0);

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
  st->remaining = STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED;

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
