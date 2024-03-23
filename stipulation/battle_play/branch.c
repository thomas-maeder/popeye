#include "stipulation/battle_play/branch.h"
#include "stipulation/pipe.h"
#include "stipulation/move_inverter.h"
#include "stipulation/conditional_pipe.h"
#include "stipulation/constraint.h"
#include "stipulation/proxy.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/branch.h"
#include "stipulation/fork.h"
#include "stipulation/move.h"
#include "stipulation/move_played.h"
#include "stipulation/binary.h"
#include "stipulation/boolean/true.h"
#include "stipulation/battle_play/attack_adapter.h"
#include "stipulation/battle_play/defense_adapter.h"
#include "stipulation/modifier.h"
#include "solving/pipe.h"
#include "output/plaintext/message.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

#include <limits.h>

#include <stdio.h>  /* included for fprintf(FILE *, char const *, ...) */
#include <stdlib.h> /* included for exit(int) */

/* Order in which the slice types appear in battle branches
 * some types are not mentioned because they have variable ranks.
 */
static slice_index const slice_rank_order[] =
{
  STAttackAdapter,
  STGoalReachedTester,
  STMummerDeadend,
  STTrivialEndFilter,
  STOutputPlainTextMoveWriter,
  STOutputLaTeXMoveWriter,
  STOhneschachDetectUndecidableGoal,
  STOutputPlaintextTreeRefutingVariationWriter,
  STOutputLaTeXTreeRefutingVariationWriter,
  STOutputPlaintextTreeCheckWriter,
  STOutputLaTeXTreeCheckWriter,
  STOutputPlaintextGoalWriter,
  STOutputLaTeXGoalWriter,
  STOutputPlaintextConstraintWriter,
  STOutputLaTeXConstraintWriter,
  STOutputPlainTextRefutationWriter,
  STOutputLaTeXRefutationWriter,
  STKeepMatingFilter,
  STEndOfBranch,
  STNotEndOfBranch,
  STEndOfIntro,
  STReadyForAttack,
  STMaxThreatLengthStart, /* separate from STThreatStart to enable hashing*/
  STAttackHashed,
  STAttackHashedTester,
  STOutputPlainTextZugzwangWriter,
  STOutputLaTeXZugzwangWriter,
  STThreatStart,
  STGoalConstraintTester,
  STResetUnsolvable,
  STConstraintSolver,
  STConstraintTester,
  STEndOfRefutationSolvingBranch,
  STMinLengthOptimiser,
  STTestingPrerequisites,
  STCounterMateFilter,
  STDoubleMateFilter,
  STCastlingFilter,
  STEnPassantFilter,
  STPrerequisiteOptimiser,
  STRepublicanType1DeadEnd,
  STDegenerateTree,
  STFindShortest,
  STDeadEnd,
  STLearnUnsolvable,
  STShortSolutionsStart,
  STGeneratingMoves,
  STExecutingKingCapture,
  STExclusiveChessExclusivityDetector,
  STExclusiveChessNestedExclusivityDetector,
  STMakeTakeResetMoveIdsCastlingAsMakeInMoveGeneration,
  STBicapturesRecolorPieces,
  STMoveGenerator,
  STNullMoveGenerator,
  STOrthodoxMatingMoveGenerator,
  STOrthodoxMatingKingContactGenerator,
  STBicapturesUnrecolorPieces,
  STRoleExchangeMoveGenerator,
  STSkipMoveGeneration,
  STDoneGeneratingMoves,
  STSuperTransmutingKingMoveGenerationFilter,
  STUncapturableRemoveCaptures,
  STBackhomeExistanceTester,
  STBackhomeRemoveIllegalMoves,
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
  STPiecesParalysingSuffocationFinder,
  STPiecesParalysingRemoveCaptures,
  STMummerOrchestrator,
  STDoneRemovingIllegalMoves,
  STEnPassantRemoveNonReachers,
  STCastlingRemoveNonReachers,
  STChess81RemoveNonReachers,
  STCaptureRemoveNonReachers,
  STTargetRemoveNonReachers,
  STCageCirceFutileCapturesRemover,
  STDoneRemovingFutileMoves,
  STRetractionPrioriser,
  STKillerMovePrioriser,
  STDonePriorisingMoves,
  STBulPlyRewinder,
  STSeriesCapturePlyRewinder,
  STMove,
  STAttackPlayed,
  STMaxTimeGuard,
  STMaxSolutionsGuard,
  STMaxSolutionsCounter,
  STRestartGuard,
  STOutputPlainTextTreeEndOfSolutionWriter,
  STOutputLaTeXTreeEndOfSolutionWriter,
  STThreatCollector,
  STThreatDefeatedTester,
  STKillerAttackCollector,
  STEndOfBranchGoal,
  STExclusiveChessGoalReachingMoveCounter,
  STGoalReachedTester,
  STNotEndOfBranchGoal,
  STDeadEndGoal,
  STSelfCheckGuard,
  STTotalInvisibleUninterceptableSelfCheckGuard,
  STOhneschachStopIfCheck,
  STOhneschachStopIfCheckAndNotMate,
  STLegalAttackCounter,
  STMummerDeadend,

  STDefenseAdapter,
  STEndOfIntro,
  STKeepMatingFilter,
  STEndOfBranch,
  STNotEndOfBranch,
  STMaxNrNonTrivial,
  STResetUnsolvable,
  STRefutationsAllocator,
  STSolvingContinuation,
  STContinuationSolver,
  STTrivialTryAvoider,
  STOutputPlainTextThreatWriter,
  STOutputLaTeXThreatWriter,
  STThreatEnd,
  STOutputPlainTextMoveWriter,
  STOutputLaTeXMoveWriter,
  STOutputPlaintextTreeCheckWriter,
  STOutputLaTeXTreeCheckWriter,
  STOhneschachDetectUndecidableGoal,
  STOutputPlaintextGoalWriter,
  STOutputLaTeXGoalWriter,
  STOutputPlaintextConstraintWriter,
  STOutputLaTeXConstraintWriter,
  STRefutationsSolver,
  STReadyForDefense,
  STOutputPlainTextKeyWriter,
  STOutputLaTeXKeyWriter,
  STOutputPlainTextTryWriter,
  STOutputLaTeXTryWriter,
  STOutputPlainTextRefutationsIntroWriter,
  STTrue,
  STOutputPlaintextLineLineWriter,
  STOutputLaTeXLineLineWriter,
  STPlaySuppressor,
  STGoalConstraintTester,
  STConstraintSolver,
  STConstraintTester,
  STEndOfBranchForced,
  STMaxFlightsquares,
  STMaxThreatLength,
  STThreatSolver,
  STTestingPrerequisites,
  STCounterMateFilter,
  STCastlingFilter,
  STEnPassantFilter,
  STPrerequisiteOptimiser,
  STDeadEnd,
  STGeneratingMoves,
  STExecutingKingCapture,
  STKillerMoveFinalDefenseMove,
  STExclusiveChessExclusivityDetector,
  STExclusiveChessNestedExclusivityDetector,
  STMakeTakeResetMoveIdsCastlingAsMakeInMoveGeneration,
  STBicapturesRecolorPieces,
  STMoveGenerator,
  STNullMoveGenerator,
  STKingMoveGenerator,
  STBicapturesUnrecolorPieces,
  STSinglePieceMoveGenerator,
  STRoleExchangeMoveGenerator,
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
  STCageCirceFutileCapturesRemover,
  STDoneRemovingFutileMoves,
  STRetractionPrioriser,
  STKillerMovePrioriser,
  STOpponentMovesFewMovesPrioriser,
  STDonePriorisingMoves,
  STBulPlyRewinder,
  STSeriesCapturePlyRewinder,
  STMove,
  STDummyMove,
  STDefensePlayed,
  STOutputPlainTextZugzwangByDummyMoveCheckWriter,
  STOutputLaTeXZugzwangByDummyMoveCheckWriter,
  STCaptureCounter,
  STResetUnsolvable,
  STAvoidUnsolvable,
  STMaxNrNonTrivialCounter,
  STRefutationsCollector,
  STRefutationsAvoider,
  STRefutationsFilter,
  STKillerDefenseCollector,
  STEndOfRoot,
  STMinLengthGuard,
  STEndOfBranchGoal,
  STNotEndOfBranchGoal,
  STSelfCheckGuard,
  STTotalInvisibleUninterceptableSelfCheckGuard,
  STOhneschachStopIfCheck,
  STOhneschachStopIfCheckAndNotMate,
  STLegalDefenseCounter,
  STOutputPlaintextLineRefutingVariationWriter,
  STCheckZigzagLanding,
  STNoShortVariations,
  STThreatEnforcer
};

enum
{
  nr_slice_rank_order_elmts = (sizeof slice_rank_order
                               / sizeof slice_rank_order[0])
};

/* Insert slices into a battle branch; the elements of
 * prototypes are *not* deallocated by battle_branch_insert_slices_nested().
 * The inserted slices are copies of the elements of prototypes).
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param adapter identifies starting point of insertion (of type STAttackAdapter
 *                or STDefenseAdapter)
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void battle_branch_insert_slices_nested(slice_index adapter,
                                        slice_index const prototypes[],
                                        unsigned int nr_prototypes)
{
  stip_structure_traversal st;
  branch_slice_insertion_state_type state =
  {
    prototypes, nr_prototypes,
    slice_rank_order, nr_slice_rank_order_elmts, 0,
    branch_slice_rank_order_recursive,
    0,
    adapter,
    0
  };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  assert(SLICE_TYPE(adapter)==STAttackAdapter
         || SLICE_TYPE(adapter)==STDefenseAdapter);

  state.base_rank = get_slice_rank(SLICE_TYPE(adapter),&state);
  assert(state.base_rank!=no_slice_rank);
  slice_insertion_init_traversal(&st,&state,stip_traversal_context_intro);
  move_init_slice_insertion_traversal(&st);
  stip_traverse_structure_children_pipe(adapter,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Prepare the instrumentation of an attack branch with some slices
 * The inserted slices will be copies of the elements of prototypes.
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 * @param st traversal to be prepared
 * @param state inseration state to be initialised
 */
void attack_branch_prepare_slice_insertion(slice_index si,
                                           slice_index const prototypes[],
                                           unsigned int nr_prototypes,
                                           stip_structure_traversal *st,
                                           branch_slice_insertion_state_type *state)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  attack_branch_prepare_slice_insertion_behind_proxy(si,prototypes,nr_prototypes,si,st,state);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert slices into a battle branch, starting between defense and attack move
 * Combines:
 * * attack_branch_prepare_slice_insertion()
 * * the actual insertion
 * * the deallocation of the prototypes
 * @param si cf. attack_branch_prepare_slice_insertion
 */
void attack_branch_insert_slices(slice_index si,
                                 slice_index const prototypes[],
                                 unsigned int nr_prototypes)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  attack_branch_insert_slices_behind_proxy(si,prototypes,nr_prototypes,si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Like attack_branch_prepare_slice_insertion, but starting at a proxy slice
 * @param base used instead of proxy for determining the current position in the
 *             sequence of defense branches
 */
void attack_branch_prepare_slice_insertion_behind_proxy(slice_index proxy,
                                                        slice_index const prototypes[],
                                                        unsigned int nr_prototypes,
                                                        slice_index base,
                                                        stip_structure_traversal *st,
                                                        branch_slice_insertion_state_type *state)
{
  state->prototypes = prototypes;
  state->nr_prototypes = nr_prototypes;
  state->slice_rank_order = slice_rank_order;
  state->nr_slice_rank_order_elmts = nr_slice_rank_order_elmts;
  state->type = branch_slice_rank_order_recursive;
  state->prev = proxy;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",base);
  TraceFunctionParamListEnd();

  assert(SLICE_TYPE(proxy)!=STAttackPlayed);

  state->base_rank = get_slice_rank(STAttackPlayed,state);
  assert(state->base_rank!=no_slice_rank);
  ++state->base_rank;

  state->base_rank = get_slice_rank(STDefensePlayed,state);
  assert(state->base_rank!=no_slice_rank);
  ++state->base_rank;

  slice_insertion_init_traversal(st,state,stip_traversal_context_attack);
  move_init_slice_insertion_traversal(st);

  state->base_rank = get_slice_rank(SLICE_TYPE(base),state);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Like attack_branch_insert_slices, but starting at a proxy slice
 * @param base used instead of proxy for determining the current position in the
 *             sequence of slices
 */
void attack_branch_insert_slices_behind_proxy(slice_index proxy,
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
    proxy,
    0
  };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  assert(SLICE_TYPE(proxy)!=STAttackPlayed);

  state.base_rank = get_slice_rank(STAttackPlayed,&state);
  assert(state.base_rank!=no_slice_rank);
  ++state.base_rank;

  state.base_rank = get_slice_rank(STDefensePlayed,&state);
  assert(state.base_rank!=no_slice_rank);
  ++state.base_rank;

  slice_insertion_init_traversal(&st,&state,stip_traversal_context_attack);
  move_init_slice_insertion_traversal(&st);

  state.base_rank = get_slice_rank(SLICE_TYPE(base),&state);
  stip_traverse_structure(proxy,&st);

  deallocate_slice_insertion_prototypes(prototypes,nr_prototypes);

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
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  defense_branch_insert_slices_behind_proxy(si,prototypes,nr_prototypes,si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Prepare the instrumentation of an attack branch with some slices
 * The inserted slices will be copies of the elements of prototypes.
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 * @param st traversal to be prepared
 * @param state inseration state to be initialised
 */
void defense_branch_prepare_slice_insertion_behind_proxy(slice_index proxy,
                                                         slice_index const prototypes[],
                                                         unsigned int nr_prototypes,
                                                         slice_index base,
                                                         branch_slice_insertion_state_type *state,
                                                         stip_structure_traversal *st)
{
  state->prototypes = prototypes;
  state->nr_prototypes = nr_prototypes;
  state->slice_rank_order = slice_rank_order;
  state->nr_slice_rank_order_elmts = nr_slice_rank_order_elmts;
  state->type = branch_slice_rank_order_recursive;
  state->prev = proxy;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParam("%u",base);
  TraceFunctionParamListEnd();

  state->base_rank = get_slice_rank(STAttackPlayed,state);
  assert(state->base_rank!=no_slice_rank);
  ++state->base_rank;

  slice_insertion_init_traversal(st,state,stip_traversal_context_defense);
  move_init_slice_insertion_traversal(st);

  state->base_rank = get_slice_rank(SLICE_TYPE(base),state);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Like defense_branch_insert_slices, but starting at a proxy slice
 * @param base_rank used instead of proxy for determining the current position in the
 *             sequence of defense branches
 */
void defense_branch_insert_slices_behind_proxy(slice_index proxy,
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
    proxy,
    0
  };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParam("%u",base);
  TraceFunctionParamListEnd();

  state.base_rank = get_slice_rank(STAttackPlayed,&state);
  assert(state.base_rank!=no_slice_rank);
  ++state.base_rank;

  slice_insertion_init_traversal(&st,&state,stip_traversal_context_defense);
  move_init_slice_insertion_traversal(&st);

  state.base_rank = get_slice_rank(SLICE_TYPE(base),&state);
  stip_traverse_structure(proxy,&st);

  deallocate_slice_insertion_prototypes(prototypes,nr_prototypes);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a branch consisting mainly of an defense move
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of entry slice to allocated branch
 */
slice_index alloc_defense_branch(stip_length_type length,
                                 stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  {
    slice_index const adapter = alloc_defense_adapter_slice(length,min_length);
    slice_index const dready = alloc_branch(STReadyForDefense,length,min_length);
    slice_index const dtestpre = alloc_pipe(STTestingPrerequisites);
    slice_index const ddeadend = alloc_pipe(STDeadEnd);
    slice_index const dgenerating = alloc_pipe(STGeneratingMoves);
    slice_index const done_dgenerating = alloc_pipe(STDoneGeneratingMoves);
    slice_index const done_dremoving_illegal = alloc_pipe(STDoneRemovingIllegalMoves);
    slice_index const done_dremoving_futile = alloc_pipe(STDoneRemovingFutileMoves);
    slice_index const done_dpriorising = alloc_pipe(STDonePriorisingMoves);
    slice_index const defense = alloc_pipe(STMove);
    slice_index const dplayed = alloc_defense_played_slice();
    slice_index const dnotgoal = alloc_pipe(STNotEndOfBranchGoal);
    slice_index const dnotend = alloc_pipe(STNotEndOfBranch);
    slice_index const aready = alloc_branch(STReadyForAttack,length-1,min_length-1);
    slice_index const atestpre = alloc_pipe(STTestingPrerequisites);
    slice_index const adeadend = alloc_pipe(STDeadEnd);
    slice_index const agenerating = alloc_pipe(STGeneratingMoves);
    slice_index const done_agenerating = alloc_pipe(STDoneGeneratingMoves);
    slice_index const done_aremoving_illegal = alloc_pipe(STDoneRemovingIllegalMoves);
    slice_index const done_aremoving_futile = alloc_pipe(STDoneRemovingFutileMoves);
    slice_index const done_apriorising = alloc_pipe(STDonePriorisingMoves);
    slice_index const attack = alloc_pipe(STMove);
    slice_index const aplayed = alloc_attack_played_slice();
    slice_index const anotgoal = alloc_pipe(STNotEndOfBranchGoal);
    slice_index const anotend = alloc_pipe(STNotEndOfBranch);

    pipe_link(adapter,dready);
    pipe_link(dready,dtestpre);
    pipe_link(dtestpre,ddeadend);
    pipe_link(ddeadend,dgenerating);
    pipe_link(dgenerating,done_dgenerating);
    pipe_link(done_dgenerating,done_dremoving_illegal);
    pipe_link(done_dremoving_illegal,done_dremoving_futile);
    pipe_link(done_dremoving_futile,done_dpriorising);
    pipe_link(done_dpriorising,defense);
    pipe_link(defense,dplayed);
    pipe_link(dplayed,dnotgoal);
    pipe_link(dnotgoal,dnotend);
    pipe_link(dnotend,aready);
    pipe_link(aready,atestpre);
    pipe_link(atestpre,adeadend);
    pipe_link(adeadend,agenerating);
    pipe_link(agenerating,done_agenerating);
    pipe_link(done_agenerating,done_aremoving_illegal);
    pipe_link(done_aremoving_illegal,done_aremoving_futile);
    pipe_link(done_aremoving_futile,done_apriorising);
    pipe_link(done_apriorising,attack);
    pipe_link(attack,aplayed);
    pipe_link(aplayed,anotgoal);
    pipe_link(anotgoal,anotend);
    pipe_link(anotend,adapter);

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

  {
    slice_index const adapter = alloc_attack_adapter_slice(length,min_length);
    slice_index const aready = alloc_branch(STReadyForAttack,length,min_length);
    slice_index const atestpre = alloc_pipe(STTestingPrerequisites);
    slice_index const adeadend = alloc_pipe(STDeadEnd);
    slice_index const agenerating = alloc_pipe(STGeneratingMoves);
    slice_index const done_agenerating = alloc_pipe(STDoneGeneratingMoves);
    slice_index const done_aremoving_illegal = alloc_pipe(STDoneRemovingIllegalMoves);
    slice_index const done_aremoving_futile = alloc_pipe(STDoneRemovingFutileMoves);
    slice_index const done_apriorising = alloc_pipe(STDonePriorisingMoves);
    slice_index const attack = alloc_pipe(STMove);
    slice_index const aplayed = alloc_attack_played_slice();
    slice_index const anotgoal = alloc_pipe(STNotEndOfBranchGoal);
    slice_index const anotend = alloc_pipe(STNotEndOfBranch);
    slice_index const dready = alloc_branch(STReadyForDefense,
                                            length-1,min_length-1);
    slice_index const dtestpre = alloc_pipe(STTestingPrerequisites);
    slice_index const ddeadend = alloc_pipe(STDeadEnd);
    slice_index const dgenerating = alloc_pipe(STGeneratingMoves);
    slice_index const done_dgenerating = alloc_pipe(STDoneGeneratingMoves);
    slice_index const done_dremoving_illegal = alloc_pipe(STDoneRemovingIllegalMoves);
    slice_index const done_dremoving_futile = alloc_pipe(STDoneRemovingFutileMoves);
    slice_index const done_dpriorising = alloc_pipe(STDonePriorisingMoves);
    slice_index const defense = alloc_pipe(STMove);
    slice_index const dplayed = alloc_defense_played_slice();
    slice_index const dnotgoal = alloc_pipe(STNotEndOfBranchGoal);
    slice_index const dnotend = alloc_pipe(STNotEndOfBranch);

    pipe_link(adapter,aready);
    pipe_link(aready,atestpre);
    pipe_link(atestpre,adeadend);
    pipe_link(adeadend,agenerating);
    pipe_link(agenerating,done_agenerating);
    pipe_link(done_agenerating,done_aremoving_illegal);
    pipe_link(done_aremoving_illegal,done_aremoving_futile);
    pipe_link(done_aremoving_futile,done_apriorising);
    pipe_link(done_apriorising,attack);
    pipe_link(attack,aplayed);
    pipe_link(aplayed,anotgoal);
    pipe_link(anotgoal,anotend);
    pipe_link(anotend,dready);
    pipe_link(dready,dtestpre);
    pipe_link(dtestpre,ddeadend);
    pipe_link(ddeadend,dgenerating);
    pipe_link(dgenerating,done_dgenerating);
    pipe_link(done_dgenerating,done_dremoving_illegal);
    pipe_link(done_dremoving_illegal,done_dremoving_futile);
    pipe_link(done_dremoving_futile,done_dpriorising);
    pipe_link(done_dpriorising,defense);
    pipe_link(defense,dplayed);
    pipe_link(dplayed,dnotgoal);
    pipe_link(dnotgoal,dnotend);
    pipe_link(dnotend,adapter);

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
  spin_off_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);
  TraceValue("%u",state->spun_off[SLICE_NEXT1(si)]);
  TraceEOL();

  state->spun_off[si] = copy_slice(si);
  if (state->spun_off[si]==no_slice)
  {
    fprintf(stderr, "\nOUT OF SPACE: Unable to copy slice in %s in %s -- aborting.\n", __func__, __FILE__);
    exit(2); /* TODO: Do we have to exit here? */
  }
  link_to_branch(state->spun_off[si],state->spun_off[SLICE_NEXT1(si)]);
  TraceValue("%u",state->spun_off[si]);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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

/* Produce slices representing set play.
 * This is supposed to be invoked from within the slice type specific
 * functions invoked by stip_apply_setplay.
 * @param adapter identifies the adapter slice into the battle branch
 * @param state address of structure holding state
 */
void battle_branch_make_setplay(slice_index adapter, spin_off_state_type *state)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  {
    slice_index const start = branch_find_slice(STReadyForDefense,
                                                adapter,
                                                stip_traversal_context_intro);
    stip_structure_traversal st;

    slice_index const notend = branch_find_slice(STNotEndOfBranchGoal,
                                                 adapter,
                                                 stip_traversal_context_intro);
    slice_index const prototype = alloc_pipe(STEndOfRoot);
    assert(notend!=no_slice);
    defense_branch_insert_slices(notend,&prototype,1);

    assert(start!=no_slice);

    stip_structure_traversal_init(&st,state);
    st.context = stip_traversal_context_defense;
    stip_structure_traversal_override_by_structure(&st,
                                                   slice_structure_pipe,
                                                   &copy_to_setplay);
    stip_structure_traversal_override_by_structure(&st,
                                                   slice_structure_branch,
                                                   &copy_to_setplay);
    stip_structure_traversal_override_by_structure(&st,
                                                   slice_structure_fork,
                                                   &copy_to_setplay);
    stip_structure_traversal_override_single(&st,STEndOfRoot,&serve_as_root_hook);
    stip_traverse_structure(start,&st);
    TraceValue("%u",state->spun_off[start]);
    TraceEOL();
    state->spun_off[adapter] = state->spun_off[start];
  }

  TraceValue("%u",state->spun_off[adapter]);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionParamListEnd();
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

  assert(SLICE_TYPE(adapter)==STAttackAdapter);

  {
    slice_index const notend = branch_find_slice(STNotEndOfBranchGoal,
                                                 adapter,
                                                 stip_traversal_context_intro);
    stip_length_type const length = SLICE_U(adapter).branch.length;
    stip_length_type const min_length = SLICE_U(adapter).branch.min_length;
    slice_index const proto = alloc_defense_adapter_slice(length-1,
                                                          min_length-1);
    assert(notend!=no_slice);
    defense_branch_insert_slices(notend,&proto,1);

    result = branch_find_slice(STDefenseAdapter,
                               notend,
                               stip_traversal_context_defense);
    assert(result!=no_slice);

    branch_shorten_slices(adapter,STDefenseAdapter,stip_traversal_context_intro);
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

  stip_traverse_structure_children_pipe(si,st);

  if (*root_slice!=no_slice)
  {
    link_to_branch(si,*root_slice);
    *root_slice = si;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Attempt to apply the postkey play option to the current stipulation
 * @param root_proxy identifies root proxy slice
 * @return true iff postkey play option is applicable (and has been
 *              applied)
 */
static boolean battle_branch_apply_postkeyplay(slice_index root_proxy)
{
  boolean result;
  slice_index postkey_slice = no_slice;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root_proxy);
  TraceFunctionParamListEnd();

  TraceStipulation(root_proxy);

  stip_structure_traversal_init(&st,&postkey_slice);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_pipe,
                                                 &move_to_postkey);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_testing_pipe,
                                                  &move_to_postkey);
  stip_structure_traversal_override_single(&st,
                                           STAttackAdapter,
                                           &attack_adapter_make_postkeyplay);
  stip_structure_traversal_override_single(&st,
                                           STHelpAdapter,
                                           &stip_structure_visitor_noop);
  stip_traverse_structure_children_pipe(root_proxy,&st);

  if (postkey_slice==no_slice)
    result = false;
  else
  {
    link_to_branch(root_proxy,postkey_slice);

    {
      slice_index const prototype = alloc_move_inverter_slice();
      slice_insertion_insert(root_proxy,&prototype,1);
    }

    result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}

/* Apply the option postkeyplay to a newly created stipulation
 * @param si identifies the postkeyplay applier slice
 */
void post_key_play_stipulation_modifier_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const stipulation_root_hook = stipulation_modifier_to_be_modified(si);
    if (stipulation_root_hook!=no_slice)
    {
      if (!battle_branch_apply_postkeyplay(stipulation_root_hook))
        output_plaintext_message(PostKeyPlayNotApplicable);
    }
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionParamListEnd();
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
    if (state->spun_off[si]==no_slice)
    {
      fprintf(stderr, "\nOUT OF SPACE: Unable to copy slice in %s in %s -- aborting.\n", __func__, __FILE__);
      exit(2); /* TODO: Do we have to exit here? */
    }
    link_to_branch(state->spun_off[si],state->spun_off[SLICE_NEXT1(si)]);
  }
  TraceValue("%u",state->spun_off[si]);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void ready_for_defense_make_root(slice_index si,
                                        stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototype = alloc_pipe(STEndOfRoot);
    defense_branch_insert_slices(si,&prototype,1);
  }

  pipe_spin_off_copy(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Create the root slices of a battle branch
 * @param adapter identifies the adapter slice at the beginning of the branch
 * @param state address of structure holding state
 */
void battle_branch_make_root_slices(slice_index adapter,
                                    spin_off_state_type *state)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st;

    stip_structure_traversal_init(&st,state);
    stip_structure_traversal_override_by_structure(&st,
                                                   slice_structure_leaf,
                                                   &leaf_spin_off_copy);
    stip_structure_traversal_override_by_structure(&st,
                                                   slice_structure_pipe,
                                                   &pipe_spin_off_copy);
    stip_structure_traversal_override_by_structure(&st,
                                                   slice_structure_branch,
                                                   &pipe_spin_off_copy);
    stip_structure_traversal_override_by_structure(&st,
                                                   slice_structure_fork,
                                                   &fork_make_root);
    stip_structure_traversal_override_by_contextual(&st,
                                                    slice_contextual_binary,
                                                    &binary_make_root);
    stip_structure_traversal_override_by_contextual(&st,
                                                    slice_contextual_conditional_pipe,
                                                    &conditional_pipe_spin_off_copy);
    stip_structure_traversal_override_single(&st,
                                             STReadyForDefense,
                                             &ready_for_defense_make_root);
    stip_structure_traversal_override_single(&st,
                                             STConstraintTester,
                                             &constraint_tester_make_root);
    stip_structure_traversal_override_single(&st,
                                             STGoalConstraintTester,
                                             &goal_constraint_tester_make_root);
    stip_structure_traversal_override_single(&st,
                                             STReadyForDefense,
                                             &ready_for_defense_make_root);
    stip_structure_traversal_override_single(&st,
                                             STEndOfRoot,
                                             &serve_as_root_hook);
    stip_traverse_structure(adapter,&st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionParamListEnd();
}

/* Wrap the slices representing the initial moves of the solution with
 * slices of appropriately equipped slice types
 * @param adapter identifies the adapter slice at the beginning of the branch
 * @param state address of structure holding state
 */
void battle_make_root(slice_index adapter, spin_off_state_type *state)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  battle_branch_make_root_slices(adapter,state);

  branch_shorten_slices(adapter,STEndOfRoot,stip_traversal_context_intro);
  pipe_remove(adapter);

  TraceFunctionExit(__func__);
  TraceFunctionParamListEnd();
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
    slice_index const ready = branch_find_slice(STReadyForDefense,
                                                si,
                                                stip_traversal_context_intro);
    assert(ready!=no_slice);
    pipe_append(ready,alloc_fork_slice(STEndOfBranchForced,forced));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a battle branch with STConstraint* slices (typically for a reflex
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
    slice_index const ready = branch_find_slice(STReadyForAttack,
                                                si,
                                                stip_traversal_context_intro);
    slice_index const prototype = alloc_constraint_tester_slice(constraint);
    assert(ready!=no_slice);
    attack_branch_insert_slices(ready,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a battle branch with STConstraint* slices (typically for a reflex
 * stipulation)
 * @param si entry slice of branch to be instrumented
 * @param constraint identifies branch that constrains the attacker
 */
void battle_branch_insert_attack_goal_constraint(slice_index si,
                                                 slice_index constraint)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",constraint);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  TraceStipulation(constraint);

  {
    slice_index const ready = branch_find_slice(STReadyForAttack,
                                                si,
                                                stip_traversal_context_intro);
    slice_index const prototype = alloc_goal_constraint_tester_slice(constraint);
    assert(ready!=no_slice);
    attack_branch_insert_slices(ready,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a battle branch with STConstraint* slices (typically for a reflex
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
    slice_index const ready = branch_find_slice(STReadyForDefense,
                                                si,
                                                stip_traversal_context_intro);
    slice_index const prototype = alloc_constraint_tester_slice(constraint);
    assert(ready!=no_slice);
    defense_branch_insert_slices(ready,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a battle branch with STConstraint* slices (typically for a reflex
 * stipulation)
 * @param si entry slice of branch to be instrumented
 * @param constraint identifies branch that constrains the attacker
 */
void battle_branch_insert_defense_goal_constraint(slice_index si,
                                                  slice_index constraint)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",constraint);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  TraceStipulation(constraint);

  {
    slice_index const ready = branch_find_slice(STReadyForDefense,
                                                si,
                                                stip_traversal_context_intro);
    slice_index const prototype = alloc_goal_constraint_tester_slice(constraint);
    assert(ready!=no_slice);
    defense_branch_insert_slices(ready,&prototype,1);
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
    slice_index const ready = branch_find_slice(STReadyForAttack,
                                                si,
                                                stip_traversal_context_intro);
    slice_index const prototype = alloc_fork_slice(STEndOfBranchGoal,goal);
    assert(ready!=no_slice);
    attack_branch_insert_slices(ready,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a branch with slices dealing with direct play
 * @param si root of branch to be instrumented
 * @param next1 identifies slice leading towards goal
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
    slice_index const ready = branch_find_slice(STReadyForAttack,
                                                si,
                                                stip_traversal_context_intro);
    slice_index const prototype = alloc_fork_slice(STEndOfBranch,next);
    assert(ready!=no_slice);
    attack_branch_insert_slices(ready,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a branch with slices dealing with self play
 * @param si root of branch to be instrumented
 * @param next1 identifies slice leading towards goal
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
    slice_index const ready = branch_find_slice(STReadyForDefense,
                                                si,
                                                stip_traversal_context_intro);
    slice_index const prototype = alloc_fork_slice(STEndOfBranch,goal);
    assert(ready!=no_slice);
    defense_branch_insert_slices(ready,&prototype,1);
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
    slice_index const ready = branch_find_slice(STReadyForDefense,
                                                si,
                                                stip_traversal_context_intro);
    slice_index const prototype = alloc_fork_slice(STEndOfBranchGoal,goal);
    assert(ready!=no_slice);
    defense_branch_insert_slices(ready,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
