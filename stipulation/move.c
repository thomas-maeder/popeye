#include "stipulation/move.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
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
    STNewKoekoRememberContact,
    STSingleBoxType3PawnPromoter,
    STAMUAttackCounter,
    STNullMovePlayer,
    STPostMoveIterationInitialiser,
    STOpponentMovesCounter,
    STMummerBookkeeper,
    STUltraMummerMeasurerDeadend,
    STBackHomeMovesOnly,
    STCastlingPlayer,
    STMessignyMovePlayer,
    STCastlingChessMovePlayer,
    STExchangeCastlingMovePlayer,
    STMovePlayer,
    STLandingAfterMovingPieceMovement,
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
    STBeforePawnPromotion,
    STNoPromotionsRemovePromotionMoving,
    STPawnToImitatorPromoter,
    STPawnPromoter,
    STChameleonChangePromoteeInto,
    STHauntedChessGhostSummoner,
    STLandingAfterPawnPromotion,
    STFootballChessSubsitutor,
    STLineChameleonArrivingAdjuster,
    STNorskArrivingAdjuster,
    STWormholeTransferer,
    STBeforePawnPromotion,
    STPawnPromoter,
    STChameleonChangePromoteeInto,
    STLandingAfterPawnPromotion,
    STEnPassantAdjuster,
    STEinsteinEnPassantAdjuster,
    STPhantomChessEnPassantAdjuster,
    STAntiMarsCirceEnPassantAdjuster,
    STFrischaufPromoteeMarker,
    STDegradierungDegrader,
    STEinsteinArrivingAdjuster,
    STReverseEinsteinArrivingAdjuster,
    STAntiEinsteinArrivingAdjuster,
    STPiecesHalfNeutralRecolorer,
    STKobulKingSubstitutor,
    STDuellistsRememberDuellist,
    STSingleboxType2LatentPawnSelector,
    STSingleboxType2LatentPawnPromoter,
    STMagicSquareType2SideChanger,
    STAnticirceConsideringRebirth,
    STCirceKamikazeCaptureFork,
    STCirceCaptureFork,
    STCirceParrainCaptureFork,
    STCirceCageNoCageFork,
    STAnticirceDeterminingRebornPiece,
    STAnticirceInitialiseFromCurrentCapture,
    STAnticirceParrainInitialiseFromCaptureInLastMove,
    STMirrorCirceOverrideRelevantSide,
    STMagicSquareType2AnticirceRelevantSideAdapter,
    STAntiCloneCirceDetermineRebornWalk,
    STDiagramCirceDetermineRebirthSquare,
    STFileCirceDetermineRebirthSquare,
    STSymmetryCirceDetermineRebirthSquare,
    STAntipodesCirceDetermineRebirthSquare,
    STCirceParrainDetermineRebirth,
    STCirceCageDetermineRebirthSquare,
    STTakeMakeCirceDetermineRebirthSquares,
    STCirceDetermineRebirthSquare,
    STAntisupercirceDetermineRebirthSquare,
    STAnticirceCheylanFilter,
    STAnticirceRemoveCapturer,
    STCirceTestRebirthSquareEmpty,
    STAnticirceRebirthOnNonEmptySquare,
    STSupercircePreventRebirthOnNonEmptySquare,
    STCirceAssassinAssassinate,
    STAnticircePlacingReborn,
    STAnticircePlaceReborn,
    STBeforePawnPromotion,
    STPawnPromoter,
    STChameleonChangePromoteeInto,
    STLandingAfterPawnPromotion,
    STCirceCageCageTester,
    STCirceRebirthAvoided,
    STLandingAfterAnticirceRebirth,
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
    STCirceCaptureFork,
    STCirceParrainCaptureFork,
    STAprilCaptureFork,
    STSuperCirceNoRebirthFork,
    STCirceCageNoCageFork,
    STCirceDeterminingRebornPiece,
    STCirceInitialiseFromCurrentCapture,
    STCirceParrainInitialiseFromCaptureInLastMove,
    STCirceParrainDetermineRebirth,
    STCirceContraparrainDetermineRebirth,
    STCircePreventKingRebirth,
    STCirceCloneDetermineRebornWalk,
    STChameleonCirceAdaptRebornWalk,
    STAntiEinsteinDetermineRebornPiece,
    STCirceCouscousMakeCapturerRelevant,
    STMirrorCirceOverrideRelevantSide,
    STRankCirceOverrideRelevantSide,
    STCirceDetermineRebirthSquare,
    STFileCirceDetermineRebirthSquare,
    STAntipodesCirceDetermineRebirthSquare,
    STSymmetryCirceDetermineRebirthSquare,
    STPWCDetermineRebirthSquare,
    STDiagramCirceDetermineRebirthSquare,
    STCirceDiametralAdjustRebirthSquare,
    STRankCirceProjectRebirthSquare,
    STTakeMakeCirceDetermineRebirthSquares,
    STSuperCirceDetermineRebirthSquare,
    STCirceCageDetermineRebirthSquare,
    STContactGridAvoidCirceRebirth,
    STCirceTestRebirthSquareEmpty,
    STCirceRebirthOnNonEmptySquare,
    STSupercircePreventRebirthOnNonEmptySquare,
    STCirceAssassinAssassinate,
    STCircePlacingReborn,
    STCircePlaceReborn,
    STBeforePawnPromotion,
    STPawnPromoter,
    STChameleonChangePromoteeInto,
    STLandingAfterPawnPromotion,
    STCirceDoubleAgentsAdaptRebornSide,
    STCirceVolageRecolorer,
    STCirceCageCageTester,
    STCirceRebirthAvoided,
    STLandingAfterCirceRebirthHandler,
    STMagicPiecesRecolorer,
    STSentinellesInserter,
    STRepublicanKingPlacer,
    STActuatedRevolvingBoard,
    STActuatedRevolvingCentre,
    STDynastyKingSquareUpdater,
    STStrictSATUpdater,
    STMasandRecolorer,
    STLandingAfterMovePlay,
    STMoveCounter,
    STMoveTracer,
    STBGLAdjuster,
    STDetectMoveRetracted,
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
    STAttackPlayed,
    STDefensePlayed,
    STHelpMovePlayed
};

enum
{
  nr_move_slice_rank_order_elmts = sizeof move_slice_rank_order / sizeof move_slice_rank_order[0],
  nr_exit_slice_types = 3
};

/* Determine whether a slice type contributes to the execution of moves
 * @param type slice type
 * @return true iff type is a slice type that contributes to the execution of moves
 */
boolean is_move_slice_type(slice_type type)
{
  unsigned int i;
  for (i = 0; i!=nr_move_slice_rank_order_elmts-nr_exit_slice_types; ++i)
    if (type==move_slice_rank_order[i])
      return true;

  return false;
}

/* Start inserting according to the slice type order for move execution
 * @param si identifies starting point of insertion
 * @param st insertion traversal where we come from and will return to
 * @param end_of_factored_order slice type where to return to insertion defined
 *                              by st
 */
void start_insertion_according_to_move_order(slice_index si,
                                             stip_structure_traversal *st,
                                             slice_type end_of_factored_order)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  branch_insert_slices_factored_order(si,
                                      st,
                                      move_slice_rank_order,
                                      nr_move_slice_rank_order_elmts,
                                      end_of_factored_order);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
    branch_insert_slices_contextual(si,st->context,&prototype,1);
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
