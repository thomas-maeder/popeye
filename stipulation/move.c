#include "stipulation/move.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "debugging/trace.h"

#include <assert.h>

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
    STOpponentMovesCounter,
    STMoveEffectJournalUndoer,
    STNewKoekoRememberContact,
    STSingleBoxType3PawnPromoter,
    STAMUAttackCounter,
    STNullMovePlayer,
    STPostMoveIterationInitialiser,
    STMummerBookkeeper,
    STUltraMummerMeasurerDeadend,
    STBackHomeMovesOnly,
    STCastlingPlayer,
    STMessignyMovePlayer,
    STCastlingChessMovePlayer,
    STExchangeCastlingMovePlayer,
    STMovePlayer,
    STLandingAfterMovingPieceMovement,
    STMutualCastlingRightsAdjuster,
    STSuperTransmutingKingTransmuter,
    STGhostChessGhostRememberer,
    STHauntedChessGhostRememberer,
    STHauntedChessGhostSummoner,
    STHaanChessHoleInserter,
    STKamikazeCapturingPieceRemover, /* avoid promotions of a vanishing pawn */
    STChameleonArrivingAdjuster,
    STChameleonChessArrivingAdjuster,
    STProteanPawnAdjuster,
    STNoPromotionsRemovePromotionMoving,
    STMovingPawnToImitatorPromoter,
    STMovingPawnPromoter,
    STPromoteMovingIntoChameleon,
    STLandingAfterMovingPawnPromoter,
    STFootballChessSubsitutor,
    STLineChameleonArrivingAdjuster,
    STNorskArrivingAdjuster,
    STWormholeTransferer,
    STWormholeTransferedPromoter,
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
    STImitatorMover,
    STKobulKingSubstitutor,
    STDuellistsRememberDuellist,
    STSingleboxType2LatentPawnSelector,
    STSingleboxType2LatentPawnPromoter,
    STMagicSquareType2SideChanger,
    STAnticirceCaptureFork,
    STCirceKamikazeCaptureFork,
    STAnticirceDetermineRebornPiece,
    STAnticirceDetermineRevelantPiece,
    STAnticirceCouscousDetermineRevelantPiece,
    STMagicSquareType2AnticirceRelevantSideAdapter,
    STAnticirceDetermineRebirthSquare,
    STAntisupercirceDetermineRebirthSquare,
    STAnticirceCheylanFilter,
    STAnticircePlaceRebornStrict,
    STAnticircePlaceRebornRelaxed,
    STAnticirceRebornPromoter,
    STPromoteAnticirceRebornIntoChameleon,
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
    STCirceCaptureFork,
    STAprilAprilFork,
    STSuperCirceNoRebirthFork,
    STCirceCageNoCageFork,
    STCirceParrainDetermineRebirth,
    STCirceDetermineRebornPiece,
    STCirceCloneDetermineRebornPiece,
    STCirceDoubleAgentsAdaptRebornPiece,
    STChameleonCirceAdaptRebornPiece,
    STAntiEinsteinDetermineRebornPiece,
    STCirceDetermineRelevantPiece,
    STCirceCouscousDetermineRelevantPiece,
    STCirceDetermineRebirthSquare,
    STTakeMakeCirceDetermineRebirthSquares,
    STTakeMakeCirceNoRebirth,
    STSuperCirceRebirthHandler,
    STContactGridAvoidCirceRebirth,
    STCirceAssassinPlaceReborn,
    STCircePlaceReborn,
    STNoPromotionsRemovePromotionReborn,
    STCirceRebirthPromoter,
    STPromoteCirceRebornIntoChameleon,
    STCirceTraitorSideChanger,
    STCirceVolageRecolorer,
    STCirceCageCageTester,
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
    STSingleBoxType1LegalityTester,
    STSingleBoxType2LegalityTester,
    STSingleBoxType3LegalityTester,
    STExclusiveChessUndecidableWriterTree,
    STExclusiveChessUndecidableWriterLine,
    STExclusiveChessLegalityTester,
    STUltraschachzwangLegalityTester,
    STIsardamLegalityTester,
    STPatienceChessLegalityTester,
    STMoveTracer,
    STAttackPlayed,
    STDefensePlayed,
    STHelpMovePlayed
};

enum
{
  nr_move_slice_rank_order_elmts = sizeof move_slice_rank_order / sizeof move_slice_rank_order[0]
};

/* Start inserting according to the slice type order for move execution
 * @param si identifies starting point of insertion
 * @param st insertion traversal where we come from and will return to
 * @param end_of_factored_order slice type where to return to insertion defined
 *                              by st
 */
void start_insertion_according_to_move_order(slice_index si,
                                             stip_structure_traversal *st,
                                             slice_type end_of_temporary)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  branch_insert_slices_factored_order(si,
                                      st,
                                      move_slice_rank_order,
                                      nr_move_slice_rank_order_elmts,
                                      end_of_temporary);

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
