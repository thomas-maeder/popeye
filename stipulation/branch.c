#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/check_zigzag_jump.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "solving/fork_on_remaining.h"
#include "pypipe.h"
#include "trace.h"

#include <assert.h>

/* Order in which the slice types at the root appear
 */
static slice_index const root_slice_rank_order[] =
{
  STProxy,
  STTemporaryHackFork,
  STOutputModeSelector,
  STSetplayFork,
  STMoveInverter,
  STOutputPlaintextMoveInversionCounter,
  /* in hXN.5 with set play, there are 2 move inversions in a row! */
  STMoveInverter,
  STOutputPlaintextMoveInversionCounter,
  STIllegalSelfcheckWriter,
  STSelfCheckGuard,
  STMaxSolutionsInitialiser,
  STStopOnShortSolutionsInitialiser,
  STEndOfPhaseWriter,
  STAttackAdapter,
  STDefenseAdapter,
  STHelpAdapter
};

enum
{
  nr_root_slice_rank_order_elmts = (sizeof root_slice_rank_order
                                    / sizeof root_slice_rank_order[0]),
  no_root_slice_rank = INT_MAX
};

/* Determine the rank of a slice type
 * @param type defense slice type
 * @return rank of type; nr_defense_slice_rank_order_elmts if the rank can't
 *         be determined
 */
static unsigned int get_root_slice_rank(slice_type type, unsigned int base)
{
  unsigned int result = no_root_slice_rank;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type,"");
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

typedef struct
{
    slice_index const *prototypes;
    unsigned int nr_prototypes;
    unsigned int base;
    slice_index prev;
} root_insertion_state_type;

static void start_root_insertion_traversal(slice_index si,
                                           root_insertion_state_type *state);

static boolean root_insert_common(slice_index si,
                                  unsigned int rank,
                                  root_insertion_state_type *state)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",rank);
  TraceFunctionParam("%u",state->prev);
  TraceFunctionParam("%u",state->base);
  TraceFunctionParamListEnd();

  if (slices[si].type==slices[state->prototypes[0]].type)
    result = true; /* we are done */
  else
  {
    slice_index const prototype = state->prototypes[0];
    slice_type const prototype_type = slices[prototype].type;
    unsigned int const prototype_rank = get_root_slice_rank(prototype_type,
                                                            state->base);
    if (rank>prototype_rank)
    {
      slice_index const copy = copy_slice(prototype);
      pipe_append(state->prev,copy);
      if (state->nr_prototypes>1)
      {
        root_insertion_state_type nested_state =
        {
            state->prototypes+1, state->nr_prototypes-1, prototype_rank+1, copy
        };
        start_root_insertion_traversal(copy,&nested_state);
      }

      result = true;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void root_insert_visit_regular(slice_index si,
                                      stip_structure_traversal *st)
{
  root_insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",state->nr_prototypes);
  TraceFunctionParam("%u",state->base);
  TraceFunctionParam("%u",state->prev);
  TraceFunctionParamListEnd();

  {
    unsigned int const rank = get_root_slice_rank(slices[si].type,state->base);
    if (rank==no_root_slice_rank)
      ; /* nothing - not for insertion into this branch */
    else if (root_insert_common(si,rank,state))
      ; /* nothing - work is done*/
    else
    {
      state->base = rank;
      state->prev = si;
      stip_traverse_structure_pipe(si,st);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void root_insert_visit_setplay_fork(slice_index si,
                                           stip_structure_traversal *st)
{
  root_insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",state->nr_prototypes);
  TraceFunctionParam("%u",state->base);
  TraceFunctionParam("%u",state->prev);
  TraceFunctionParamListEnd();

  {
    unsigned int const rank = get_root_slice_rank(slices[si].type,state->base);
    assert(rank!=no_root_slice_rank);
    if (root_insert_common(si,rank,state))
      ; /* nothing - work is done*/
    else
    {
      state->base = rank;
      state->prev = si;
      stip_traverse_structure_pipe(si,st);

      state->base = rank;
      state->prev = si;
      start_root_insertion_traversal(slices[si].u.fork.fork,state);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void root_insert_visit_battle_adapter(slice_index si,
                                             stip_structure_traversal *st)
{
  root_insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",state->nr_prototypes);
  TraceFunctionParam("%u",state->base);
  TraceFunctionParam("%u",state->prev);
  TraceFunctionParamListEnd();

  {
    unsigned int const rank = get_root_slice_rank(slices[si].type,state->base);
    assert(rank!=no_root_slice_rank);
    if (root_insert_common(si,rank,state))
      ; /* nothing - work is done*/
    else
      battle_branch_insert_slices_nested(si,
                                         state->prototypes,
                                         state->nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void root_insert_visit_help_adapter(slice_index si,
                                           stip_structure_traversal *st)
{
  root_insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",state->nr_prototypes);
  TraceFunctionParam("%u",state->base);
  TraceFunctionParam("%u",state->prev);
  TraceFunctionParamListEnd();

  {
    unsigned int const rank = get_root_slice_rank(slices[si].type,state->base);
    assert(rank!=no_root_slice_rank);
    if (root_insert_common(si,rank,state))
      ; /* nothing - work is done*/
    else
      help_branch_insert_slices_nested(si,
                                       state->prototypes,
                                       state->nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void root_insert_visit_goal_tester(slice_index si,
                                          stip_structure_traversal *st)
{
  root_insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",state->nr_prototypes);
  TraceFunctionParam("%u",state->base);
  TraceFunctionParam("%u",state->prev);
  TraceFunctionParamListEnd();

  {
    unsigned int const rank = get_root_slice_rank(slices[si].type,state->base);
    assert(rank!=no_root_slice_rank);
    if (root_insert_common(si,rank,state))
      ; /* nothing - work is done*/
    else
      leaf_branch_insert_slices_nested(si,
                                       state->prototypes,
                                       state->nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void root_insert_visit_proxy(slice_index si,
                                    stip_structure_traversal *st)
{
  root_insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",state->nr_prototypes);
  TraceFunctionParam("%u",state->base);
  TraceFunctionParam("%u",state->prev);
  TraceFunctionParamListEnd();

  state->prev = si;
  stip_traverse_structure_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors const root_insertion_visitors[] =
{
  { STSetplayFork,       &root_insert_visit_setplay_fork   },
  { STAttackAdapter,     &root_insert_visit_battle_adapter },
  { STDefenseAdapter,    &root_insert_visit_battle_adapter },
  { STHelpAdapter,       &root_insert_visit_help_adapter   },
  { STGoalReachedTester, &root_insert_visit_goal_tester    },
  { STProxy,             &root_insert_visit_proxy          }
};

enum
{
  nr_root_insertion_visitors =
      sizeof root_insertion_visitors / sizeof root_insertion_visitors[0]
};

static void start_root_insertion_traversal(slice_index si,
                                           root_insertion_state_type *state)
{
  unsigned int i;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",state->base);
  TraceFunctionParam("%u",state->nr_prototypes);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,state);
  for (i = 0; i!=nr_root_slice_rank_order_elmts; ++i)
    stip_structure_traversal_override_single(&st,
                                             root_slice_rank_order[i],
                                             &root_insert_visit_regular);
  stip_structure_traversal_override(&st,
                                    root_insertion_visitors,
                                    nr_root_insertion_visitors);
  stip_traverse_structure(slices[si].u.pipe.next,&st);

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

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  {
    root_insertion_state_type state =
    {
      prototypes, nr_prototypes,
      get_root_slice_rank(slices[si].type,0),
      si
    };
    assert(state.base!=no_root_slice_rank);
    start_root_insertion_traversal(si,&state);
  }

  for (i = 0; i!=nr_prototypes; ++i)
    dealloc_slice(prototypes[i]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
    slice_index const *prototypes;
    unsigned int nr_prototypes;
} branch_insertion_state_type;

static void branch_insert_visit_battle_adapter(slice_index si,
                                               stip_structure_traversal *st)
{
  branch_insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",state->nr_prototypes);
  TraceFunctionParamListEnd();

  battle_branch_insert_slices_nested(si,
                                     state->prototypes,
                                     state->nr_prototypes);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void branch_insert_visit_help_adapter(slice_index si,
                                             stip_structure_traversal *st)
{
  branch_insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",state->nr_prototypes);
  TraceFunctionParamListEnd();

  help_branch_insert_slices_nested(si,
                                   state->prototypes,
                                   state->nr_prototypes);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void branch_insert_visit_goal_tester(slice_index si,
                                            stip_structure_traversal *st)
{
  branch_insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",state->nr_prototypes);
  TraceFunctionParamListEnd();

  leaf_branch_insert_slices_nested(si,
                                   state->prototypes,
                                   state->nr_prototypes);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors const branch_insertion_visitors[] =
{
  { STAttackAdapter,     &branch_insert_visit_battle_adapter },
  { STDefenseAdapter,    &branch_insert_visit_battle_adapter },
  { STHelpAdapter,       &branch_insert_visit_help_adapter   },
  { STGoalReachedTester, &branch_insert_visit_goal_tester    }
};

enum
{
  nr_branch_insertion_visitors =
      sizeof branch_insertion_visitors / sizeof branch_insertion_visitors[0]
};

/* Insert slices into a generic branch; the elements of
 * prototypes are *not* deallocated by leaf_branch_insert_slices_nested().
 * The inserted slices are copies of the elements of prototypes).
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void branch_insert_slices_nested(slice_index si,
                                 slice_index const prototypes[],
                                 unsigned int nr_prototypes)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st;
    branch_insertion_state_type state = { prototypes, nr_prototypes };
    stip_structure_traversal_init(&st,&state);
    stip_structure_traversal_override(&st,
                                      branch_insertion_visitors,
                                      nr_branch_insertion_visitors);
    stip_traverse_structure(si,&st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert slices into a branch.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by leaf_branch_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void branch_insert_slices(slice_index si,
                          slice_index const prototypes[],
                          unsigned int nr_prototypes)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  branch_insert_slices_nested(si,prototypes,nr_prototypes);

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
  STGoalReachableGuardFilterMate,
  STGoalReachableGuardFilterStalemate,
  STGoalReachableGuardFilterProof,
  STGoalReachableGuardFilterProofFairy,
  STGoalReachedTester,
  STIntelligentSolutionsPerTargetPosCounter,
  STIntelligentDuplicateAvoider,
  STLegalMoveCounter,
  STAttackAdapter,
  STDefenseAdapter,
  STReadyForDefense,
  STMoveWriter,
  STKeyWriter,
  STRefutingVariationWriter,
  STCheckDetector,
  STMaxSolutionsGuard,
  STOutputPlaintextTreeCheckWriter,
  STOutputPlaintextTreeGoalWriter,
  STOutputPlaintextTreeDecorationWriter,
  STOutputPlaintextLineLineWriter,
  STTrue
};

enum
{
  nr_leaf_slice_rank_order_elmts = (sizeof leaf_slice_rank_order
                                    / sizeof leaf_slice_rank_order[0]),
  no_leaf_slice_rank = INT_MAX
};

/* Determine the rank of a slice type
 * @param type defense slice type
 * @return rank of type; nr_defense_slice_rank_order_elmts if the rank can't
 *         be determined
 */
static unsigned int get_leaf_slice_rank(slice_type type)
{
  unsigned int result = no_leaf_slice_rank;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type,"");
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

typedef struct
{
    slice_index const *prototypes;
    unsigned int nr_prototypes;
    unsigned int base;
    slice_index prev;
} leaf_insertion_state_type;

static void start_leaf_insertion_traversal(slice_index si,
                                           leaf_insertion_state_type *state);

static boolean leaf_insert_common(slice_index si,
                                  unsigned int rank,
                                  leaf_insertion_state_type *state)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",rank);
  TraceFunctionParam("%u",state->prev);
  TraceFunctionParam("%u",state->base);
  TraceFunctionParamListEnd();

  if (slices[si].type==slices[state->prototypes[0]].type)
    result = true; /* we are done */
  else
  {
    slice_index const prototype = state->prototypes[0];
    slice_type const prototype_type = slices[prototype].type;
    unsigned int const prototype_rank = get_leaf_slice_rank(prototype_type);
    if (rank>prototype_rank)
    {
      slice_index const copy = copy_slice(prototype);
      pipe_append(state->prev,copy);
      if (state->nr_prototypes>1)
      {
        leaf_insertion_state_type nested_state =
        {
            state->prototypes+1, state->nr_prototypes-1, prototype_rank+1, copy
        };
        start_leaf_insertion_traversal(copy,&nested_state);
      }

      result = true;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void leaf_insert_visit_regular(slice_index si,
                                      stip_structure_traversal *st)
{
  leaf_insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",state->nr_prototypes);
  TraceFunctionParam("%u",state->base);
  TraceFunctionParam("%u",state->prev);
  TraceFunctionParamListEnd();

  {
    unsigned int const rank = get_leaf_slice_rank(slices[si].type);
    if (rank==no_leaf_slice_rank)
      ; /* nothing - not for insertion into this branch */
    else if (leaf_insert_common(si,rank,state))
      ; /* nothing - work is done*/
    else
    {
      state->base = rank;
      state->prev = si;
      stip_traverse_structure_pipe(si,st);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void leaf_insert_visit_true_false(slice_index si,
                                         stip_structure_traversal *st)
{
  leaf_insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",state->nr_prototypes);
  TraceFunctionParam("%u",state->base);
  TraceFunctionParam("%u",state->prev);
  TraceFunctionParamListEnd();

  {
    unsigned int const rank = get_leaf_slice_rank(slices[si].type);
    if (rank==no_leaf_slice_rank)
      ; /* nothing - not for insertion into this branch */
    else
      leaf_insert_common(si,rank,state);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void start_leaf_insertion_traversal(slice_index si,
                                           leaf_insertion_state_type *state)
{
  unsigned int i;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",state->base);
  TraceFunctionParam("%u",state->nr_prototypes);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,state);
  for (i = 0; i!=nr_leaf_slice_rank_order_elmts; ++i)
    stip_structure_traversal_override_single(&st,
                                             leaf_slice_rank_order[i],
                                             &leaf_insert_visit_regular);
  stip_structure_traversal_override_single(&st,
                                           STTrue,
                                           &leaf_insert_visit_true_false);
  stip_structure_traversal_override_single(&st,
                                           STFalse,
                                           &leaf_insert_visit_true_false);
  stip_traverse_structure(slices[si].u.pipe.next,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
    leaf_insertion_state_type state =
    {
      prototypes, nr_prototypes,
      get_leaf_slice_rank(slices[si].type),
      si
    };
    assert(state.base!=no_leaf_slice_rank);
    start_leaf_insertion_traversal(si,&state);
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
slice_index alloc_branch(slice_type type,
                         stip_length_type length,
                         stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_pipe(type);
  slices[result].u.branch.length = length;
  slices[result].u.branch.min_length = min_length;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

typedef struct
{
  slice_type to_be_found;
  slice_index result;
} branch_find_slice_state_type;

static void branch_find_slice_pipe(slice_index si, stip_structure_traversal *st)
{
  branch_find_slice_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].type==state->to_be_found)
    state->result = si;
  else
    stip_traverse_structure_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void branch_find_slice_binary(slice_index si, stip_structure_traversal *st)
{
  branch_find_slice_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].type==state->to_be_found)
    state->result = si;
  else
  {
    slice_index result1;
    slice_index result2;

    stip_traverse_structure(slices[si].u.binary.op1,st);
    result1 = state->result;
    state->result = no_slice;

    stip_traverse_structure(slices[si].u.binary.op2,st);
    result2 = state->result;

    if (result1==no_slice)
      state->result = result2;
    else if (result2==no_slice)
      state->result = result1;
    else
    {
      assert(result1==result2);
      state->result = result1;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Find the next slice with a specific type in a branch
 * @param type type of slice to be found
 * @param si identifies the slice where to start searching
 * @return identifier for slice with type type; no_slice if none is found
 */
slice_index branch_find_slice(slice_type type, slice_index si)
{
  branch_find_slice_state_type state = { type, no_slice };
  stip_structure_traversal st;
  slice_structural_type structural_type;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  for (structural_type = 0;
       structural_type!=nr_slice_structure_types;
       ++structural_type)
    if (slice_structure_is_subclass(structural_type,slice_structure_pipe))
      stip_structure_traversal_override_by_structure(&st,
                                                     structural_type,
                                                     &branch_find_slice_pipe);
    else if (slice_structure_is_subclass(structural_type,slice_structure_binary))
      stip_structure_traversal_override_by_structure(&st,
                                                     structural_type,
                                                     &branch_find_slice_binary);
  stip_traverse_structure(slices[si].u.pipe.next,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",state.result);
  TraceFunctionResultEnd();
  return state.result;
}

/* Traversal of the moves of a branch
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_branch(slice_index si, stip_moves_traversal *st)
{
  stip_length_type const save_remaining = st->remaining;
  stip_length_type const save_full_length = st->full_length;
  stip_traversal_context_type const save_context = st->context;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  st->remaining = STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED;
  st->full_length = STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED;
  st->context = stip_traversal_context_global;

  stip_traverse_moves(si,st);

  st->context = save_context;
  st->full_length = save_full_length;
  st->remaining = save_remaining;
  TraceFunctionParam("->%u\n",st->remaining);

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

static void shorten_pipe(slice_index si, stip_structure_traversal *st)
{
  slice_type const * const end_type = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].type!=*end_type)
    stip_traverse_structure_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionParamListEnd();
}

static void shorten_branch(slice_index si, stip_structure_traversal *st)
{
  slice_type const * const end_type = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].type!=*end_type)
  {
    stip_traverse_structure_pipe(si,st);
    slices[si].u.branch.length -= 2;
    slices[si].u.branch.min_length -= 2;
  }

  TraceFunctionExit(__func__);
  TraceFunctionParamListEnd();
}

/* Shorten slices of a branch by 2 half moves
 * @param start identfies start of sequence of slices to be shortened
 * @param end_type identifies type of slice where to stop shortening
 */
void branch_shorten_slices(slice_index start, slice_type end_type)
{
  stip_structure_traversal st;
  slice_structural_type structural_type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",start);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&end_type);
  for (structural_type = 0;
       structural_type!=nr_slice_structure_types;
       ++structural_type)
    if (slice_structure_is_subclass(structural_type,slice_structure_branch))
      stip_structure_traversal_override_by_structure(&st,
                                                     structural_type,
                                                     &shorten_branch);
    else if (slice_structure_is_subclass(structural_type,slice_structure_pipe))
      stip_structure_traversal_override_by_structure(&st,
                                                     structural_type,
                                                     &shorten_pipe);
  stip_traverse_structure(start,&st);

  TraceFunctionExit(__func__);
  TraceFunctionParamListEnd();
}
