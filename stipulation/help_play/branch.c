#include "stipulation/help_play/branch.h"
#include "stipulation/pipe.h"
#include "stipulation/fork.h"
#include "stipulation/proxy.h"
#include "stipulation/conditional_pipe.h"
#include "stipulation/constraint.h"
#include "stipulation/branch.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/move.h"
#include "stipulation/move_played.h"
#include "stipulation/binary.h"
#include "stipulation/help_play/adapter.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

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
  STIntelligentFilter,
  STMaxTimeGuard,
  STMaxSolutionsGuard,
  STIntelligentFlightsGuarder,
  STMaxTimeGuard,
  STMaxSolutionsGuard,
  STIntelligentFlightsBlocker,
  STIntelligentProof,
  STMaxTimeGuard,
  STMaxSolutionsGuard,
  STIntelligentMateTargetPositionTester,
  STIntelligentStalemateTargetPositionTester,
  STIntelligentSolutionsPerTargetPosInitialiser,
  STIntelligentTargetCounter,
  STIntelligentTargetPositionFound,
  STHashOpener,
  STEndOfBranch,
  STEndOfBranchForced,
  STEndOfBranchTester,
  STDeadEnd,
  STEndOfIntro,

  STReadyForHelpMove,
  STHelpHashed,
  STIfThenElse,
  STTestingPrerequisites,
  STDoubleMateFilter,
  STCounterMateFilter,
  STEnPassantFilter,
  STCastlingFilter,
  STPrerequisiteOptimiser,
  STDeadEnd,
  STGeneratingMoves,
  STTotalInvisibleMoveGeneratorInitialiser,
  STExecutingKingCapture,
  STExclusiveChessExclusivityDetector,
  STExclusiveChessNestedExclusivityDetector,
  STMoveGenerator,
  STTotalInvisibleMoveGeneratorUninitialiser,
  STBlackChecksNullMoveGenerator,
  STOrthodoxMatingMoveGenerator,
  STOrthodoxMatingKingContactGenerator,
  STKingMoveGenerator,
  STSinglePieceMoveGenerator,
  STSkipMoveGeneration,
  STDoneGeneratingMoves,
  STSuperTransmutingKingMoveGenerationFilter,
  STUncapturableRemoveCaptures,
  STBackhomeExistanceTester,
  STBackhomeRemoveIllegalMoves,
  STPiecesParalysingRemoveCaptures,
  STNocaptureRemoveCaptures,
  STWoozlesRemoveIllegalCaptures,
  STBiWoozlesRemoveIllegalCaptures,
  STHeffalumpsRemoveIllegalCaptures,
  STBiHeffalumpsRemoveIllegalCaptures,
  STNorskRemoveIllegalCaptures,
  STProvocateursRemoveUnobservedCaptures,
  STLortapRemoveSupportedCaptures,
  STPatrolRemoveUnsupportedCaptures,
  STTakeAndMakeGenerateMake,
  STTakeAndMakeAvoidPawnMakeToBaseLine,
  STImitatorRemoveIllegalMoves,
  STGridRemoveIllegalMoves,
  STMonochromeRemoveNonMonochromeMoves,
  STBichromeRemoveNonBichromeMoves,
  STEdgeMoverRemoveIllegalMoves,
  STWormholeRemoveIllegalCaptures,
  STBGLEnforcer,
  STShieldedKingsRemoveIllegalCaptures,
  STSuperguardsRemoveIllegalCaptures,
  STMummerOrchestrator,
  STDoneRemovingIllegalMoves,
  STEnPassantRemoveNonReachers,
  STCastlingRemoveNonReachers,
  STChess81RemoveNonReachers,
  STCaptureRemoveNonReachers,
  STTargetRemoveNonReachers,
  STCageCirceFutileCapturesRemover,
  STDoneRemovingFutileMoves,
  STDonePriorisingMoves,
  STTakeMakeCirceCollectRebirthSquares,
  STMove,
  STHelpMovePlayed,
  STCaptureCounter,
  STAnyMoveCounter,
  STMaxTimeGuard,
  STMaxSolutionsGuard,
  STRestartGuardNested,
  STRestartGuard,
  STKeepMatingFilter,
  STIntelligentLimitNrSolutionsPerTargetPosLimiter,
  STGoalReachableGuardFilterMate,
  STGoalReachableGuardFilterStalemate,
  STGoalReachableGuardFilterProof,
  STGoalReachableGuardFilterProofFairy,
  STEndOfRoot,

  STEndOfBranchGoal,
  STEndOfBranchGoalImmobile,
  STEndOfBranchGoalTester,
  STGoalReachedTester,
  STNotEndOfBranchGoal,
  STDeadEndGoal,
  STSelfCheckGuard,
  STOhneschachStopIfCheck,
  STOhneschachStopIfCheckAndNotMate,
  STMummerDeadend,
  STIntelligentSolutionsPerTargetPosCounter,
  STIntelligentDuplicateAvoider,
  STIntelligentSolutionRememberer,
  STExclusiveChessGoalReachingMoveCounter,
  STLegalAttackCounter,
  STLegalDefenseCounter,
  STMaxSolutionsCounter,
  STGoalConstraintTester,
  STOutputPlaintextLineLineWriter,
  STOutputLaTeXLineLineWriter,
  STOhneschachDetectUndecidableGoal,
  STOutputPlaintextGoalWriter,
  STOutputLaTeXGoalWriter,
  STTrue,
  STIntelligentImmobilisationCounter,
  STFlightsquaresCounter,
  STCheckZigzagLanding
};

enum
{
  nr_slice_rank_order_elmts = (sizeof slice_rank_order
                               / sizeof slice_rank_order[0])
};

static void help_branch_insert_slices_impl(slice_index si,
                                           slice_index const prototypes[],
                                           unsigned int nr_prototypes,
                                           slice_index base)
{
  stip_structure_traversal st;
  branch_slice_insertion_state_type state =
  {
      prototypes, nr_prototypes,
      slice_rank_order, nr_slice_rank_order_elmts, 0,
      branch_slice_rank_order_recursive,
      0,
      si,
      0
  };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  slice_insertion_init_traversal(&st,&state,stip_traversal_context_help);
  move_init_slice_insertion_traversal(&st);

  state.base_rank = get_slice_rank(SLICE_TYPE(base),&state);

  stip_traverse_structure(si,&st);

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

  assert(SLICE_TYPE(adapter)==STHelpAdapter);
  help_branch_insert_slices_impl(adapter,prototypes,nr_prototypes,adapter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Like help_branch_insert_slices, but starting at a proxy slice
 * @param base used instead of si for determining the current position in the
 *             sequence of defense branches
 */
void help_branch_insert_slices_behind_proxy(slice_index si,
                                            slice_index const prototypes[],
                                            unsigned int nr_prototypes,
                                            slice_index base)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  help_branch_insert_slices_impl(si,prototypes,nr_prototypes,base);
  deallocate_slice_insertion_prototypes(prototypes,nr_prototypes);

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
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  help_branch_insert_slices_impl(si,prototypes,nr_prototypes,si);
  deallocate_slice_insertion_prototypes(prototypes,nr_prototypes);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void increase_min_length_branch(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);
  SLICE_U(si).branch.min_length += 2;

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
  slice_index const next = SLICE_NEXT1(adapter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  assert(SLICE_TYPE(adapter)==STHelpAdapter);

  {
    /* find the new spot for adapter by inserting a copy */
    slice_index const prototype = copy_slice(adapter);
    help_branch_insert_slices(next,&prototype,1);
  }

  {
    /* move adapter to its new spot */
    slice_index const copy = branch_find_slice(STHelpAdapter,
                                               next,
                                               stip_traversal_context_help);
    assert(copy!=no_slice);
    pipe_link(SLICE_PREV(adapter),next);
    pipe_append(copy,adapter);
    pipe_remove(copy);
  }

  /* adjust the length and min_length members */
  --SLICE_U(adapter).branch.length;
  if (SLICE_U(adapter).branch.min_length<=0)
    increase_min_length(adapter);
  --SLICE_U(adapter).branch.min_length;
  branch_shorten_slices(next,STHelpAdapter,stip_traversal_context_help);

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

  stip_traverse_structure_binary_operand1(si,st);
  if (*goal_implies_immobility)
  {
    *goal_implies_immobility = false;
    stip_traverse_structure_binary_operand2(si,st);
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

  stip_traverse_structure_binary_operand1(si,st);
  if (!*goal_implies_immobility)
    stip_traverse_structure_binary_operand2(si,st);

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

  if (SLICE_U(si).goal_filter.applies_to_who==goal_applies_to_starter)
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

  stip_traverse_structure_children_pipe(si,st);
  *goal_implies_immobility = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor const end_of_branch_goal_immobility_detectors[] =
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

/* Find a STHelpMovePlayed slice with a specific parity
 * @param si identifies the entry slice of a help branch
 * @param parity indicates after which help move of the branch to insert
 * @return identifier of found STHelpMovePlayed slice; no_slice if no such slice was found
 */
static slice_index help_branch_locate_played(slice_index si, unsigned int parity)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",parity);
  TraceFunctionParamListEnd();

  {
    slice_index const ready = branch_find_slice(STReadyForHelpMove,si,stip_traversal_context_help);
    slice_index const played1 = branch_find_slice(STHelpMovePlayed,ready,stip_traversal_context_help);
    slice_index const played2 = branch_find_slice(STHelpMovePlayed,played1,stip_traversal_context_help);
    assert(ready!=no_slice);
    assert(played1!=no_slice);
    assert(played2!=no_slice);

    if (SLICE_U(ready).branch.length%2==parity%2)
      result = played1;
    else
      result = played2;
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
    slice_index const pos = help_branch_locate_played(si,parity);
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

/* Insert a fork to the next1 branch
 * @param si identifies the entry slice of a help branch
 * @param next1 identifies the entry slice of the next1 branch
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

  help_branch_insert_end_of_branch(si,alloc_fork_slice(STEndOfBranch,next),parity);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a fork to the goal branch
 * @param si identifies the entry slice of a help branch
 * @param next1 identifies the entry slice of the next1 branch
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
    branch = alloc_fork_slice(STEndOfBranchGoalImmobile,to_goal);
  else
    branch = alloc_fork_slice(STEndOfBranchGoal,to_goal);

  {
#if !defined(NDEBUG)
    boolean const inserted =
#endif
    help_branch_insert_end_of_branch(si,branch,parity);
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
    slice_index const proxy = alloc_proxy_slice();
    slice_index const fork = alloc_fork_slice(STEndOfBranchForced,proxy);
#if !defined(NDEBUG)
    boolean const inserted =
#endif
    help_branch_insert_end_of_branch(si,fork,parity);
    assert(inserted);
    pipe_link(proxy,forced);
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
    slice_index const done_generating1 = alloc_pipe(STDoneGeneratingMoves);
    slice_index const done_removing_illegal1 = alloc_pipe(STDoneRemovingIllegalMoves);
    slice_index const done_removing_futile1 = alloc_pipe(STDoneRemovingFutileMoves);
    slice_index const done_priorising1 = alloc_pipe(STDonePriorisingMoves);
    slice_index const move1 = alloc_pipe(STMove);
    slice_index const played1 = alloc_help_move_played_slice();
    slice_index const not_end_goal1 = alloc_pipe(STNotEndOfBranchGoal);
    slice_index const ready2 = alloc_branch(STReadyForHelpMove,
                                            length-1,min_length-1);
    slice_index const testpre2 = alloc_pipe(STTestingPrerequisites);
    slice_index const generating2 = alloc_pipe(STGeneratingMoves);
    slice_index const done_generating2 = alloc_pipe(STDoneGeneratingMoves);
    slice_index const done_removing_illegal2 = alloc_pipe(STDoneRemovingIllegalMoves);
    slice_index const done_removing_futile2 = alloc_pipe(STDoneRemovingFutileMoves);
    slice_index const done_priorising2 = alloc_pipe(STDonePriorisingMoves);
    slice_index const move2 = alloc_pipe(STMove);
    slice_index const played2 = alloc_help_move_played_slice();
    slice_index const not_end_goal2 = alloc_pipe(STNotEndOfBranchGoal);

    slice_index const deadend = alloc_pipe(STDeadEnd);

    pipe_link(adapter,ready1);
    pipe_link(ready1,testpre1);
    pipe_link(testpre1,generating1);
    pipe_link(generating1,done_generating1);
    pipe_link(done_generating1,done_removing_illegal1);
    pipe_link(done_removing_illegal1,done_removing_futile1);
    pipe_link(done_removing_futile1,done_priorising1);
    pipe_link(done_priorising1,move1);
    pipe_link(move1,played1);
    pipe_link(played1,not_end_goal1);
    pipe_link(not_end_goal1,ready2);
    pipe_link(ready2,testpre2);
    pipe_link(testpre2,generating2);
    pipe_link(generating2,done_generating2);
    pipe_link(done_generating2,done_removing_illegal2);
    pipe_link(done_removing_illegal2,done_removing_futile2);
    pipe_link(done_removing_futile2,done_priorising2);
    pipe_link(done_priorising2,move2);
    pipe_link(move2,played2);
    pipe_link(played2,not_end_goal2);
    pipe_link(not_end_goal2,adapter);

    if (length%2==0)
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
  TraceValue("%u",state->spun_off[SLICE_NEXT1(si)]);
  TraceEOL();

  if (state->spun_off[SLICE_NEXT1(si)]!=no_slice)
  {
    state->spun_off[si] = copy_slice(si);
    link_to_branch(state->spun_off[si],state->spun_off[SLICE_NEXT1(si)]);
  }

  TraceValue("%u",state->spun_off[si]);
  TraceEOL();

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
    stip_structure_traversal_override_by_contextual(&st,
                                                    slice_contextual_binary,
                                                    &binary_make_root);
    stip_structure_traversal_override_by_structure(&st,
                                                   slice_structure_fork,
                                                   &fork_make_root);
    stip_structure_traversal_override_by_contextual(&st,
                                                    slice_contextual_conditional_pipe,
                                                    &conditional_pipe_spin_off_copy);
    stip_structure_traversal_override_single(&st,
                                             STConstraintTester,
                                             &constraint_tester_make_root);
    stip_structure_traversal_override_single(&st,STEndOfRoot,&serve_as_root_hook);
    stip_traverse_structure(adapter,&st);
  }

  TraceValue("%u",state->spun_off[adapter]);
  TraceEOL();

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

  assert(SLICE_TYPE(adapter)==STHelpAdapter);

  {
    slice_index const prototype = alloc_pipe(STEndOfRoot);
    slice_insertion_insert(adapter,&prototype,1);
    help_branch_make_root_slices(adapter,state);
    branch_shorten_slices(adapter,STEndOfRoot,stip_traversal_context_intro);
    pipe_remove(adapter);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Produce slices representing set play.
 * @param adapter identifies the adapter slice at the beginning of the branch
 * @param state address of structure holding state
 */
void help_branch_make_setplay(slice_index adapter, spin_off_state_type *state)
{
  stip_length_type const length = SLICE_U(adapter).branch.length;
  stip_length_type min_length = SLICE_U(adapter).branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  assert(SLICE_TYPE(adapter)==STHelpAdapter);

  if (min_length==0)
    min_length = 2;

  if (length>1)
  {
    slice_index const next = SLICE_NEXT1(adapter);
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
      slice_index const set_adapter = branch_find_slice(STHelpAdapter,
                                                        next,
                                                        stip_traversal_context_help);
      assert(set_adapter!=no_slice);
      help_branch_make_root_slices(set_adapter,state);
      state->spun_off[adapter] = state->spun_off[set_adapter];
      pipe_remove(set_adapter);
    }
  }

  TraceValue("%u",state->spun_off[adapter]);
  TraceEOL();

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
    slice_index const done_generating = alloc_pipe(STDoneGeneratingMoves);
    slice_index const done_removing_illegal = alloc_pipe(STDoneRemovingIllegalMoves);
    slice_index const done_removing_futile = alloc_pipe(STDoneRemovingFutileMoves);
    slice_index const done_priorising = alloc_pipe(STDonePriorisingMoves);
    slice_index const move = alloc_pipe(STMove);
    slice_index const played1 = alloc_help_move_played_slice();
    slice_index const not_end_goal1 = alloc_pipe(STNotEndOfBranchGoal);
    slice_index const played2 = alloc_help_move_played_slice();
    slice_index const not_end_goal2 = alloc_pipe(STNotEndOfBranchGoal);

    slice_index const deadend = alloc_pipe(STDeadEnd);

    result = adapter;

    pipe_link(adapter,ready);
    pipe_link(ready,testpre);
    pipe_link(testpre,generating);
    pipe_link(generating,done_generating);
    pipe_link(done_generating,done_removing_illegal);
    pipe_link(done_removing_illegal,done_removing_futile);
    pipe_link(done_removing_futile,done_priorising);
    pipe_link(done_priorising,move);
    pipe_link(move,played1);
    pipe_link(played1,not_end_goal1);
    pipe_link(not_end_goal1,played2);
    pipe_link(played2,not_end_goal2);
    pipe_link(not_end_goal2,adapter);

    if (length%2==0)
      /* branch ends after the pseudo move */
      pipe_append(not_end_goal2,deadend);
    else
      /* branch ends after the real move */
      pipe_append(not_end_goal1,deadend);
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
    slice_index const next = SLICE_NEXT1(adapter);
    slice_index const prototypes[] =
    {
      alloc_help_adapter_slice(0,0),
      alloc_pipe(STEndOfRoot)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    help_branch_insert_slices(next,prototypes,nr_prototypes);

    {
      slice_index const set_adapter = branch_find_slice(STHelpAdapter,
                                                        next,
                                                        stip_traversal_context_help);
      assert(set_adapter!=no_slice);
      if (SLICE_TYPE(SLICE_NEXT1(set_adapter))==STDeadEnd)
        ; /* set play not applicable */
      else
        help_branch_make_root_slices(set_adapter,state);
      TraceValue("%u",state->spun_off[set_adapter]);
      TraceEOL();
      state->spun_off[adapter] = state->spun_off[set_adapter];
      pipe_remove(set_adapter);
    }
  }

  TraceValue("%u",state->spun_off[adapter]);
  TraceEOL();

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

  assert(SLICE_TYPE(constraint)==STProxy);

  {
    slice_index const prototype = alloc_constraint_tester_slice(constraint);
    slice_insertion_insert(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a series branch with STConstraintTester slices (as possible in
 * elabore sstip stipulations)
 * @param si entry slice of branch to be instrumented
 * @param constraint identifies branch that constrains the attacker
 */
void series_branch_insert_goal_constraint(slice_index si, slice_index constraint)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",constraint);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  TraceStipulation(constraint);

  assert(SLICE_TYPE(constraint)==STProxy);

  {
    slice_index const prototype = alloc_goal_constraint_tester_slice(constraint);
    slice_insertion_insert(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
