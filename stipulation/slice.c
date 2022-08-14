#include "stipulation/slice.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

static slice_structural_type highest_structural_type[nr_slice_types];

static slice_type const leaf_slice_types[] =
{
    STFalse,
    STTrue,
    STGeneratedMovesForPiece,
    STTestedCheck
};

static slice_type const branch_slice_types[] =
{
    STAttackAdapter,
    STDefenseAdapter,
    STReadyForAttack,
    STReadyForDefense,
    STMinLengthOptimiser,
    STHelpAdapter,
    STReadyForHelpMove,
    STMinLengthGuard,
    STFindShortest,
    STFindByIncreasingLength,
    STAttackHashed,
    STHelpHashed,
    STDegenerateTree,
    STStopOnShortSolutionsFilter
};

static slice_type const fork_slice_types[] =
{
    STStipulationCopier,
    STEndOfBranchGoal,
    STEndOfBranchGoalImmobile,
    STAvoidUnsolvable,
    STIfThenElse,
    STAnd,
    STOr,
    STForkOnRemaining,
    STRefutationsSolver,
    STTemporaryHackFork,
    STSetplayFork,
    STEndOfBranch,
    STEndOfBranchForced,
    STEndOfBranchTester,
    STEndOfBranchGoalTester,
    STConstraintSolver,
    STConstraintTester,
    STGoalConstraintTester,
    STGoalReachedTester,
    STLostPiecesTester,
    STGoalImmobileReachedTester,
    STCastlingIntermediateMoveLegalityTester,
    STContinuationSolver,
    STThreatSolver,
    STThreatEnforcer,
    STDoubleMateFilter,
    STCounterMateFilter,
    STNoShortVariations,
    STIntelligentMateTargetPositionTester,
    STIntelligentStalemateTargetPositionTester,
    STMaxFlightsquares,
    STMaxNrNonTrivial,
    STMaxThreatLength,
    STOpponentMovesCounterFork,
    STExclusiveChessMatingMoveCounterFork,
    STBrunnerDefenderFinder,
    STKingCaptureLegalityTester,
    STMoveLegalityTester,
    STCageCirceNonCapturingMoveFinder,
    STTakeMakeCirceCollectRebirthSquaresFork,
    STMummerOrchestrator,
    STUltraMummerMeasurerFork,
    STBackHomeFinderFork,
    STPiecesParalysingSuffocationFinderFork,
    STCheckTesterFork,
    STTrivialEndFilter,
    STNullMovePlayer,
    STRoleExchangeMovePlayer,
    STCastlingPlayer,
    STMessignyMovePlayer,
    STCastlingChessMovePlayer,
    STExchangeCastlingMovePlayer,
    STCirceCaptureFork,
    STCirceParrainThreatFork,
    STCircePreventKingRebirth,
    STCirceTestRebirthSquareEmpty,
    STCirceKamikazeCaptureFork,
    STAprilCaptureFork,
    STCirceCageNoCageFork,
    STSuperCirceNoRebirthFork,
    STPawnToImitatorPromoter,
    STKillerMoveFinalDefenseMove,
    STOhneschachStopIfCheckAndNotMate,
    STMoveGeneratorFork,
    STIsSquareObservedFork,
    STTransmutingKingIsSquareObserved,
    STVaultingKingIsSquareObserved,
    STValidatingCheckFork,
    STValidatingObservationFork,
    STValidatingObserverFork,
    STValidatingObservationGeometryFork,
    STMoveForPieceGeneratorTwoPaths,
    STIsSquareObservedTwoPaths,
    STOpponentMovesFewMovesPrioriser
};

static void init_one_highest_structural_type(slice_type const slice_types[],
                                             unsigned int nr_slice_types,
                                             slice_structural_type type)
{
  unsigned int i;

  for (i = 0; i!=nr_slice_types; ++i)
    highest_structural_type[slice_types[i]] = type;
}

static void init_highest_structural_type(void)
{
  /* no Trace instrumentation here - this is used by the Trace machinery! */
  static boolean initialised = false;

  if (!initialised)
  {
    initialised = true;

    /* default value is slice_structure_pipe - override for other types */
#define init_one_type(type) init_one_highest_structural_type(type##_slice_types, \
                                                             sizeof type##_slice_types / sizeof type##_slice_types[0], \
                                                             slice_structure_##type)
    init_one_type(leaf);
    init_one_type(branch);
    init_one_type(fork);
#undef init_one_type
  }
}

/* Retrieve the structural type of a slice type
 * @param type identifies slice type of which to retrieve structural type
 * @return structural type of slice type type
 */
slice_structural_type slice_type_get_structural_type(slice_type type)
{
  /* no Trace instrumentation here - this is used by the Trace machinery! */
  assert(type<=nr_slice_types);
  return highest_structural_type[type];
}

static slice_functional_type functional_type[nr_slice_types];

static slice_contextual_type contextual_type[nr_slice_types];

static slice_type const proxy_slice_types[] =
{
    STProxy,
    STReadyForAttack,
    STReadyForDefense,
    STNotEndOfBranchGoal,
    STNotEndOfBranch,
    STReadyForHelpMove,
    STEndOfRoot,
    STEndOfIntro,
    STMove,
    STLandingAfterMovingPieceMovement,
    STLandingAfterMovePlay,
    STShortSolutionsStart,
    STCheckZigzagLanding,
    STGoalMateReachedTester,
    STGoalStalemateReachedTester,
    STGoalDoubleStalemateReachedTester,
    STGoalAutoStalemateReachedTester,
    STGoalAutoMateReachedTester,
    STImmobilityTester,
    STGeneratingMoves,
    STExecutingKingCapture,
    STSkipMoveGeneration,
    STDoneGeneratingMoves,
    STDoneRemovingIllegalMoves,
    STDoneRemovingFutileMoves,
    STDonePriorisingMoves,
    STEndOfRefutationSolvingBranch,
    STSolvingContinuation,
    STThreatStart,
    STThreatEnd,
    STTestingPrerequisites,
    STMaxThreatLengthStart,
    STOutputModeSelector,
    STCirceConsideringRebirth,
    STGenevaConsideringRebirth,
    STMarsCirceConsideringRebirth,
    STMarsCirceConsideringObserverRebirth,
    STAntimarsCirceConsideringRebirth,
    STCirceDeterminingRebirth,
    STCirceDeterminedRebirth,
    STCirceRebirthAvoided,
    STCirceRebirthOnNonEmptySquare,
    STCircePlacingReborn,
    STAnticirceConsideringRebirth,
    STBeforePawnPromotion,
    STLandingAfterPawnPromotion,
    STMummerDeadend,
    STGeneratingMovesForPiece,
    STGeneratedMovesForPiece,
    STTestingCheck,
    STTestedCheck,
    STTestingIfSquareIsObserved,
    STTestingIfSquareIsObservedWithSpecificWalk,
    STOptimisingObserverWalk,
    STValidatingCheck,
    STValidatingObservation,
    STValidatingObserver,
    STValidatingObservationGeometry,
    STMoveForPieceGeneratorPathsJoint,
    STMoveForPieceGeneratorAlternativePath,
    STMoveForPieceGeneratorStandardPath,
    STMoveForPieceGeneratorPathsJoint,
    STIsSquareObservedStandardPath,
    STIsSquareObservedAlternativePath,
    STIsSquareObservedPathsJoint,
    STIntelligentTargetPositionFound,
    STUnsaveableTester
};

static slice_type const move_generator_slice_types[] =
{
    STMoveGenerator,
    STKingMoveGenerator,
    STNonKingMoveGenerator,
    STOrthodoxMatingMoveGenerator,
    STOrthodoxMatingKingContactGenerator,
    STSinglePieceMoveGenerator,
    STNullMoveGenerator,
    STRoleExchangeMoveGenerator
};

static slice_type const move_reordering_optimiser_slice_types[] =
{
    STOpponentMovesFewMovesPrioriser,
    STKillerMovePrioriser,
    STRetractionPrioriser
};

static slice_type const move_removing_optimiser_slice_types[] =
{
    STEnPassantRemoveNonReachers,
    STCastlingRemoveNonReachers,
    STChess81RemoveNonReachers,
    STCaptureRemoveNonReachers,
    STTargetRemoveNonReachers
};

static slice_type const binary_slice_types[] =
{
    STEndOfBranchGoal,
    STEndOfBranchGoalImmobile,
    STAvoidUnsolvable,
    STIfThenElse,
    STAnd,
    STOr,
    STForkOnRemaining,
    STRefutationsSolver,
    STThreatSolver,
    STNullMovePlayer,
    STRoleExchangeMovePlayer,
    STCastlingPlayer,
    STMessignyMovePlayer,
    STCastlingChessMovePlayer,
    STExchangeCastlingMovePlayer,
    STCirceCaptureFork,
    STCirceParrainThreatFork,
    STCircePreventKingRebirth,
    STCirceTestRebirthSquareEmpty,
    STCirceKamikazeCaptureFork,
    STAprilCaptureFork,
    STCirceCageNoCageFork,
    STSuperCirceNoRebirthFork,
    STPawnToImitatorPromoter,
    STKillerMoveFinalDefenseMove,
    STTransmutingKingIsSquareObserved,
    STVaultingKingIsSquareObserved,
    STMoveForPieceGeneratorTwoPaths,
    STIsSquareObservedTwoPaths
};

static slice_type const testing_pipe_slice_types[] =
{
    STContinuationSolver,
    STThreatEnforcer,
    STNoShortVariations,
    STMaxNrNonTrivial,
    STMaxThreatLength,
    STTrivialEndFilter,
    STMummerOrchestrator,
    STOpponentMovesFewMovesPrioriser
};

static slice_type const conditional_pipe_slice_types[] =
{
    STTemporaryHackFork,
    STEndOfBranchTester,
    STEndOfBranchGoalTester,
    STConstraintTester,
    STGoalConstraintTester,
    STGoalReachedTester,
    STLostPiecesTester,
    STGoalImmobileReachedTester,
    STCastlingIntermediateMoveLegalityTester,
    STDoubleMateFilter,
    STCounterMateFilter,
    STIntelligentMateTargetPositionTester,
    STIntelligentStalemateTargetPositionTester,
    STMaxFlightsquares,
    STOpponentMovesCounterFork,
    STExclusiveChessMatingMoveCounterFork,
    STBrunnerDefenderFinder,
    STKingCaptureLegalityTester,
    STMoveLegalityTester,
    STUltraMummerMeasurerFork,
    STBackHomeFinderFork,
    STPiecesParalysingSuffocationFinderFork,
    STCheckTesterFork,
    STCageCirceNonCapturingMoveFinder,
    STTakeMakeCirceCollectRebirthSquaresFork,
    STOhneschachStopIfCheckAndNotMate,
    STMoveGeneratorFork,
    STIsSquareObservedFork,
    STValidatingCheckFork,
    STValidatingObservationFork,
    STValidatingObserverFork,
    STValidatingObservationGeometryFork
};

static slice_type const end_of_branch_slice_types[] =
{
    STEndOfBranch,
    STEndOfBranchForced,
    STConstraintSolver
};

static slice_type const writer_slice_types[] =
{
    STZeropositionSolvingStopper,
    STOutputPlaintextTwinIntroWriter,
    STOutputLaTeXTwinningWriter,
    STIllegalSelfcheckWriter,
    STOutputPlainTextEndOfPhaseWriter,
    STOutputPlainTextTreeEndOfSolutionWriter,
    STOutputPlainTextThreatWriter,
    STOutputPlainTextMoveWriter,
    STOutputPlainTextKeyWriter,
    STOutputPlainTextTryWriter,
    STOutputPlainTextZugzwangByDummyMoveCheckWriter,
    STOutputPlainTextZugzwangWriter,
    STOutputPlaintextTreeRefutingVariationWriter,
    STOutputPlaintextLineRefutingVariationWriter,
    STOutputPlainTextRefutationsIntroWriter,
    STOutputPlainTextRefutationWriter,
    STOutputPlaintextTreeCheckWriter,
    STOutputPlaintextLineLineWriter,
    STOutputPlaintextGoalWriter,
    STOutputPlaintextConstraintWriter,
    STOutputLaTeXLineLineWriter,
    STOutputLaTeXGoalWriter,
    STOutputLaTeXConstraintWriter
};

static slice_type const output_plaintext_position_writer_slice_types[] =
{
    STOutputPlainTextMetaWriter,
    STOutputPlainTextBoardWriter,
    STOutputPlainTextStipulationWriter,
    STOutputPlainTextPieceCountsWriter,
    STOutputPlainTextRoyalPiecePositionsWriter,
    STOutputPlainTextNonRoyalAttributesWriter,
    STOutputPlainTextEndOfPositionWriters,
    STOutputPlainTextAToBIntraWriter,
    STOutputPlainTextConditionsWriter,
    STOutputPlainTextMutuallyExclusiveCastlingsWriter,
    STOutputPlainTextDuplexWriter,
    STOutputPlainTextHalfDuplexWriter,
    STOutputPlainTextQuodlibetWriter,
    STOutputPlainTextGridWriter
};

static void init_one_functional_type(slice_type const slice_types[],
                                     unsigned int nr_slice_types,
                                     slice_functional_type type)
{
  unsigned int i;

  for (i = 0; i!=nr_slice_types; ++i)
    functional_type[slice_types[i]] = type;
}

static void init_functional_type(void)
{
  /* no Trace instrumentation here - this is used by the Trace machinery! */

  /* default value is slice_function_unspecified - override for other types */
#define init_one_type(type) init_one_functional_type(type##_slice_types, \
                                                     sizeof type##_slice_types / sizeof type##_slice_types[0], \
                                                     slice_function_##type)
  init_one_type(proxy);
  init_one_type(move_generator);
  init_one_type(move_reordering_optimiser);
  init_one_type(move_removing_optimiser);
  init_one_type(writer);
  init_one_type(output_plaintext_position_writer);
#undef init_one_type
}

/* Retrieve the functional type of a slice type
 * @param type identifies slice type of which to retrieve structural type
 * @return functional type of slice type type
 */
slice_functional_type slice_type_get_functional_type(slice_type type)
{
  assert(type<=nr_slice_types);
  return functional_type[type];
}

static void init_one_contextual_type(slice_type const slice_types[],
                                     unsigned int nr_slice_types,
                                     slice_contextual_type type)
{
  unsigned int i;

  for (i = 0; i!=nr_slice_types; ++i)
    contextual_type[slice_types[i]] = type;
}

static void init_contextual_type(void)
{
  /* no Trace instrumentation here - this is used by the Trace machinery! */

  /* default value is slice_function_unspecified - override for other types */
#define init_one_type(type) init_one_contextual_type(type##_slice_types, \
                                                     sizeof type##_slice_types / sizeof type##_slice_types[0], \
                                                     slice_contextual_##type)
  init_one_type(binary);
  init_one_type(testing_pipe);
  init_one_type(conditional_pipe);
  init_one_type(end_of_branch);
#undef init_one_type
}

/* Retrieve the contextual type of a slice type
 * @param type identifies slice type of which to retrieve structural type
 * @return contextual type of slice type type
 */
slice_contextual_type slice_type_get_contextual_type(slice_type type)
{
  assert(type<=nr_slice_types);
  return contextual_type[type];
}

/* Initialise slice properties at start of program */
void initialise_slice_properties(void)
{
  init_highest_structural_type();
  init_contextual_type();
  init_functional_type();
}
