#include "stipulation/help_play/branch.h"
#include "pypipe.h"
#include "stipulation/constraint.h"
#include "stipulation/branch.h"
#include "stipulation/end_of_branch.h"
#include "stipulation/dead_end.h"
#include "stipulation/end_of_branch_goal.h"
#include "stipulation/move.h"
#include "stipulation/boolean/binary.h"
#include "stipulation/help_play/adapter.h"
#include "trace.h"

#include <assert.h>
#include <limits.h>

/* Order in which the slice types dealing with help moves appear
 */
static slice_index const slice_rank_order[] =
{
  STHelpAdapter,
  STSeriesAdapter,
  STConstraint,
  STFindByIncreasingLength,
  STFindShortest,
  STStopOnShortSolutionsFilter,
  STIntelligentFilter,
  STEndOfBranch,
  STEndOfBranchForced,
  STDeadEnd,
  STForkOnRemaining,
  STEndOfIntro,

  STReadyForHelpMove,
  STReadyForSeriesMove,
  STReadyForSeriesDummyMove,
  STHelpHashed,
  STCheckZigzagJump,
  STDoubleMateFilter,
  STCounterMateFilter,
  STEnPassantFilter,
  STCastlingFilter,
  STPrerequisiteOptimiser,
  STForkOnRemaining,
  STMoveGenerator,
  STOrthodoxMatingMoveGenerator,
  STMove,
  STDummyMove,
  STMaxTimeGuard,
  STMaxSolutionsGuard,
  STRestartGuard,
  STKeepMatingFilter,
  STGoalReachableGuardFilter,
  STEndOfRoot,

  STEndOfBranchGoal,
  STEndOfBranchGoalImmobile,
  STDeadEndGoal,
  STSelfCheckGuard,
  STCheckZigzagLanding
};

enum
{
  nr_slice_rank_order_elmts = (sizeof slice_rank_order
                               / sizeof slice_rank_order[0]),
  no_slice_rank = INT_MAX
};

/* Determine the rank of a help slice type
 * @param type help slice type
 * @param base base rank value
 * @return rank of type; no_slice_rank if the rank can't be determined
 */
static unsigned int get_slice_rank(slice_type type, unsigned int base)
{
  unsigned int result = no_slice_rank;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParam("%u",base);
  TraceFunctionParamListEnd();

  for (i = 0; i!=nr_slice_rank_order_elmts; ++i)
    if (slice_rank_order[(i+base)%nr_slice_rank_order_elmts]==type)
    {
      result = i+base;
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
} insertion_state_type;

static void start_insertion_traversal(slice_index si,
                                      insertion_state_type *state);

static boolean insert_common(slice_index si,
                             unsigned int rank,
                             insertion_state_type *state)
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
    unsigned int const prototype_rank = get_slice_rank(prototype_type,
                                                       state->base);
    if (rank>prototype_rank)
    {
      slice_index const copy = copy_slice(prototype);
      pipe_append(state->prev,copy);
      if (state->nr_prototypes>1)
      {
        insertion_state_type nested_state =
        {
            state->prototypes+1, state->nr_prototypes-1, prototype_rank+1, copy
        };
        start_insertion_traversal(copy,&nested_state);
      }

      result = true;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_visit_regular(slice_index si, stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",state->nr_prototypes);
  TraceFunctionParam("%u",state->base);
  TraceFunctionParam("%u",state->prev);
  TraceFunctionParamListEnd();

  {
    unsigned int const rank = get_slice_rank(slices[si].type,state->base);
    if (rank==no_slice_rank)
      ; /* nothing - not for insertion into this branch */
    else if (insert_common(si,rank,state))
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

static void insert_visit_end_of_branch_goal(slice_index si,
                                            stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",state->nr_prototypes);
  TraceFunctionParam("%u",state->base);
  TraceFunctionParam("%u",state->prev);
  TraceFunctionParamListEnd();

  {
    unsigned int const rank = get_slice_rank(slices[si].type,state->base);
    assert(rank!=no_slice_rank);
    if (insert_common(si,rank,state))
      ; /* nothing - work is done*/
    else
    {
      branch_insert_slices_nested(slices[si].u.fork.fork,
                                  state->prototypes,state->nr_prototypes);
      state->base = rank;
      state->prev = si;
      stip_traverse_structure_pipe(si,st);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_visit_check_zigzag_jump(slice_index si,
                                           stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",state->nr_prototypes);
  TraceFunctionParam("%u",state->base);
  TraceFunctionParam("%u",state->prev);
  TraceFunctionParamListEnd();

  {
    unsigned int const rank = get_slice_rank(slices[si].type,state->base);
    assert(rank!=no_slice_rank);
    if (insert_common(si,rank,state))
      ; /* nothing - work is done*/
    else
    {
      help_branch_insert_slices_nested(slices[si].u.fork.fork,
                                       state->prototypes,
                                       state->nr_prototypes);
      state->base = rank;
      state->prev = si;
      stip_traverse_structure_pipe(si,st);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_visit_proxy(slice_index si, stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;

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

static structure_traversers_visitors const insertion_visitors[] =
{
  { STEndOfBranchGoal,         &insert_visit_end_of_branch_goal },
  { STEndOfBranchGoalImmobile, &insert_visit_end_of_branch_goal },
  { STCheckZigzagJump,         &insert_visit_check_zigzag_jump  },
  { STProxy,                   &insert_visit_proxy              }
};

enum
{
  nr_insertion_visitors = sizeof insertion_visitors / sizeof insertion_visitors[0]
};

static void start_insertion_traversal(slice_index si,
                                      insertion_state_type *state)
{
  unsigned int i;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",state->base);
  TraceFunctionParam("%u",state->nr_prototypes);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,state);
  for (i = 0; i!=nr_slice_rank_order_elmts; ++i)
    stip_structure_traversal_override_single(&st,
                                             slice_rank_order[i],
                                             &insert_visit_regular);
  stip_structure_traversal_override(&st,insertion_visitors,nr_insertion_visitors);
  stip_traverse_structure(slices[si].u.pipe.next,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert slices into a help branch; the elements of
 * prototypes are *not* deallocated by help_branch_insert_slices_nested().
 * The inserted slices are copies of the elements of prototypes).
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void help_branch_insert_slices_nested(slice_index si,
                                      slice_index const prototypes[],
                                      unsigned int nr_prototypes)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  {
    insertion_state_type state =
    {
        prototypes,
        nr_prototypes,
        get_slice_rank(slices[si].type,0),
        si
    };
    assert(state.base!=no_slice_rank);
    start_insertion_traversal(si,&state);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert slices into a help branch.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by help_branch_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void help_branch_insert_slices(slice_index si,
                               slice_index const prototypes[],
                               unsigned int nr_prototypes)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  help_branch_insert_slices_nested(si,prototypes,nr_prototypes);

  for (i = 0; i!=nr_prototypes; ++i)
    dealloc_slice(prototypes[i]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void increase_min_length_branch(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);
  slices[si].u.branch.min_length += 2;

  TraceFunctionExit(__func__);
  TraceFunctionParamListEnd();
}

/* Increase the min_length values of a branch to prevent subsequent shortening
 * to set it to an invalidly low value
 * @param si identifies slice where to start
 */
static void increase_min_length(slice_index si)
{
  stip_structure_traversal st;
  slice_structural_type structural_type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  for (structural_type = 0;
       structural_type!=nr_slice_structure_types;
       ++structural_type)
    if (slice_structure_is_subclass(structural_type,slice_structure_branch))
      stip_structure_traversal_override_by_structure(&st,
                                                     structural_type,
                                                     &increase_min_length_branch);
    else if (slice_structure_is_subclass(structural_type,slice_structure_pipe))
      stip_structure_traversal_override_by_structure(&st,
                                                     structural_type,
                                                     &stip_traverse_structure_pipe);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionParamListEnd();
}

/* Shorten a help branch by 1 half move
 * @param identifies entry slice of branch to be shortened
 */
void help_branch_shorten(slice_index adapter)
{
  slice_index const next = slices[adapter].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  assert(slices[adapter].type==STHelpAdapter);

  {
    /* find the new spot for adapter by inserting a copy */
    slice_index const prototype = copy_slice(adapter);
    help_branch_insert_slices(next,&prototype,1);
  }

  {
    /* move adapter to its new spot */
    slice_index const copy = branch_find_slice(STHelpAdapter,next);
    assert(copy!=no_slice);
    pipe_link(slices[adapter].prev,next);
    pipe_append(copy,adapter);
    pipe_remove(copy);
  }

  /* adjust the length and min_length members */
  --slices[adapter].u.branch.length;
  --slices[adapter].u.branch.min_length;
  if (slices[adapter].u.branch.min_length<slack_length_help)
    increase_min_length(adapter);
  branch_shorten_slices(next,STHelpAdapter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a slice marking the end of the branch
 * @param si identifies the entry slice of a help branch
 * @param end_proto end of branch prototype slice
 * @param parity indicates after which help move of the branch to insert
 */
void help_branch_insert_end_of_branch(slice_index si,
                                      slice_index end_proto,
                                      unsigned int parity)
{
  slice_index pos = si;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",end_proto);
  TraceFunctionParam("%u",parity);
  TraceFunctionParamListEnd();

  do
  {
    pos = branch_find_slice(STReadyForHelpMove,pos);
    assert(pos!=no_slice);
  } while ((slices[pos].u.branch.length-slack_length_help)%2
           !=(parity-slack_length_help)%2);

  help_branch_insert_slices(pos,&end_proto,1);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a fork to the next branch
 * @param si identifies the entry slice of a help branch
 * @param next identifies the entry slice of the next branch
 * @param parity indicates after which help move of the branch to insert
 */
void help_branch_set_end(slice_index si,
                         slice_index next,
                         unsigned int parity)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",next);
  TraceFunctionParam("%u",parity);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  TraceStipulation(next);

  help_branch_insert_end_of_branch(si,alloc_end_of_branch_slice(next),parity);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a fork to the goal branch
 * @param si identifies the entry slice of a help branch
 * @param next identifies the entry slice of the next branch
 * @param parity indicates after which help move of the branch to insert
 */
void help_branch_set_end_goal(slice_index si,
                              slice_index to_goal,
                              unsigned int parity)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",to_goal);
  TraceFunctionParam("%u",parity);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  TraceStipulation(to_goal);

  help_branch_insert_end_of_branch(si,alloc_end_of_branch_goal(to_goal),parity);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a series branch with STEndOfBranchForced slices (typically for a
 * hr stipulation)
 * @param si entry slice of branch to be instrumented
 * @param forced identifies branch forced on the defender
 * @param parity indicates after which help move of the branch to insert
 */
void help_branch_set_end_forced(slice_index si,
                                slice_index forced,
                                unsigned int parity)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",forced);
  TraceFunctionParam("%u",parity);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  TraceStipulation(forced);

  help_branch_insert_end_of_branch(si,alloc_end_of_branch_forced(forced),parity);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a series branch with STConstraint slices (typically for a hr
 * stipulation)
 * @param si entry slice of branch to be instrumented
 * @param constraint identifies branch that constrains the attacker
 * @param parity indicates after which help move of the branch to insert
 */
void help_branch_insert_constraint(slice_index si,
                                   slice_index constraint,
                                   unsigned int parity)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",constraint);
  TraceFunctionParam("%u",parity);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  TraceStipulation(constraint);

  help_branch_insert_end_of_branch(si,alloc_constraint_slice(constraint),parity);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a help branch.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param proxy_to_next identifies slice leading towards goal
 * @return index of entry slice into allocated series branch
 */
slice_index alloc_help_branch(stip_length_type length,
                              stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  {
    slice_index const adapter = alloc_help_adapter_slice(length,min_length);
    slice_index const ready1 = alloc_branch(STReadyForHelpMove,
                                            length,min_length);
    slice_index const move1 = alloc_move_slice();
    slice_index const ready2 = alloc_branch(STReadyForHelpMove,
                                            length-1,min_length-1);
    slice_index const move2 = alloc_move_slice();

    slice_index const deadend = alloc_dead_end_slice();

    pipe_link(adapter,ready1);
    pipe_link(ready1,move1);
    pipe_link(move1,ready2);
    pipe_link(ready2,move2);
    pipe_link(move2,adapter);

    if ((length-slack_length_help)%2==0)
      pipe_append(adapter,deadend);
    else
      pipe_append(move1,deadend);

    result = adapter;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void serve_as_root_hook(slice_index si, stip_structure_traversal *st)
{
  slice_index * const root_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *root_slice = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Create the root slices of a help branch
 * @param adapter identifies the adapter slice at the beginning of the branch
 * @return identifier of initial root slice
 */
slice_index help_branch_make_root_slices(slice_index adapter)
{
  slice_index result = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st;
    slice_structural_type i;

    TraceStipulation(adapter);

    stip_structure_traversal_init(&st,&result);
    for (i = 0; i!=nr_slice_structure_types; ++i)
      if (slice_structure_is_subclass(i,slice_structure_pipe))
        stip_structure_traversal_override_by_structure(&st,i,&pipe_make_root);
      else if (slice_structure_is_subclass(i,slice_structure_binary))
        stip_structure_traversal_override_by_structure(&st,i,&binary_make_root);
    stip_structure_traversal_override_single(&st,STEndOfRoot,&serve_as_root_hook);
    stip_traverse_structure(adapter,&st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Wrap the slices representing the initial moves of the solution with
 * slices of appropriately equipped slice types
 * @param adapter identifies the adapter slice at the beginning of the branch
 * @return identifier of initial root slice
 */
slice_index help_make_root(slice_index adapter)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  assert(slices[adapter].type==STHelpAdapter
         || slices[adapter].type==STSeriesAdapter);

  {
    slice_index const prototype = alloc_pipe(STEndOfRoot);
    help_branch_insert_slices(adapter,&prototype,1);
    result = help_branch_make_root_slices(adapter);
    branch_shorten_slices(adapter,STEndOfRoot);
    pipe_remove(adapter);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Spin the intro slices off a nested help branch
 * @param adapter identifies adapter slice of the nested help branch
 */
void help_spin_off_intro(slice_index adapter)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  assert(slices[adapter].type==STHelpAdapter
         || slices[adapter].type==STSeriesAdapter);

  TraceStipulation(adapter);

  {
    slice_index const prototype = alloc_pipe(STEndOfIntro);
    help_branch_insert_slices(adapter,&prototype,1);
  }

  {
    slice_index const next = slices[adapter].u.pipe.next;
    slice_index nested = no_slice;
    stip_structure_traversal st;
    slice_structural_type i;

    stip_structure_traversal_init(&st,&nested);
    for (i = 0; i!=nr_slice_structure_types; ++i)
      if (slice_structure_is_subclass(i,slice_structure_pipe))
        stip_structure_traversal_override_by_structure(&st,i,&pipe_make_root);
      else if (slice_structure_is_subclass(i,slice_structure_binary))
        stip_structure_traversal_override_by_structure(&st,i,&binary_make_root);
    stip_structure_traversal_override_single(&st,STEndOfIntro,&serve_as_root_hook);
    stip_traverse_structure(next,&st);

    pipe_link(slices[adapter].prev,next);
    link_to_branch(adapter,nested);
    slices[adapter].prev = no_slice;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Produce slices representing set play.
 * @param adapter identifies the adapter slice at the beginning of the branch
 * @return entry point of the slices representing set play
 *         no_slice if set play is not applicable
 */
slice_index help_branch_make_setplay(slice_index adapter)
{
  slice_index result;
  stip_length_type const length = slices[adapter].u.branch.length;
  stip_length_type min_length = slices[adapter].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  assert(slices[adapter].type==STHelpAdapter);

  if (min_length==slack_length_help)
    min_length += 2;

  if (length>slack_length_help+1)
  {
    slice_index const next = slices[adapter].u.pipe.next;
    slice_index const prototypes[] =
    {
      alloc_help_adapter_slice(length-1,min_length-1),
      alloc_pipe(STEndOfRoot)
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    help_branch_insert_slices(next,prototypes,nr_prototypes);

    {
      slice_index const set_adapter = branch_find_slice(STHelpAdapter,next);
      assert(set_adapter!=no_slice);
      result = help_branch_make_root_slices(set_adapter);
      pipe_remove(set_adapter);
    }
  }
  else
    result = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}
