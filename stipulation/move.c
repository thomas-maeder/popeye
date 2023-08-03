#include "stipulation/move.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "pieces/walks/pawns/promotion.h"
#include "conditions/circe/circe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Order in which the slice types for move execution appear
 */
static slice_index const move_slice_rank_order[] =
{
    STMove,
    STDummyMove,
    STForEachAttack,
    STFindAttack,
    STForEachDefense,
    STFindDefense,
    STMoveEffectJournalUndoer,
    STTotalInvisibleMoveSequenceMoveRepeater,
    STNewKoekoRememberContact,
    STSingleBoxType3PawnPromoter,
    STAMUAttackCounter,
    STOpponentMovesCounter,
    STPostMoveIterationInitialiser,
    STNullMovePlayer,
    STRoleExchangeMovePlayer,
    STTotalInvisibleSpecialMovesPlayer,
    STMummerBookkeeper,
    STUltraMummerMeasurerDeadend,
    STBackHomeMovesOnly,
    STMarsCirceMoveToRebirthSquare,
    STCastlingPlayer,
    STMessignyMovePlayer,
    STCastlingChessMovePlayer,
    STExchangeCastlingMovePlayer,
    STMovePlayer,
    STLandingAfterMovingPieceMovement,
    STMakeTakeMoveCastlingPartner,
    STGenevaConsideringRebirth,
    STCirceDoneWithRebirth,
    STImitatorMover,
    STMutualCastlingRightsAdjuster,
    STSuperTransmutingKingTransmuter,
    STGhostChessGhostRememberer,
    STHauntedChessGhostRememberer,
    STHaanChessHoleInserter,
    STKamikazeCapturingPieceRemover, /* avoid promotions of a vanishing pawn */
    STChameleonArrivingAdjuster,
    STChameleonChessArrivingAdjuster,
    STProteanPawnAdjuster,
    STBretonApplier,
    STBeforePawnPromotion, /* moving pawn */
    STLandingAfterPawnPromotion,
    STHauntedChessGhostSummoner,
    STFootballChessSubsitutor,
    STLineChameleonArrivingAdjuster,
    STNorskArrivingAdjuster,
    STWormholeTransferer,
    STBeforePawnPromotion, /* transfered pawn */
    STLandingAfterPawnPromotion,
    STEnPassantAdjuster,
    STEinsteinEnPassantAdjuster,
    STFrischaufPromoteeMarker,
    STDegradierungDegrader,
    STEinsteinArrivingAdjuster,
    STReverseEinsteinArrivingAdjuster,
    STAntiEinsteinArrivingAdjuster,
    STPiecesHalfNeutralRecolorer,
    STKobulKingSubstitutor,
    STSnekSubstitutor,
    STSnekCircleSubstitutor,
    STDuellistsRememberDuellist,
    STSingleboxType2LatentPawnSelector,
    STSingleboxType2LatentPawnPromoter,
    STMagicSquareType2SideChanger,
    STAnticirceConsideringRebirth,
    STCirceDoneWithRebirth,
    STBretonPopeyeApplier,
    STTibetSideChanger,
    STDoubleTibetSideChanger,
    STAndernachSideChanger,
    STAntiAndernachSideChanger,
    STChameleonPursuitSideChanger,
    STTraitorSideChanger,
    STVolageSideChanger,
    STMagicSquareSideChanger,
    STOscillatingKingsTypeA,
    STOscillatingKingsTypeB,
    STOscillatingKingsTypeC,
    STHurdleColourChanger,
    STCirceConsideringRebirth,
    STCirceDoneWithRebirth,
    STMagicPiecesRecolorer,
    STSentinellesInserter,
    STCirceParachuteUncoverer,
    STRepublicanKingPlacer,
    STActuatedRevolvingBoard,
    STActuatedRevolvingCentre,
    STStrictSATUpdater,
    STMasandRecolorer,
    STMasandGeneralisedRecolorer,
    STInfluencerWalkChanger,
    STBeforePawnPromotion, /* pawn created by influence */
    STLandingAfterPawnPromotion,
    STBeforeSeriesCapture,
    STSeriesCaptureFork,
    STSeriesCapture,
    STBeforePawnPromotion, /* pawn moved by Series Capture */
    STLandingAfterPawnPromotion,
    STSeriesCaptureRecursor,
    STLandingAfterPawnPromotion,
    STDynastyKingSquareUpdater,
    STLostPiecesRemover,
    STLandingAfterMovePlay,
    STMoveCounter,
    STTotalInvisibleReserveKingMovement,
    STKingSquareUpdater,
    STMoveTracer,
    STBGLAdjuster,
    STKoekoLegalityTester,
    STGridContactLegalityTester,
    STAntiKoekoLegalityTester,
    STNewKoekoLegalityTester,
    STJiggerLegalityTester,
    STOwnKingCaptureAvoider,
    STOpponentKingCaptureAvoider,
    STCastlingRightsAdjuster,
    STBlackChecks,
    STImitatorDetectIllegalMoves,
    STSingleBoxType1LegalityTester,
    STSingleBoxType2LegalityTester,
    STSingleBoxType3LegalityTester,
    STExclusiveChessUndecidableWriterTree,
    STExclusiveChessUndecidableWriterLine,
    STExclusiveChessLegalityTester,
    STUltraschachzwangLegalityTester,
    STIsardamLegalityTester,
    STPatienceChessLegalityTester,
    STDetectMoveRetracted,
    STAttackPlayed,
    STDefensePlayed,
    STHelpMovePlayed
};

enum
{
  nr_move_slice_rank_order_elmts = sizeof move_slice_rank_order / sizeof move_slice_rank_order[0],
  nr_move_exit_slice_types = 3
};

/* Insert slices into a move execution slices sequence.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by move_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param context initial context of the insertion traversal; typically the
 *                current context of a surrounding traversal that has arrived
 *                at slice si
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void move_insert_slices(slice_index si,
                        stip_traversal_context_type context,
                        slice_index const prototypes[],
                        unsigned int nr_prototypes)
{
  stip_structure_traversal st;
  branch_slice_insertion_state_type state =
  {
      prototypes, nr_prototypes,
      move_slice_rank_order, nr_move_slice_rank_order_elmts, nr_move_exit_slice_types,
      branch_slice_rank_order_nonrecursive,
      0,
      si,
      0
  };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",context);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  slice_insertion_init_traversal(&st,&state,context);
  circe_init_slice_insertion_traversal(&st);
  promotion_init_slice_insertion_traversal(&st);

  state.base_rank = get_slice_rank(SLICE_TYPE(si),&state);
  stip_traverse_structure(si,&st);

  deallocate_slice_insertion_prototypes(prototypes,nr_prototypes);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_visit_move(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!slice_insertion_insert_before(si,st))
  {
    stip_structure_traversal st_nested;
    branch_slice_insertion_state_type state_nested;
    slice_insertion_prepare_factored_order(si,
                                           st,
                                           &st_nested,&state_nested,
                                           move_slice_rank_order,
                                           nr_move_slice_rank_order_elmts,
                                           nr_move_exit_slice_types);
    stip_traverse_structure_children_pipe(si,&st_nested);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise a structure traversal for inserting slices
 * into the move execution sequence
 * @param st address of structure representing the traversal
 */
void move_init_slice_insertion_traversal(stip_structure_traversal *st)
{
  stip_structure_traversal_override_single(st,STMove,&insert_visit_move);
  stip_structure_traversal_override_single(st,STDummyMove,&insert_visit_move);

  circe_init_slice_insertion_traversal(st);
  promotion_init_slice_insertion_traversal(st);
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_type const * const type = st->param;
    slice_index const prototype = alloc_pipe(*type);
    move_insert_slices(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument moves with a slice type
 * @param identifies where to start instrumentation
 * @param type type of slice with which to instrument moves
 */
void stip_instrument_moves(slice_index si, slice_type type)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&type);
  stip_structure_traversal_override_single(&st,STMove,&instrument_move);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument dummy moves with a slice type
 * @param identifies where to start instrumentation
 * @param type type of slice with which to instrument moves
 */
void stip_instrument_dummy_moves(slice_index si, slice_type type)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&type);
  stip_structure_traversal_override_single(&st,STDummyMove,&instrument_move);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
