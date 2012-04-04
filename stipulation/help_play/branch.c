#include "stipulation/help_play/branch.h"
#include "pypipe.h"
#include "stipulation/conditional_pipe.h"
#include "stipulation/constraint.h"
#include "stipulation/branch.h"
#include "stipulation/end_of_branch.h"
#include "stipulation/end_of_branch_goal.h"
#include "stipulation/dead_end.h"
#include "stipulation/dummy_move.h"
#include "stipulation/move_played.h"
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
  STResetUnsolvable,
  STConstraintSolver,
  STConstraintTester,
  STFindByIncreasingLength,
  STFindShortest,
  STStopOnShortSolutionsFilter,
  STIntelligentMovesLeftInitialiser,
  STRestartGuardIntelligent,
  STIntelligentMateFilter,
  STIntelligentStalemateFilter,
  STIntelligentProof,
  STIntelligentTargetCounter,
  STEndOfBranch,
  STEndOfBranchForced,
  STEndOfBranchTester,
  STDeadEnd,
  STEndOfIntro,

  STReadyForHelpMove,
  STReadyForDummyMove,
  STHelpHashed,
  STCheckZigzagJump,
  STTestingPrerequisites,
  STDoubleMateFilter,
  STCounterMateFilter,
  STEnPassantFilter,
  STCastlingFilter,
  STPrerequisiteOptimiser,
  STDeadEnd,
  STGeneratingMoves,
  STMoveGenerator,
  STOrthodoxMatingMoveGenerator,
  STKingMoveGenerator,
  STSingleMoveGeneratorWithKingCapture,
  STSinglePieceMoveGenerator,
  STCastlingIntermediateMoveGenerator,
  STSingleMoveGenerator,
  STOhneschachSuspender,
  STExclusiveChessUnsuspender,
  STMove,
  STForEachMove,
  STFindMove,
  STBGLFilter,
  STMoveTracer,
  STHelpMovePlayed,
  STDummyMove,
  STCaptureCounter,
  STAnyMoveCounter,
  STOpponentMovesCounter,
  STMaxTimeGuard,
  STMaxSolutionsGuard,
  STMaxSolutionsCounter,
  STRestartGuard,
  STKeepMatingFilter,
  STIntelligentLimitNrSolutionsPerTargetPos,
  STGoalReachableGuardFilterMate,
  STGoalReachableGuardFilterStalemate,
  STGoalReachableGuardFilterProof,
  STGoalReachableGuardFilterProofFairy,
  STEndOfRoot,

  STEndOfBranchGoal,
  STEndOfBranchGoalImmobile,
  STEndOfBranchGoalTester,
  STNotEndOfBranchGoal,
  STDeadEndGoal,
  STSelfCheckGuard,
  STOhneschachCheckGuard,
  STLegalMoveCounter,
  STIntelligentImmobilisationCounter,
  STFlightsquaresCounter,
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

static boolean insert_before(slice_index si,
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
  stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_visit_pipe(slice_index si, stip_structure_traversal *st)
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
    else if (insert_before(si,rank,state))
      ; /* nothing - work is done*/
    else
    {
      state->base = rank;
      state->prev = si;
      stip_traverse_structure_children(si,st);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_visit_fork(slice_index si, stip_structure_traversal *st)
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
    else if (insert_before(si,rank,state))
      ; /* nothing - work is done*/
    else
    {
      state->base = rank;
      state->prev = si;
      stip_traverse_structure_children_pipe(si,st);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_visit_leaf(slice_index si, stip_structure_traversal *st)
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
    insert_before(si,rank,state);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_visit_binary_operands(slice_index si,
                                         stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;
  insertion_state_type const save_state = *state;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",state->nr_prototypes);
  TraceFunctionParam("%u",state->base);
  TraceFunctionParam("%u",state->prev);
  TraceFunctionParamListEnd();

  if (slices[si].u.binary.op1!=no_slice)
    insert_visit_proxy(si,st);

  *state = save_state;

  if (slices[si].u.binary.op2!=no_slice)
  {
    assert(slices[slices[si].u.binary.op2].type==STProxy);
    insert_visit_proxy(slices[si].u.binary.op2,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_visit_binary(slice_index si,
                                stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",state->nr_prototypes);
  TraceFunctionParam("%u",state->base);
  TraceFunctionParam("%u",state->prev);
  TraceFunctionParamListEnd();

  {
    insertion_state_type * const state = st->param;
    unsigned int const rank = get_slice_rank(slices[si].type,state->base);
    if (rank==no_slice_rank)
      insert_visit_binary_operands(si,st);
    else if (insert_before(si,rank,state))
      ; /* nothing - work is done*/
    else
    {
      state->base = rank+1;
      insert_visit_binary_operands(si,st);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void start_insertion_traversal(slice_index si,
                                      insertion_state_type *state)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",state->base);
  TraceFunctionParam("%u",state->nr_prototypes);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,state);
  st.context = stip_traversal_context_help;
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_leaf,
                                                 &insert_visit_leaf);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_pipe,
                                                 &insert_visit_pipe);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_branch,
                                                 &insert_visit_pipe);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_fork,
                                                 &insert_visit_fork);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_binary,
                                                 &insert_visit_binary);
  stip_structure_traversal_override_single(&st,STProxy,&insert_visit_proxy);
  stip_traverse_structure(slices[si].u.pipe.next,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void help_branch_insert_slices_impl(slice_index si,
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

/* Insert slices into a help branch; the elements of
 * prototypes are *not* deallocated by help_branch_insert_slices_nested().
 * The inserted slices are copies of the elements of prototypes).
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param adapter identifies starting point of insertion (of type STHelpAdapter)
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void help_branch_insert_slices_nested(slice_index adapter,
                                      slice_index const prototypes[],
                                      unsigned int nr_prototypes)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  assert(slices[adapter].type==STHelpAdapter);
  help_branch_insert_slices_impl(adapter,prototypes,nr_prototypes);

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

  help_branch_insert_slices_impl(si,prototypes,nr_prototypes);

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

  stip_traverse_structure_children_pipe(si,st);
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

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_pipe,
                                                 &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_branch,
                                                 &increase_min_length_branch);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_fork,
                                                 &stip_traverse_structure_children_pipe);
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
  if (slices[adapter].u.branch.min_length<slack_length)
    increase_min_length(adapter);
  branch_shorten_slices(next,STHelpAdapter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Remember the goal imminent after a defense or attack move
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void and_immobility(slice_index si, stip_structure_traversal *st)
{
  boolean * const goal_implies_immobility = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure(slices[si].u.binary.op1,st);
  if (*goal_implies_immobility)
  {
    *goal_implies_immobility = false;
    stip_traverse_structure(slices[si].u.binary.op2,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Remember the goal imminent after a defense or attack move
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void or_immobility(slice_index si, stip_structure_traversal *st)
{
  boolean * const goal_implies_immobility = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure(slices[si].u.binary.op1,st);
  if (!*goal_implies_immobility)
    stip_traverse_structure(slices[si].u.binary.op2,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether there are more moves after this branch
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void remember_immobility(slice_index si, stip_structure_traversal *st)
{
  boolean * const goal_implies_immobility = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (slices[si].u.goal_filter.applies_to_who==goal_applies_to_starter)
    *goal_implies_immobility = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Forget a remembered goal because it is to be reached by a move
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void forget_immobility(slice_index si, stip_structure_traversal *st)
{
  boolean * const goal_implies_immobility = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  *goal_implies_immobility = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors const end_of_branch_goal_immobility_detectors[] =
{
  { STAttackAdapter,             &forget_immobility   },
  { STDefenseAdapter,            &forget_immobility   },
  { STHelpAdapter,               &forget_immobility   },
  { STGoalImmobileReachedTester, &remember_immobility },
  { STOr,                        &and_immobility      },
  { STAnd,                       &or_immobility       }
};

enum
{
  nr_end_of_branch_goal_immobility_detectors = (sizeof end_of_branch_goal_immobility_detectors
                                                / sizeof end_of_branch_goal_immobility_detectors[0])
};

/* Detect whether a goal implies immobility of the "goaled" side
 * @param si identifies entry slice to the goal testing machinery
 * @return true iff the goal implies immobility
 */
static boolean does_goal_imply_immobility(slice_index si)
{
  boolean result = false;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&result);
  stip_structure_traversal_override(&st,
                                    end_of_branch_goal_immobility_detectors,
                                    nr_end_of_branch_goal_immobility_detectors);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find a STReadyMove slice with a specific parity
 * @param si identifies the entry slice of a help branch
 * @param parity indicates after which help move of the branch to insert
 * @return identifier of found STReadyMove slice; no_slice if no such slice was found
 */
slice_index help_branch_locate_ready(slice_index si, unsigned int parity)
{
  slice_index result = si;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",parity);
  TraceFunctionParamListEnd();

  result = branch_find_slice(STReadyForHelpMove,result);
  assert(result!=no_slice);

  while ((slices[result].u.branch.length-slack_length)%2!=parity%2)
  {
    slice_index const next = branch_find_slice(STReadyForHelpMove,result);
    assert(next!=no_slice);
    if (result==next)
    {
      result = no_slice;
      break;
    }
    else
      result = next;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert a slice marking the end of the branch
 * @param si identifies the entry slice of a help branch
 * @param end_proto end of branch prototype slice
 * @param parity indicates after which help move of the branch to insert
 * @return true iff the end could be inserted
 */
static boolean help_branch_insert_end_of_branch(slice_index si,
                                                slice_index end_proto,
                                                unsigned int parity)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",end_proto);
  TraceFunctionParam("%u",parity);
  TraceFunctionParamListEnd();

  {
    slice_index const pos = help_branch_locate_ready(si,parity);
    if (pos==no_slice)
      result = false;
    else
    {
      help_branch_insert_slices(pos,&end_proto,1);
      result = true;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
  slice_index branch;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",to_goal);
  TraceFunctionParam("%u",parity);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  TraceStipulation(to_goal);

  if (does_goal_imply_immobility(to_goal))
    branch = alloc_end_of_branch_goal_immobile(to_goal);
  else
    branch = alloc_end_of_branch_goal(to_goal);

  {
    boolean const inserted = help_branch_insert_end_of_branch(si,branch,parity);
    assert(inserted);
  }

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

  {
    slice_index const fork = alloc_end_of_branch_forced(forced);
    boolean const inserted = help_branch_insert_end_of_branch(si,fork,parity);
    assert(inserted);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a series branch with STConstraint* slices (typically for a hr
 * stipulation)
 * @param si entry slice of branch to be instrumented
 * @param constraint identifies branch that constrains the attacker
 * @param parity indicates after which help move of the branch to insert
 * @return true iff the constraint could be inserted
 * @note deallocates the constraint if couldn't be inserted
 */
boolean help_branch_insert_constraint(slice_index si,
                                      slice_index constraint,
                                      unsigned int parity)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",constraint);
  TraceFunctionParam("%u",parity);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  TraceStipulation(constraint);

  {
    slice_index const tester = alloc_constraint_tester_slice(constraint);
    if (help_branch_insert_end_of_branch(si,tester,parity))
      result = true;
    else
    {
      result = false;
      dealloc_slices(tester);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
    slice_index const testpre1 = alloc_pipe(STTestingPrerequisites);
    slice_index const generating1 = alloc_pipe(STGeneratingMoves);
    slice_index const move1 = alloc_pipe(STMove);
    slice_index const played1 = alloc_help_move_played_slice();
    slice_index const not_end_goal1 = alloc_pipe(STNotEndOfBranchGoal);
    slice_index const ready2 = alloc_branch(STReadyForHelpMove,
                                            length-1,min_length-1);
    slice_index const testpre2 = alloc_pipe(STTestingPrerequisites);
    slice_index const generating2 = alloc_pipe(STGeneratingMoves);
    slice_index const move2 = alloc_pipe(STMove);
    slice_index const played2 = alloc_help_move_played_slice();
    slice_index const not_end_goal2 = alloc_pipe(STNotEndOfBranchGoal);

    slice_index const deadend = alloc_dead_end_slice();

    pipe_link(adapter,ready1);
    pipe_link(ready1,testpre1);
    pipe_link(testpre1,generating1);
    pipe_link(generating1,move1);
    pipe_link(move1,played1);
    pipe_link(played1,not_end_goal1);
    pipe_link(not_end_goal1,ready2);
    pipe_link(ready2,testpre2);
    pipe_link(testpre2,generating2);
    pipe_link(generating2,move2);
    pipe_link(move2,played2);
    pipe_link(played2,not_end_goal2);
    pipe_link(not_end_goal2,adapter);

    if ((length-slack_length)%2==0)
      help_branch_insert_slices(adapter,&deadend,1);
    else
      help_branch_insert_slices(move1,&deadend,1);

    result = adapter;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void serve_as_root_hook(slice_index si, stip_structure_traversal *st)
{
  spin_off_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->spun_off[si] = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void fork_make_root(slice_index si, stip_structure_traversal *st)
{
  spin_off_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);
  TraceValue("%u\n",state->spun_off[slices[si].u.pipe.next]);

  if (state->spun_off[slices[si].u.pipe.next]!=no_slice)
  {
    state->spun_off[si] = copy_slice(si);
    link_to_branch(state->spun_off[si],state->spun_off[slices[si].u.pipe.next]);
  }

  TraceValue("%u\n",state->spun_off[si]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Create the root slices of a help branch
 * @param adapter identifies the adapter slice at the beginning of the branch
 * @param state address of structure holding state
 */
void help_branch_make_root_slices(slice_index adapter,
                                  spin_off_state_type *state)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st;

    TraceStipulation(adapter);

    stip_structure_traversal_init(&st,state);
    stip_structure_traversal_override_by_structure(&st,
                                                   slice_structure_pipe,
                                                   &pipe_spin_off_copy);
    stip_structure_traversal_override_by_structure(&st,
                                                   slice_structure_branch,
                                                   &pipe_spin_off_copy);
    stip_structure_traversal_override_by_structure(&st,
                                                   slice_structure_binary,
                                                   &binary_make_root);
    stip_structure_traversal_override_by_structure(&st,
                                                   slice_structure_fork,
                                                   &fork_make_root);
    stip_structure_traversal_override_by_function(&st,
                                                  slice_function_conditional_pipe,
                                                  &conditional_pipe_spin_off_copy);
    stip_structure_traversal_override_single(&st,
                                             STConstraintTester,
                                             &constraint_tester_make_root);
    stip_structure_traversal_override_single(&st,STEndOfRoot,&serve_as_root_hook);
    stip_traverse_structure(adapter,&st);
  }

  TraceValue("%u\n",state->spun_off[adapter]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Wrap the slices representing the initial moves of the solution with
 * slices of appropriately equipped slice types
 * @param adapter identifies the adapter slice at the beginning of the branch
 * @param state address of structure holding state
 */
void help_make_root(slice_index adapter, spin_off_state_type *state)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  assert(slices[adapter].type==STHelpAdapter);

  {
    slice_index const prototype = alloc_pipe(STEndOfRoot);
    help_branch_insert_slices(adapter,&prototype,1);
    help_branch_make_root_slices(adapter,state);
    branch_shorten_slices(adapter,STEndOfRoot);
    pipe_remove(adapter);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Spin the intro slices off a nested help branch
 * @param adapter identifies adapter slice of the nested help branch
 * @param state address of structure holding state
 */
void help_spin_off_intro(slice_index adapter, spin_off_state_type *state)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  assert(slices[adapter].type==STHelpAdapter);

  TraceStipulation(adapter);

  {
    slice_index const prototype = alloc_pipe(STEndOfIntro);
    help_branch_insert_slices(adapter,&prototype,1);
  }

  {
    slice_index const next = slices[adapter].u.pipe.next;
    stip_structure_traversal st;

    stip_structure_traversal_init(&st,state);
    stip_structure_traversal_override_by_structure(&st,
                                                   slice_structure_pipe,
                                                   &pipe_spin_off_copy);
    stip_structure_traversal_override_by_structure(&st,
                                                   slice_structure_branch,
                                                   &pipe_spin_off_copy);
    stip_structure_traversal_override_by_structure(&st,
                                                   slice_structure_fork,
                                                   &pipe_spin_off_copy);
    stip_structure_traversal_override_by_structure(&st,
                                                   slice_structure_binary,
                                                   &binary_make_root);
    stip_structure_traversal_override_by_function(&st,
                                                  slice_function_conditional_pipe,
                                                  &conditional_pipe_spin_off_copy);
    stip_structure_traversal_override_single(&st,
                                             STConstraintTester,
                                             &constraint_tester_make_root);
    stip_structure_traversal_override_single(&st,STEndOfIntro,&serve_as_root_hook);
    stip_traverse_structure(next,&st);

    pipe_link(slices[adapter].prev,next);
    link_to_branch(adapter,state->spun_off[next]);
    state->spun_off[adapter] = state->spun_off[next];
    slices[adapter].prev = no_slice;
  }

  TraceValue("%u\n",state->spun_off[adapter]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Produce slices representing set play.
 * @param adapter identifies the adapter slice at the beginning of the branch
 * @param state address of structure holding state
 */
void help_branch_make_setplay(slice_index adapter, spin_off_state_type *state)
{
  stip_length_type const length = slices[adapter].u.branch.length;
  stip_length_type min_length = slices[adapter].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  assert(slices[adapter].type==STHelpAdapter);

  if (min_length==slack_length)
    min_length += 2;

  if (length>slack_length+1)
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
      help_branch_make_root_slices(set_adapter,state);
      state->spun_off[adapter] = state->spun_off[set_adapter];
      pipe_remove(set_adapter);
    }
  }

  TraceValue("%u\n",state->spun_off[adapter]);

  TraceFunctionExit(__func__);
  TraceFunctionParamListEnd();
}

/* Allocate a series branch
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of entry slice into allocated series branch
 */
slice_index alloc_series_branch(stip_length_type length,
                                stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  {
    slice_index const adapter = alloc_help_adapter_slice(length,min_length);
    slice_index const ready = alloc_branch(STReadyForHelpMove,
                                           length,min_length);
    slice_index const testpre = alloc_pipe(STTestingPrerequisites);
    slice_index const generating = alloc_pipe(STGeneratingMoves);
    slice_index const move = alloc_pipe(STMove);
    slice_index const played = alloc_help_move_played_slice();
    slice_index const not_end_goal = alloc_pipe(STNotEndOfBranchGoal);
    slice_index const deadend = alloc_dead_end_slice();
    slice_index const ready2 = alloc_pipe(STReadyForDummyMove);
    slice_index const dummy = alloc_dummy_move_slice();

    result = adapter;

    pipe_link(adapter,ready);
    pipe_link(ready,testpre);
    pipe_link(testpre,generating);
    pipe_link(generating,move);
    pipe_link(move,played);
    pipe_link(played,not_end_goal);
    pipe_link(not_end_goal,deadend);
    pipe_link(deadend,ready2);
    pipe_link(ready2,dummy);
    pipe_link(dummy,adapter);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Produce slices representing set play.
 * @param adapter identifies the adapter slice at the beginning of the branch
 * @param state address of structure holding state
 */
void series_branch_make_setplay(slice_index adapter, spin_off_state_type *state)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  {
    slice_index const next = slices[adapter].u.pipe.next;
    slice_index const prototypes[] =
    {
      alloc_help_adapter_slice(slack_length,slack_length),
      alloc_pipe(STEndOfRoot)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    help_branch_insert_slices(next,prototypes,nr_prototypes);

    {
      slice_index const set_adapter = branch_find_slice(STHelpAdapter,next);
      assert(set_adapter!=no_slice);
      if (slices[slices[set_adapter].u.pipe.next].type==STDeadEnd)
        ; /* set play not applicable */
      else
        help_branch_make_root_slices(set_adapter,state);
      TraceValue("%u\n",state->spun_off[set_adapter]);
      state->spun_off[adapter] = state->spun_off[set_adapter];
      pipe_remove(set_adapter);
    }
  }

  TraceValue("%u\n",state->spun_off[adapter]);

  TraceFunctionExit(__func__);
  TraceFunctionParamListEnd();
}

/* Instrument a series branch with STConstraint* slices (typically for a ser-r
 * stipulation)
 * @param si entry slice of branch to be instrumented
 * @param constraint identifies branch that constrains the attacker
 */
void series_branch_insert_constraint(slice_index si, slice_index constraint)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",constraint);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  TraceStipulation(constraint);

  assert(slices[constraint].type==STProxy);

  {
    slice_index const prototype = alloc_constraint_tester_slice(constraint);
    branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
