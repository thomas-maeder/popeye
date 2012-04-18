#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/check_zigzag_jump.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "solving/fork_on_remaining.h"
#include "pypipe.h"
#include "debugging/trace.h"

#include <assert.h>

void deallocate_slice_insertion_prototypes(slice_index const prototypes[],
                                           unsigned int nr_prototypes)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  for (i = 0; i!=nr_prototypes; ++i)
    dealloc_slice(prototypes[i]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine the rank of a slice type
 * @param type defense slice type
 * @return rank of type; no_slice_rank if the rank can't be determined
 */
static unsigned int get_slice_rank(slice_type type,
                                   branch_slice_insertion_state_type const *state)
{
  unsigned int result = no_slice_rank;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParamListEnd();

  for (i = 0; i!=state->nr_slice_rank_order_elmts; ++i)
    if (state->slice_rank_order[(i+state->base_rank)%state->nr_slice_rank_order_elmts]==type)
    {
      result = i+state->base_rank;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void start_nested_insertion_traversal(slice_index si,
                                             branch_slice_insertion_state_type *state,
                                             stip_structure_traversal *outer)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,state);
  st.context = outer->context;
  st.level = outer->level;
  st.map = outer->map;
  stip_traverse_structure(slices[si].u.pipe.next,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void next_insertion(slice_index si,
                           unsigned int prototype_rank,
                           stip_structure_traversal *st)
{
  branch_slice_insertion_state_type const * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",prototype_rank);
  TraceFunctionParamListEnd();

  if (state->nr_prototypes>1)
  {
    branch_slice_insertion_state_type nested_state =
    {
        state->prototypes+1, state->nr_prototypes-1,
        state->slice_rank_order, state->nr_slice_rank_order_elmts,
        prototype_rank+1,
        si
    };
    start_nested_insertion_traversal(si,&nested_state,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean insert_before(slice_index si,
                             unsigned int rank,
                             stip_structure_traversal *st)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",rank);
  TraceFunctionParamListEnd();

  assert(rank!=no_slice_rank);

  {
    branch_slice_insertion_state_type * const state = st->param;
    slice_index const prototype = state->prototypes[0];
    slice_type const prototype_type = slices[prototype].type;
    unsigned int const prototype_rank = get_slice_rank(prototype_type,state);
    if (slices[si].type==slices[state->prototypes[0]].type)
    {
      next_insertion(si,prototype_rank,st);
      result = true;
    }
    else if (rank>prototype_rank)
    {
      slice_index const copy = copy_slice(prototype);
      pipe_append(state->prev,copy);
      next_insertion(copy,prototype_rank,st);
      result = true;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_visit_leaf(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    branch_slice_insertion_state_type * const state = st->param;
    unsigned int const rank = get_slice_rank(slices[si].type,state);
    if (rank!=no_slice_rank)
      insert_before(si,rank,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_beyond(slice_index si, stip_structure_traversal *st)
{
  branch_slice_insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->prev = si;
  stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_visit_pipe(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    branch_slice_insertion_state_type * const state = st->param;
    unsigned int const rank = get_slice_rank(slices[si].type,state);
    if (rank==no_slice_rank)
      insert_beyond(si,st);
    else if (insert_before(si,rank,st))
      ; /* nothing - work is done*/
    else
    {
      state->base_rank = rank;
      state->prev = si;
      stip_traverse_structure_children(si,st);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_visit_fork(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    branch_slice_insertion_state_type * const state = st->param;
    unsigned int const rank = get_slice_rank(slices[si].type,state);
    if (rank==no_slice_rank)
      insert_beyond(si,st);
    else if (insert_before(si,rank,st))
      ; /* nothing - work is done*/
    else
    {
      state->base_rank = rank;
      state->prev = si;
      stip_traverse_structure_children_pipe(si,st);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_visit_binary_operands(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    branch_slice_insertion_state_type * const state = st->param;
    branch_slice_insertion_state_type const save_state = *state;

    if (slices[si].u.binary.op1!=no_slice)
      insert_beyond(si,st);

    *state = save_state;

    if (slices[si].u.binary.op2!=no_slice)
    {
      assert(slices[slices[si].u.binary.op2].type==STProxy);
      insert_beyond(slices[si].u.binary.op2,st);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_visit_binary(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    branch_slice_insertion_state_type * const state = st->param;
    unsigned int const rank = get_slice_rank(slices[si].type,state);
    if (rank==no_slice_rank)
      insert_visit_binary_operands(si,st);
    else if (insert_before(si,rank,st))
      ; /* nothing - work is done*/
    else
    {
      state->base_rank = rank+1;
      insert_visit_binary_operands(si,st);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_visit_setplay_fork(slice_index si,
                                      stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    branch_slice_insertion_state_type * const state = st->param;
    unsigned int const rank = get_slice_rank(slices[si].type,state);
    if (insert_before(si,rank,st))
      ; /* nothing - work is done*/
    else
    {
      state->base_rank = rank;
      insert_beyond(si,st);

      state->base_rank = rank;
      state->prev = si;
      start_nested_insertion_traversal(slices[si].u.fork.fork,state,st);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_visit_battle_adapter(slice_index si,
                                        stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    branch_slice_insertion_state_type * const state = st->param;
    unsigned int const rank = get_slice_rank(slices[si].type,state);
    if (rank==no_slice_rank || !insert_before(si,rank,st))
      battle_branch_insert_slices_nested(si,
                                         state->prototypes,
                                         state->nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_visit_help_adapter(slice_index si,
                                      stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    branch_slice_insertion_state_type * const state = st->param;
    unsigned int const rank = get_slice_rank(slices[si].type,state);
    if (rank==no_slice_rank || !insert_before(si,rank,st))
      help_branch_insert_slices_nested(si,
                                       state->prototypes,
                                       state->nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise a structure traversal for inserting slices into a branch
 * @param st address of structure representing the traversal
 * @param state address of structure representing the insertion
 * @param context initial context of traversal
 */
void init_slice_insertion_traversal(stip_structure_traversal *st,
                                    branch_slice_insertion_state_type *state,
                                    stip_traversal_context_type context)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(st,state);
  st->context = context;
  stip_structure_traversal_override_by_structure(st,
                                                 slice_structure_leaf,
                                                 &insert_visit_leaf);
  stip_structure_traversal_override_by_structure(st,
                                                 slice_structure_pipe,
                                                 &insert_visit_pipe);
  stip_structure_traversal_override_by_structure(st,
                                                 slice_structure_branch,
                                                 &insert_visit_pipe);
  stip_structure_traversal_override_by_structure(st,
                                                 slice_structure_fork,
                                                 &insert_visit_fork);
  stip_structure_traversal_override_by_structure(st,
                                                 slice_structure_binary,
                                                 &insert_visit_binary);
  stip_structure_traversal_override_single(st,STProxy,&insert_beyond);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Order in which the slice types at the root appear
 */
static slice_index const slice_rank_order[] =
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
  STImmobilityTester,
  STAttackAdapter,
  STDefenseAdapter,
  STHelpAdapter
};

enum
{
  nr_slice_rank_order_elmts = sizeof slice_rank_order / sizeof slice_rank_order[0]
};

static structure_traversers_visitors const insertion_visitors[] =
{
  { STSetplayFork,    &insert_visit_setplay_fork   },
  { STAttackAdapter,  &insert_visit_battle_adapter },
  { STDefenseAdapter, &insert_visit_battle_adapter },
  { STHelpAdapter,    &insert_visit_help_adapter   }
};

enum
{
  nr_insertion_visitors = sizeof insertion_visitors / sizeof insertion_visitors[0]
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
  stip_structure_traversal st;
  branch_slice_insertion_state_type state = {
      prototypes, nr_prototypes,
      slice_rank_order, nr_slice_rank_order_elmts,
      0,
      si
  };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  state.base_rank = get_slice_rank(slices[si].type,&state);
  assert(state.base_rank!=no_slice_rank);
  init_slice_insertion_traversal(&st,&state,stip_traversal_context_intro);
  stip_structure_traversal_override(&st,insertion_visitors,nr_insertion_visitors);
  stip_traverse_structure(si,&st);

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
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  branch_insert_slices_nested(si,prototypes,nr_prototypes);
  deallocate_slice_insertion_prototypes(prototypes,nr_prototypes);

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
    stip_traverse_structure_children_pipe(si,st);

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

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_pipe,
                                                 &branch_find_slice_pipe);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_branch,
                                                 &branch_find_slice_pipe);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_fork,
                                                 &branch_find_slice_pipe);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_binary,
                                                 &branch_find_slice_binary);
  stip_traverse_structure(slices[si].u.pipe.next,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",state.result);
  TraceFunctionResultEnd();
  return state.result;
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
    stip_traverse_structure_children_pipe(si,st);

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
    stip_traverse_structure_children_pipe(si,st);
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

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",start);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&end_type);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_pipe,
                                                 &shorten_pipe);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_branch,
                                                 &shorten_branch);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_fork,
                                                 &shorten_pipe);
  stip_traverse_structure(start,&st);

  TraceFunctionExit(__func__);
  TraceFunctionParamListEnd();
}
