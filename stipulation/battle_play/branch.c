#include "stipulation/battle_play/branch.h"
#include "pypipe.h"
#include "pymovein.h"
#include "stipulation/constraint.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/dead_end.h"
#include "stipulation/end_of_branch.h"
#include "stipulation/end_of_branch_goal.h"
#include "stipulation/move.h"
#include "stipulation/boolean/binary.h"
#include "stipulation/battle_play/attack_adapter.h"
#include "stipulation/battle_play/defense_adapter.h"
#include "trace.h"

#include <assert.h>
#include <limits.h>

/* Order in which the slice types appear in battle branches
 * some types are not mentioned because they have variable ranks.
 */
static slice_index const slice_rank_order[] =
{
  STAttackAdapter,
  STTrivialEndFilter,
  STEndOfIntro,
  STReadyForAttack,
  STMaxThreatLengthStart, /* separate from STThreatStart to enable hashing*/
  STAttackHashed,
  STThreatStart,
  STConstraint,
  STMinLengthOptimiser,
  STCastlingFilter,
  STCounterMateFilter,
  STDoubleMateFilter,
  STEnPassantFilter,
  STPrerequisiteOptimiser,
  STDeadEnd,
  STDegenerateTree,
  STFindShortest,
  STShortSolutionsStart,
  STForkOnRemaining,
  STMoveGenerator,
  STKillerMoveMoveGenerator,
  STOrthodoxMatingMoveGenerator,
  STMove,
  STMaxTimeGuard,
  STMaxSolutionsGuard,
  STRestartGuard,
  STEndOfSolutionWriter,
  STThreatCollector,
  STKillerMoveCollector,
  STGoalReachedTester,
  STEndOfBranchGoal,
  STEndOfBranchGoalImmobile,
  STDeadEndGoal,
  STSelfCheckGuard,

  STDefenseAdapter,
  STEndOfIntro,
  STKeepMatingFilter,
  STEndOfBranch,
  STMaxNrNonTrivial,
  STMaxNrNonChecks,
  STRefutationsAllocator,
  STContinuationSolver,
  STMoveWriter,
  STKeyWriter,
  STTrySolver,
  STRefutationsSolver,
  STTryWriter,
  STCheckDetector,
  STMaxFlightsquares,
  STOutputPlaintextTreeCheckWriter,
  STOutputPlaintextTreeGoalWriter,
  STOutputPlaintextTreeDecorationWriter,
  STPlaySuppressor,
  STMinLengthGuard,
  STReadyForDefense,
  STConstraint,
  STEndOfBranchForced,
  STMaxThreatLength,
  STThreatSolver,
  STCounterMateFilter,
  STEnPassantFilter,
  STPrerequisiteOptimiser,
  STDeadEnd,
  STCheckZigzagJump,
  STForkOnRemaining,
  STForkOnRemaining,
  STMoveGenerator,
  STKillerMoveMoveGenerator,
  STCountNrOpponentMovesMoveGenerator,
  STKillerMoveFinalDefenseMove,
  STMove,
  STDummyMove,
  STMaxNrNonTrivialCounter,
  STRefutationsCollector,
  STRefutationWriter,
  STEndOfRoot,
  STKillerMoveCollector,
  STMinLengthGuard,
  STEndOfBranchGoal,
  STEndOfBranchGoalImmobile,
  STSelfCheckGuard,
  STCheckZigzagLanding,
  STNoShortVariations,
  STKeepMatingFilter,
  STThreatEnforcer,
  STMoveWriter,
  STRefutingVariationWriter,
  STOutputPlaintextTreeCheckWriter,
  STOutputPlaintextTreeDecorationWriter,
  STEndOfBranch
};

enum
{
  nr_slice_rank_order_elmts = (sizeof slice_rank_order
                               / sizeof slice_rank_order[0]),
  no_slice_rank = INT_MAX
};

/* Determine the rank of a defense slice type, relative to some base rank
 * @param type defense slice type
 * @param base base rank value
 * @return rank of type (>=base)
 *         base+nr_slice_rank_order_elmts if the rank can't be determined
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
      state->base = rank;
      state->prev = si;
      stip_traverse_structure_pipe(si,st);

      state->base = rank;
      state->prev = slices[si].u.fork.fork;
      /* start a new traversal to avoid a clash at the slice where next and
       * fork join */
      start_insertion_traversal(slices[si].u.fork.fork,state);
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
  { STEndOfBranchGoal,         &insert_visit_end_of_branch_goal  },
  { STEndOfBranchGoalImmobile, &insert_visit_end_of_branch_goal  },
  { STForkOnRemaining,         &insert_visit_check_zigzag_jump   },
  { STCheckZigzagJump,         &insert_visit_check_zigzag_jump   },
  { STProxy,                   &insert_visit_proxy               }
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

/* Insert slices into a battle branch; the elements of
 * prototypes are *not* deallocated by battle_branch_insert_slices_nested().
 * The inserted slices are copies of the elements of prototypes).
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void battle_branch_insert_slices_nested(slice_index si,
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
      prototypes, nr_prototypes,
      get_slice_rank(slices[si].type,0),
      si
    };
    assert(state.base!=no_slice_rank);
    start_insertion_traversal(si,&state);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert slices into a battle branch.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by battle_branch_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void battle_branch_insert_slices(slice_index si,
                                 slice_index const prototypes[],
                                 unsigned int nr_prototypes)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  battle_branch_insert_slices_nested(si,prototypes,nr_prototypes);

  for (i = 0; i!=nr_prototypes; ++i)
    dealloc_slice(prototypes[i]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert slices into a battle branch, starting between defense and attack move
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by battle_branch_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void attack_branch_insert_slices(slice_index si,
                                 slice_index const prototypes[],
                                 unsigned int nr_prototypes)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  {
    unsigned int const ready_rank = get_slice_rank(STReadyForDefense,0);
    unsigned int const move_rank = get_slice_rank(STMove,ready_rank);
    insertion_state_type state =
    {
      prototypes, nr_prototypes,
      move_rank,
      si
    };
    assert(state.base!=no_slice_rank);
    start_insertion_traversal(si,&state);
  }

  for (i = 0; i!=nr_prototypes; ++i)
    dealloc_slice(prototypes[i]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert slices into a battle branch, starting between attack and defense move
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by battle_branch_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void defense_branch_insert_slices(slice_index si,
                                  slice_index const prototypes[],
                                  unsigned int nr_prototypes)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  {
    unsigned int const ready_rank = get_slice_rank(STReadyForAttack,0);
    unsigned int const move_rank = get_slice_rank(STMove,ready_rank);
    insertion_state_type state =
    {
      prototypes, nr_prototypes,
      move_rank,
      si
    };
    assert(state.base!=no_slice_rank);
    start_insertion_traversal(si,&state);
  }

  for (i = 0; i!=nr_prototypes; ++i)
    dealloc_slice(prototypes[i]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a branch consisting mainly of an defense move
 * @param next identifies the slice that the defense branch lead sto
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of entry slice to allocated branch
 */
slice_index alloc_defense_branch(slice_index next,
                                 stip_length_type length,
                                 stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",next);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  {
    slice_index const adapter = alloc_defense_adapter_slice(length,min_length);
    slice_index const ready = alloc_branch(STReadyForDefense,length,min_length);
    slice_index const deadend = alloc_dead_end_slice();
    slice_index const defense = alloc_move_slice();

    pipe_link(adapter,ready);
    pipe_link(ready,deadend);
    pipe_link(deadend,defense);
    pipe_link(defense,next);

    result = adapter;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a branch that represents battle play
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of entry slice to allocated branch
 */
slice_index alloc_battle_branch(stip_length_type length,
                                stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  assert(length>slack_length_battle);
  assert(min_length>=slack_length_battle);

  {
    slice_index const adapter = alloc_attack_adapter_slice(length,min_length);
    slice_index const aready = alloc_branch(STReadyForAttack,length,min_length);
    slice_index const adeadend = alloc_dead_end_slice();
    slice_index const attack = alloc_move_slice();
    slice_index const dready = alloc_branch(STReadyForDefense,
                                            length-1,min_length-1);
    slice_index const ddeadend = alloc_dead_end_slice();
    slice_index const defense = alloc_move_slice();

    pipe_link(adapter,aready);
    pipe_link(aready,adeadend);
    pipe_link(adeadend,attack);
    pipe_link(attack,dready);
    pipe_link(dready,ddeadend);
    pipe_link(ddeadend,defense);
    pipe_link(defense,adapter);

    result = adapter;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Add the copy of a slice into the set play branch
 * @param si slice index
 * @param st state of traversal
 */
static void copy_to_setplay(slice_index si, stip_structure_traversal *st)
{
  slice_index * const result = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  {
    slice_index const copy = copy_slice(si);
    link_to_branch(copy,*result);
    *result = copy;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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

/* Produce slices representing set play.
 * This is supposed to be invoked from within the slice type specific
 * functions invoked by stip_apply_setplay.
 * @param adapter identifies the adapter slice into the battle branch
 * @return entry point of the slices representing set play
 *         no_slice if set play is not applicable
 */
slice_index battle_branch_make_setplay(slice_index adapter)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  {
    slice_index const start = branch_find_slice(STReadyForDefense,adapter);
    stip_structure_traversal st;
    slice_structural_type type;

    slice_index const prototype = alloc_pipe(STEndOfRoot);
    battle_branch_insert_slices(adapter,&prototype,1);

    assert(start!=no_slice);

    stip_structure_traversal_init(&st,&result);
    for (type = 0; type!=nr_slice_structure_types; ++type)
      stip_structure_traversal_override_by_structure(&st,type,&copy_to_setplay);
    stip_structure_traversal_override_single(&st,STEndOfRoot,&serve_as_root_hook);
    stip_traverse_structure(start,&st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}

/* Make the postkey play representation of a non-postkey play representation
 * @param adapter identifies adapter slice into battle branch
 * @return identifier to adapter slice into postkey representation
 */
slice_index battle_branch_make_postkeyplay(slice_index adapter)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  TraceStipulation(adapter);

  assert(slices[adapter].type==STAttackAdapter);

  {
    stip_length_type const length = slices[adapter].u.branch.length;
    stip_length_type const min_length = slices[adapter].u.branch.min_length;
    slice_index const proto = alloc_defense_adapter_slice(length-1,
                                                          min_length-1);
    battle_branch_insert_slices(adapter,&proto,1);

    result = branch_find_slice(STDefenseAdapter,adapter);
    assert(result!=no_slice);

    branch_shorten_slices(adapter,STDefenseAdapter);
    pipe_remove(adapter);
  }

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}

static void attack_adapter_make_postkeyplay(slice_index adapter,
                                            stip_structure_traversal *st)
{
  slice_index * const postkey = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  *postkey = battle_branch_make_postkeyplay(adapter);

  TraceFunctionExit(__func__);
  TraceFunctionParamListEnd();
}

static void move_to_postkey(slice_index si, stip_structure_traversal *st)
{
  slice_index * const root_slice = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  link_to_branch(si,*root_slice);
  *root_slice = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Attempt to apply the postkey play option to the current stipulation
 * @param root_proxy identifies root proxy slice
 * @return true iff postkey play option is applicable (and has been
 *              applied)
 */
boolean battle_branch_apply_postkeyplay(slice_index root_proxy)
{
  boolean result;
  slice_index postkey_slice;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root_proxy);
  TraceFunctionParamListEnd();

  TraceStipulation(root_proxy);

  stip_structure_traversal_init(&st,&postkey_slice);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_pipe,
                                                 &move_to_postkey);
  stip_structure_traversal_override_single(&st,
                                           STAttackAdapter,
                                           &attack_adapter_make_postkeyplay);
  stip_traverse_structure(slices[root_proxy].u.pipe.next,&st);

  if (postkey_slice==no_slice)
    result = false;
  else
  {
    link_to_branch(root_proxy,postkey_slice);

    {
      slice_index const prototype = alloc_move_inverter_slice();
      root_branch_insert_slices(root_proxy,&prototype,1);
    }

    result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}

/* Create the root slices of a battle branch
 * @param adapter identifies the adapter slice at the beginning of the branch
 * @return identifier of initial root slice
 */
slice_index battle_branch_make_root_slices(slice_index adapter)
{
  slice_index result = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st;
    slice_structural_type i;

    slice_index const prototype = alloc_pipe(STEndOfRoot);
    battle_branch_insert_slices(adapter,&prototype,1);

    stip_structure_traversal_init(&st,&result);
    for (i = 0; i!=nr_slice_structure_types; ++i)
      if (slice_structure_is_subclass(i,slice_structure_pipe))
        stip_structure_traversal_override_by_structure(&st,i,&pipe_make_root);
      else if (slice_structure_is_subclass(i,slice_structure_binary))
        stip_structure_traversal_override_by_structure(&st,i,&binary_make_root);
    stip_structure_traversal_override_single(&st,
                                             STEndOfRoot,
                                             &serve_as_root_hook);
    stip_traverse_structure(adapter,&st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}

/* Wrap the slices representing the initial moves of the solution with
 * slices of appropriately equipped slice types
 * @param adapter identifies the adapter slice at the beginning of the branch
 * @return identifier of root slice
 */
slice_index battle_make_root(slice_index adapter)
{
  slice_index result = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  result = battle_branch_make_root_slices(adapter);
  assert(result!=no_slice);

  branch_shorten_slices(adapter,STEndOfRoot);
  pipe_remove(adapter);

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}

/* Spin the intro slices off a nested battle branch
 * @param adapter identifies adapter slice of the nested help branch
 */
void battle_spin_off_intro(slice_index adapter)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  assert(slices[adapter].type==STAttackAdapter
         || slices[adapter].type==STDefenseAdapter);

  TraceStipulation(adapter);

  {
    slice_index const prototype = alloc_pipe(STEndOfIntro);
    battle_branch_insert_slices(adapter,&prototype,1);
  }

  if (branch_find_slice(STEndOfIntro,adapter)!=no_slice)
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

/* Instrument a branch with STEndOfBranchForced slices (typically for a
 * (semi-)reflex stipulation)
 * @param si root of branch to be instrumented
 * @param forced identifies branch that needs to be guarded from
 */
void battle_branch_insert_end_of_branch_forced(slice_index si,
                                               slice_index forced)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",forced);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  TraceStipulation(forced);

  {
    slice_index const ready = branch_find_slice(STReadyForDefense,si);
    assert(ready!=no_slice);
    pipe_append(ready,alloc_end_of_branch_forced(forced));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a battle branch with STConstraint slices (typically for a reflex
 * stipulation)
 * @param si entry slice of branch to be instrumented
 * @param constraint identifies branch that constrains the attacker
 */
void battle_branch_insert_attack_constraint(slice_index si,
                                            slice_index constraint)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",constraint);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  TraceStipulation(constraint);

  {
    slice_index const ready = branch_find_slice(STReadyForAttack,si);
    slice_index const prototype = alloc_constraint_slice(constraint);
    assert(ready!=no_slice);
    battle_branch_insert_slices(ready,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a battle branch with STConstraint slices (typically for a reflex
 * stipulation)
 * @param si entry slice of branch to be instrumented
 * @param constraint identifies branch that constrains the attacker
 */
void battle_branch_insert_defense_constraint(slice_index si,
                                            slice_index constraint)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",constraint);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  TraceStipulation(constraint);

  {
    slice_index const ready = branch_find_slice(STReadyForDefense,si);
    slice_index const prototype = alloc_constraint_slice(constraint);
    assert(ready!=no_slice);
    battle_branch_insert_slices(ready,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a branch with slices dealing with direct play
 * @param si root of branch to be instrumented
 * @param goal identifies slice leading towards goal
 */
void battle_branch_insert_direct_end_of_branch_goal(slice_index si,
                                                    slice_index goal)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  TraceStipulation(goal);

  {
    slice_index const ready = branch_find_slice(STReadyForAttack,si);
    slice_index const prototype = alloc_end_of_branch_goal(goal);
    assert(ready!=no_slice);
    battle_branch_insert_slices(ready,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a branch with slices dealing with direct play
 * @param si root of branch to be instrumented
 * @param next identifies slice leading towards goal
 */
void battle_branch_insert_direct_end_of_branch(slice_index si, slice_index next)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",next);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  TraceStipulation(next);

  {
    slice_index const ready = branch_find_slice(STReadyForAttack,si);
    slice_index const prototype = alloc_end_of_branch_slice(next);
    assert(ready!=no_slice);
    battle_branch_insert_slices(ready,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a branch with slices dealing with self play
 * @param si root of branch to be instrumented
 * @param next identifies slice leading towards goal
 */
void battle_branch_insert_self_end_of_branch(slice_index si, slice_index goal)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  TraceStipulation(goal);

  {
    slice_index const ready = branch_find_slice(STReadyForDefense,si);
    slice_index const prototype = alloc_end_of_branch_slice(goal);
    assert(ready!=no_slice);
    battle_branch_insert_slices(ready,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a branch for detecting whether the defense was forced to reach a
 * goal
 * @param si root of branch to be instrumented
 * @param goal identifies slice leading towards goal
 */
void battle_branch_insert_self_end_of_branch_goal(slice_index si,
                                                  slice_index goal)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  TraceStipulation(goal);

  {
    slice_index const ready = branch_find_slice(STReadyForDefense,si);
    slice_index const prototype = alloc_end_of_branch_goal(goal);
    assert(ready!=no_slice);
    battle_branch_insert_slices(ready,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
