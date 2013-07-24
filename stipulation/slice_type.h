#if !defined(STIPULATION_SLICE_TYPE_H)
#define STIPULATION_SLICE_TYPE_H

/* To deal with the complexity of stipulations used in chess problems,
 * their representation in Popeye splits them up into "slices".
 * This enumeration identify the slice types.
 */

#define ENUMERATION_TYPENAME slice_type
#define ENUMERATORS \
  ENUMERATOR(STProxy),                                                  \
  ENUMERATOR(STTemporaryHackFork),                                      \
  /* battle play structural slices */                                 \
    ENUMERATOR(STAttackAdapter),   /* switch from generic play to attack play */ \
    ENUMERATOR(STDefenseAdapter),  /* switch from generic play to defense play */ \
    ENUMERATOR(STReadyForAttack),     /* proxy mark before we start playing attacks */ \
    ENUMERATOR(STReadyForDefense),     /* proxy mark before we start playing defenses */ \
    ENUMERATOR(STNotEndOfBranchGoal),     /* proxy mark after testing whether the branch is ended because it reached a goal */ \
    ENUMERATOR(STNotEndOfBranch),     /* proxy mark after testing whether the branch is ended */ \
    ENUMERATOR(STMinLengthOptimiser), /* don't even try attacks in less than min_length moves */ \
    /* help play structural slices */                                   \
    ENUMERATOR(STHelpAdapter), /* switch from generic play to help play */ \
    ENUMERATOR(STReadyForHelpMove),                                     \
    /* other structural slices */                                       \
    ENUMERATOR(STSetplayFork),                                          \
    ENUMERATOR(STEndOfBranch), /* end of branch, general case (not reflex, not goal) */ \
    ENUMERATOR(STEndOfBranchForced),  /* side at the move is forced to solve fork if possible */ \
    ENUMERATOR(STEndOfBranchGoal), /* end of branch leading to immediate goal */ \
    ENUMERATOR(STEndOfBranchTester), /* test for end of branch only */ \
    ENUMERATOR(STEndOfBranchGoalTester), /* test for end of branch leading to immediate goal only */ \
    ENUMERATOR(STAvoidUnsolvable), /* avoid solving branches that we know are unsolvable */ \
    ENUMERATOR(STResetUnsolvable), /* reset knowledge about unsolvability */ \
    ENUMERATOR(STLearnUnsolvable), /* update knowledge about unsolvability */ \
    ENUMERATOR(STConstraintSolver),  /* stop unless some condition is met */ \
    ENUMERATOR(STConstraintTester),  /* stop unless some condition is met */ \
    ENUMERATOR(STGoalConstraintTester),  /* stop unless some goal has (not) been reached */ \
    ENUMERATOR(STEndOfRoot), /* proxy slice marking the end of the root branch */ \
    ENUMERATOR(STEndOfIntro), /* proxy slice marking the end of the intro branch */ \
    ENUMERATOR(STDeadEnd), /* stop solving if there are no moves left to be played */ \
    ENUMERATOR(STMove),                                                \
    ENUMERATOR(STForEachAttack), /* iterate over each attack */          \
    ENUMERATOR(STForEachDefense), /* iterate over each defense */      \
    ENUMERATOR(STFindAttack), /* find an attack */                     \
    ENUMERATOR(STFindDefense), /* find a defense*/                     \
    ENUMERATOR(STPostMoveIterationInitialiser),                        \
    ENUMERATOR(STMoveEffectJournalUndoer),                             \
    ENUMERATOR(STMovePlayer),                                          \
    ENUMERATOR(STCastlingPlayer),                                      \
    ENUMERATOR(STEnPassantAdjuster),                                   \
    ENUMERATOR(STMovingPawnPromoter),                                  \
    ENUMERATOR(STFootballChessSubsitutor),                             \
    ENUMERATOR(STLandingAfterMovingPawnPromoter),                      \
    ENUMERATOR(STNullMovePlayer),                                      \
    ENUMERATOR(STLandingAfterMovingPieceMovement),                     \
    ENUMERATOR(STLandingAfterMovePlay),                                \
    ENUMERATOR(STAttackPlayed),                                        \
    ENUMERATOR(STDefensePlayed),                                       \
    ENUMERATOR(STHelpMovePlayed),                                      \
    ENUMERATOR(STDummyMove),    /* dummy move */                       \
    ENUMERATOR(STShortSolutionsStart), /* proxy slice marking where we start looking for short battle solutions in line mode */ \
    ENUMERATOR(STIfThenElse),                                          \
    ENUMERATOR(STCheckZigzagLanding),                                  \
    ENUMERATOR(STBlackChecks),                                         \
    ENUMERATOR(STSingleBoxType1LegalityTester),                        \
    ENUMERATOR(STSingleBoxType2LegalityTester),                        \
    ENUMERATOR(STSingleBoxType3PawnPromoter),                          \
    ENUMERATOR(STSingleBoxType3TMovesForPieceGenerator),               \
    ENUMERATOR(STSingleBoxType3LegalityTester),                        \
    ENUMERATOR(STSingleBoxType3IsSquareObserved),                      \
    ENUMERATOR(STIsardamLegalityTester),                               \
    ENUMERATOR(STCirceAssassinPlaceReborn),                            \
    ENUMERATOR(STOwnKingCaptureAvoider),                               \
    ENUMERATOR(STOpponentKingCaptureAvoider),                          \
    ENUMERATOR(STPatienceChessLegalityTester),                         \
    ENUMERATOR(STGoalReachedTester), /* proxy slice marking the start of goal testing */ \
    ENUMERATOR(STGoalMateReachedTester), /* tests whether a mate goal has been reached */ \
    ENUMERATOR(STGoalStalemateReachedTester), /* tests whether a stalemate goal has been reached */ \
    ENUMERATOR(STGoalDoubleStalemateReachedTester), /* tests whether a double stalemate goal has been reached */ \
    ENUMERATOR(STGoalTargetReachedTester), /* tests whether a target goal has been reached */ \
    ENUMERATOR(STGoalCheckReachedTester), /* tests whether a check goal has been reached */ \
    ENUMERATOR(STGoalCaptureReachedTester), /* tests whether a capture goal has been reached */ \
    ENUMERATOR(STGoalSteingewinnReachedTester), /* tests whether a steingewinn goal has been reached */ \
    ENUMERATOR(STGoalEnpassantReachedTester), /* tests whether an en passant goal has been reached */ \
    ENUMERATOR(STGoalDoubleMateReachedTester), /* tests whether a double mate goal has been reached */ \
    ENUMERATOR(STGoalCounterMateReachedTester), /* tests whether a counter-mate goal has been reached */ \
    ENUMERATOR(STGoalCastlingReachedTester), /* tests whether a castling goal has been reached */ \
    ENUMERATOR(STGoalAutoStalemateReachedTester), /* tests whether an auto-stalemate goal has been reached */ \
    ENUMERATOR(STGoalCircuitReachedTester), /* tests whether a circuit goal has been reached */ \
    ENUMERATOR(STGoalExchangeReachedTester), /* tests whether an exchange goal has been reached */ \
    ENUMERATOR(STGoalCircuitByRebirthReachedTester), /* tests whether a circuit by rebirth goal has been reached */ \
    ENUMERATOR(STGoalExchangeByRebirthReachedTester), /* tests whether an "exchange B" goal has been reached */ \
    ENUMERATOR(STGoalAnyReachedTester), /* tests whether an any goal has been reached */ \
    ENUMERATOR(STGoalProofgameReachedTester), /* tests whether a proof game goal has been reached */ \
    ENUMERATOR(STGoalAToBReachedTester), /* tests whether an "A to B" goal has been reached */ \
    ENUMERATOR(STGoalMateOrStalemateReachedTester), /* just a placeholder - we test using the mate and stalemate testers */ \
    ENUMERATOR(STGoalChess81ReachedTester), /* tests whether an Chess81 goal has been reached */ \
    ENUMERATOR(STGoalImmobileReachedTester), /* auxiliary slice testing whether a side is immobile */ \
    ENUMERATOR(STGoalNotCheckReachedTester), /* auxiliary slice enforcing that a side is not in check */ \
    /* boolean logic */                                                 \
    ENUMERATOR(STTrue),            /* true leaf slice */                \
    ENUMERATOR(STFalse),           /* false leaf slice */               \
    ENUMERATOR(STAnd),      /* logical AND */                           \
    ENUMERATOR(STOr),       /* logical OR */                            \
    ENUMERATOR(STNot),             /* logical NOT */                    \
    /* auxiliary slices */                                              \
    ENUMERATOR(STSelfCheckGuard),  /* stop when a side has exposed its king */ \
    ENUMERATOR(STOhneschachStopIfCheck),  /* stop when a side has attacked the opponent's king */ \
    ENUMERATOR(STOhneschachStopIfCheckAndNotMate),  /* stop when a side has attacked the opponent's king */ \
    ENUMERATOR(STOhneschachDetectUndecidableGoal),                      \
    ENUMERATOR(STRecursionStopper), /* avoid infinite recursions */     \
    ENUMERATOR(STMoveInverter),    /* inverts side to move */           \
    ENUMERATOR(STMinLengthGuard), /* make sure that the minimum length of a branch is respected */  \
    ENUMERATOR(STForkOnRemaining),     /* fork depending on the number of remaining moves */ \
    /* solver slices */                                                 \
    ENUMERATOR(STFindShortest), /* find the shortest continuation(s) */                                  \
    ENUMERATOR(STFindByIncreasingLength), /* find all solutions */      \
    ENUMERATOR(STGeneratingMoves),  /* proxy marking start of move generation */ \
    ENUMERATOR(STDoneGeneratingMoves),  /* proxy marking end of move generation */ \
    ENUMERATOR(STDoneRemovingIllegalMoves),                             \
    ENUMERATOR(STDoneRemovingFutileMoves),                              \
    ENUMERATOR(STDonePriorisingMoves),                                  \
    ENUMERATOR(STMoveGenerator), /* unoptimised move generator */       \
    ENUMERATOR(STKingMoveGenerator), /* move generator for king moves */ \
    ENUMERATOR(STNonKingMoveGenerator), /* move generator for other moves */ \
    ENUMERATOR(STCastlingIntermediateMoveGenerator), /* generates intermediate castling moves */ \
    ENUMERATOR(STCastlingIntermediateMoveLegalityTester), /* tests the legality of intermediate castling moves */ \
    ENUMERATOR(STCastlingRightsAdjuster),                                \
    ENUMERATOR(STMoveGeneratorFork),                                     \
    ENUMERATOR(STGeneratingMovesForPiece),                               \
    ENUMERATOR(STMovesForPieceBasedOnWalkGenerator),                     \
    ENUMERATOR(STCastlingGenerator),                                     \
    ENUMERATOR(STRefutationsAllocator), /* (de)allocate the table holding the refutations */ \
    ENUMERATOR(STRefutationsSolver), /* find battle play refutations */ \
    ENUMERATOR(STRefutationsFilter), /* only consider refutations */ \
    ENUMERATOR(STEndOfRefutationSolvingBranch), /* end of special branch */ \
    ENUMERATOR(STPlaySuppressor), /* suppresses play */                 \
    ENUMERATOR(STContinuationSolver), /* solves battle play continuations */ \
    ENUMERATOR(STSolvingContinuation), /* proxy slice representing STContinuationSolver in testing mode */ \
    ENUMERATOR(STThreatSolver), /* solves threats */                    \
    ENUMERATOR(STThreatEnforcer), /* filters out defense that don't defend against the threat(s) */ \
    ENUMERATOR(STThreatStart), /* proxy slice marking where to start solving threats */ \
    ENUMERATOR(STThreatEnd), /* proxy slice marking where to end solving threats */ \
    ENUMERATOR(STThreatCollector), /* collects threats */               \
    ENUMERATOR(STThreatDefeatedTester), /* detect whether threats have been defeated  */ \
    ENUMERATOR(STRefutationsCollector), /* collects refutations */      \
    ENUMERATOR(STRefutationsAvoider), /* aovids refutations while solving variations */      \
    ENUMERATOR(STLegalAttackCounter), /* counts legal moves */          \
    ENUMERATOR(STLegalDefenseCounter), /* counts legal moves */         \
    ENUMERATOR(STAnyMoveCounter), /* counts moves whether legal or not */            \
    ENUMERATOR(STCaptureCounter), /* counts captures */                 \
    /* slices enforcing prerequisites of some stipulations */           \
    ENUMERATOR(STTestingPrerequisites),  /* proxy marking start of prerequisites tests */ \
    ENUMERATOR(STDoubleMateFilter),  /* enforces precondition for doublemate */ \
    ENUMERATOR(STCounterMateFilter),  /* enforces precondition for counter-mate */ \
    ENUMERATOR(STPrerequisiteOptimiser), /* optimise if prerequisites are not met */ \
    /* slices implementing user options */                              \
    ENUMERATOR(STNoShortVariations), /* filters out short variations */ \
    ENUMERATOR(STRestartGuard),    /* write move numbers */             \
    ENUMERATOR(STRestartGuardIntelligent), /* filter out too short solutions */ \
    ENUMERATOR(STIntelligentTargetCounter), /* count target positions */ \
    ENUMERATOR(STMaxTimeGuard), /* deals with option maxtime */         \
    ENUMERATOR(STMaxSolutionsInitialiser), /* initialise solution counter for option maxsolutions */  \
    ENUMERATOR(STMaxSolutionsGuard), /* deals with option maxsolutions */  \
    ENUMERATOR(STMaxSolutionsCounter), /* count solutions for option maxsolutions */  \
    /* slices implementing optimisations */                             \
    ENUMERATOR(STEndOfBranchGoalImmobile), /* end of branch leading to "immobile goal" (#, =, ...) */ \
    ENUMERATOR(STDeadEndGoal), /* like STDeadEnd, but all ends are goals */ \
    ENUMERATOR(STOrthodoxMatingMoveGenerator),                          \
    ENUMERATOR(STOrthodoxMatingKingContactGenerator),                   \
    ENUMERATOR(STKillerAttackCollector), /* remember killer defenses */ \
    ENUMERATOR(STKillerDefenseCollector), /* remember killer moves */   \
    ENUMERATOR(STKillerMovePrioriser), /* priorise killer move */ \
    ENUMERATOR(STKillerMoveFinalDefenseMove), /* priorise killer move */ \
    ENUMERATOR(STEnPassantFilter),  /* enforces precondition for goal ep */ \
    ENUMERATOR(STEnPassantRemoveNonReachers),                           \
    ENUMERATOR(STCastlingFilter),  /* enforces precondition for goal castling */ \
    ENUMERATOR(STCastlingRemoveNonReachers),                            \
    ENUMERATOR(STChess81RemoveNonReachers),                             \
    ENUMERATOR(STCaptureRemoveNonReachers),                             \
    ENUMERATOR(STTargetRemoveNonReachers),                              \
    ENUMERATOR(STDetectMoveRetracted),                                  \
    ENUMERATOR(STRetractionPrioriser),                                  \
    ENUMERATOR(STUncapturableRemoveCaptures),                           \
    ENUMERATOR(STNocatpureRemoveCaptures),                              \
    ENUMERATOR(STWoozlesRemoveIllegalCaptures),                         \
    ENUMERATOR(STImmuneRemoveCapturesOfImmune),                         \
    ENUMERATOR(STProvocateursRemoveUnobservedCaptures),                 \
    ENUMERATOR(STLortapRemoveSupportedCaptures),                        \
    ENUMERATOR(STPatrolRemoveUnsupportedCaptures),                      \
    ENUMERATOR(STUltraPatrolMovesForPieceGenerator),                    \
    ENUMERATOR(STGenevaRemoveIllegalCaptures),                          \
    ENUMERATOR(STNoPromotionsRemovePromotionMoving),                    \
    ENUMERATOR(STNoPromotionsRemovePromotionReborn),                    \
    ENUMERATOR(STGridRemoveIllegalMoves),                               \
    ENUMERATOR(STMonochromeRemoveBichromeMoves),                        \
    ENUMERATOR(STBichromeRemoveMonochromeMoves),                        \
    ENUMERATOR(STEdgeMoverRemoveIllegalMoves),                          \
    ENUMERATOR(STSuperguardsRemoveIllegalCaptures),                     \
    ENUMERATOR(STKoekoLegalityTester),                                  \
    ENUMERATOR(STGridContactLegalityTester),                            \
    ENUMERATOR(STAntiKoekoLegalityTester),                              \
    ENUMERATOR(STNewKoekoRememberContact),                              \
    ENUMERATOR(STNewKoekoLegalityTester),                               \
    ENUMERATOR(STJiggerLegalityTester),                                 \
    ENUMERATOR(STTakeAndMakeGenerateMake),                              \
    ENUMERATOR(STTakeAndMakeAvoidPawnMakeToBaseLine),                   \
    ENUMERATOR(STWormholeTransferer),                                   \
    ENUMERATOR(STWormholeTransferedPromoter),                           \
    ENUMERATOR(STHashOpener),                                           \
    ENUMERATOR(STAttackHashed),    /* hash table support for attack */  \
    ENUMERATOR(STAttackHashedTester),  /* attack play with hash table */      \
    ENUMERATOR(STHelpHashed),      /* help play with hash table */      \
    ENUMERATOR(STHelpHashedTester),      /* help play with hash table */      \
    ENUMERATOR(STIntelligentMovesLeftInitialiser), /* count the moves left for both sides */ \
    ENUMERATOR(STIntelligentMateFilter), /* solve mate problems in intelligent mode */ \
    ENUMERATOR(STIntelligentStalemateFilter), /* solve stalemate problems in intelligent mode */ \
    ENUMERATOR(STIntelligentProof), /* solve proof games and A=>B in intelligent mode */ \
    ENUMERATOR(STGoalReachableGuardFilterMate), /* goal still reachable in intelligent mode? */ \
    ENUMERATOR(STGoalReachableGuardFilterStalemate), /* goal still reachable in intelligent mode? */ \
    ENUMERATOR(STGoalReachableGuardFilterProof), /* goal still reachable in intelligent mode? */ \
    ENUMERATOR(STGoalReachableGuardFilterProofFairy), /* goal still reachable in intelligent mode? */ \
    ENUMERATOR(STIntelligentSolutionsPerTargetPosCounter), /* count solutions per target position */ \
    ENUMERATOR(STIntelligentLimitNrSolutionsPerTargetPos), /* limit number of solutions per target position */ \
    ENUMERATOR(STIntelligentDuplicateAvoider), /* avoid double solutions in intelligent mode */ \
    ENUMERATOR(STIntelligentImmobilisationCounter), /* determine how immobile the side to be stalemated is already */ \
    ENUMERATOR(STKeepMatingFilter), /* deals with option KeepMatingPiece */ \
    ENUMERATOR(STMaxFlightsquares), /* deals with option MaxFlightsquares */ \
    ENUMERATOR(STFlightsquaresCounter), /* deals with option MaxFlightsquares */ \
    ENUMERATOR(STDegenerateTree),  /* degenerate tree optimisation */   \
    ENUMERATOR(STMaxNrNonTrivial), /* deals with option NonTrivial */   \
    ENUMERATOR(STMaxNrNonTrivialCounter), /* deals with option NonTrivial */ \
    ENUMERATOR(STMaxThreatLength), /* deals with option Threat */       \
    ENUMERATOR(STMaxThreatLengthStart), /* where should STMaxThreatLength start looking for threats */ \
    ENUMERATOR(STStopOnShortSolutionsInitialiser), /* intialise stoponshortsolutions machinery */  \
    ENUMERATOR(STStopOnShortSolutionsFilter), /* enforce option stoponshortsolutions */  \
    ENUMERATOR(STAmuMateFilter), /* detect whether AMU prevents a mate */ \
    ENUMERATOR(STUltraschachzwangLegalityTester),                      \
    ENUMERATOR(STCirceSteingewinnFilter), /* is 'won' piece reborn? */ \
    ENUMERATOR(STCirceCircuitSpecial), /* has reborn capturee made a circuit? */ \
    ENUMERATOR(STCirceExchangeSpecial), /* has reborn capturee made an exchange? */ \
    ENUMERATOR(STAnticirceTargetSquareFilter), /* target square is not reached by capture */ \
    ENUMERATOR(STAnticirceCircuitSpecial), /* special circuit by rebirth */ \
    ENUMERATOR(STAnticirceExchangeSpecial), /* special exchange by rebirth */ \
    ENUMERATOR(STAnticirceExchangeFilter), /* only apply special test after capture in Anticirce */ \
    ENUMERATOR(STPiecesParalysingMateFilter), /* goal not reached because of special rule? */ \
    ENUMERATOR(STPiecesParalysingMateFilterTester), /* goal not reached because of special rule? */ \
    ENUMERATOR(STPiecesParalysingStalemateSpecial), /* stalemate by special rule? */ \
    ENUMERATOR(STPiecesParalysingRemoveCaptures),                     \
    ENUMERATOR(STPiecesParalysingSuffocationFinderFork),              \
    ENUMERATOR(STPiecesParalysingSuffocationFinder),                  \
    ENUMERATOR(STPiecesKamikazeTargetSquareFilter), /* target square not reached because of capture by Kamikaze piece? */ \
    ENUMERATOR(STPiecesHalfNeutralRecolorer),                         \
    ENUMERATOR(STMadrasiMovesForPieceGenerator),                      \
    ENUMERATOR(STEiffelMovesForPieceGenerator),                       \
    ENUMERATOR(STDisparateMovesForPieceGenerator),                    \
    ENUMERATOR(STParalysingMovesForPieceGenerator),                   \
    ENUMERATOR(STCentralMovesForPieceGenerator),                      \
    ENUMERATOR(STBeamtenMovesForPieceGenerator),                      \
    ENUMERATOR(STImmobilityTester),                                   \
    ENUMERATOR(STOpponentMovesCounterFork),                           \
    ENUMERATOR(STOpponentMovesCounter),                               \
    ENUMERATOR(STOpponentMovesFewMovesPrioriser),                     \
    ENUMERATOR(STStrictSATInitialiser), \
    ENUMERATOR(STStrictSATUpdater), \
    ENUMERATOR(STDynastyKingSquareUpdater), \
    ENUMERATOR(STHurdleColourChanger), \
    ENUMERATOR(STOscillatingKingsTypeA), \
    ENUMERATOR(STOscillatingKingsTypeB), \
    ENUMERATOR(STOscillatingKingsTypeC), \
    ENUMERATOR(STMovingPawnToImitatorPromoter),                       \
    /* other slices related to fairy chess */                         \
    ENUMERATOR(STExclusiveChessExclusivityDetector),                  \
    ENUMERATOR(STExclusiveChessNestedExclusivityDetector),            \
    ENUMERATOR(STExclusiveChessLegalityTester),                       \
    ENUMERATOR(STExclusiveChessMatingMoveCounterFork),                \
    ENUMERATOR(STExclusiveChessGoalReachingMoveCounter),              \
    ENUMERATOR(STExclusiveChessUndecidableWriterTree),                \
    ENUMERATOR(STExclusiveChessUndecidableWriterLine),                \
    ENUMERATOR(STExclusiveChessGoalReachingMoveCounterSelfCheckGuard),\
    ENUMERATOR(STMaffImmobilityTesterKing), \
    ENUMERATOR(STOWUImmobilityTesterKing), \
    ENUMERATOR(STBrunnerDefenderFinder), \
    ENUMERATOR(STKingCaptureLegalityTester), \
    ENUMERATOR(STCageCirceNonCapturingMoveFinder), \
    ENUMERATOR(STSinglePieceMoveGenerator),                          \
    ENUMERATOR(STSingleMoveGenerator), \
    ENUMERATOR(STMummerOrchestrator),                                   \
    ENUMERATOR(STMummerBookkeeper),                                     \
    ENUMERATOR(STMummerDeadend),                                        \
    ENUMERATOR(STUltraMummerMeasurerFork),                              \
    ENUMERATOR(STUltraMummerMeasurerDeadend),                           \
    ENUMERATOR(STBackHomeFinderFork),                                   \
    ENUMERATOR(STBackHomeMovesOnly),                                    \
    ENUMERATOR(STCheckTesterFork),                                      \
    ENUMERATOR(STTestingCheck),                                         \
    ENUMERATOR(STVogtlaenderCheckTester),                               \
    ENUMERATOR(STExtinctionCheckTester),                                \
    ENUMERATOR(STNoKingCheckTester),                                    \
    ENUMERATOR(STSATCheckTester),                                       \
    ENUMERATOR(STSATxyCheckTester),                                     \
    ENUMERATOR(STStrictSATCheckTester),                                 \
    ENUMERATOR(STAssassinCirceCheckTester),                             \
    ENUMERATOR(STKingSquareObservationTesterPlyInitialiser),            \
    ENUMERATOR(STBicoloresCheckTester),                                 \
    ENUMERATOR(STAntikingsCheckTester),                                 \
    ENUMERATOR(STKingSquareObservationTester),                          \
    ENUMERATOR(STBGLEnforcer),                                          \
    ENUMERATOR(STMasandRecolorer),                                      \
    ENUMERATOR(STMessignyMovePlayer),                                   \
    ENUMERATOR(STActuatedRevolvingCentre),                              \
    ENUMERATOR(STActuatedRevolvingBoard),                               \
    ENUMERATOR(STRepublicanKingPlacer),                                 \
    ENUMERATOR(STRepublicanType1DeadEnd),                               \
    ENUMERATOR(STCirceCaptureFork),                                     \
    ENUMERATOR(STAprilAprilFork),                                       \
    ENUMERATOR(STCirceDetermineRebornPiece),                            \
    ENUMERATOR(STCirceCloneDetermineRebornPiece),                       \
    ENUMERATOR(STCirceDoubleAgentsAdaptRebornPiece),                    \
    ENUMERATOR(STChameleonCirceAdaptRebornPiece),                       \
    ENUMERATOR(STAntiEinsteinDetermineRebornPiece),                     \
    ENUMERATOR(STCirceDetermineRelevantPiece),                          \
    ENUMERATOR(STCirceCouscousDetermineRelevantPiece),                  \
    ENUMERATOR(STCirceDetermineRebirthSquare),                          \
    ENUMERATOR(STTakeMakeCirceDetermineRebirthSquares),                 \
    ENUMERATOR(STTakeMakeCirceNoRebirth),                               \
    ENUMERATOR(STTakeMakeCirceCollectRebirthSquaresFork),               \
    ENUMERATOR(STTakeMakeCirceCollectRebirthSquares),                   \
    ENUMERATOR(STContactGridAvoidCirceRebirth),                         \
    ENUMERATOR(STCircePlaceReborn),                                     \
    ENUMERATOR(STSuperCirceNoRebirthFork),                              \
    ENUMERATOR(STSuperCirceRebirthHandler),                             \
    ENUMERATOR(STCirceCageNoCageFork)    ,                              \
    ENUMERATOR(STCirceCageCageTester),                                  \
    ENUMERATOR(STLandingAfterAnticirceRebirth),                         \
    ENUMERATOR(STCirceParrainDetermineRebirth),                         \
    ENUMERATOR(STLandingAfterCirceRebirthHandler),                      \
    ENUMERATOR(STCirceVolageRecolorer),                                 \
    ENUMERATOR(STCirceRebirthPromoter),                                 \
    ENUMERATOR(STAnticirceCaptureFork),                                 \
    ENUMERATOR(STCirceKamikazeCaptureFork),                             \
    ENUMERATOR(STAnticirceDetermineRebornPiece),                        \
    ENUMERATOR(STAnticirceDetermineRevelantPiece),                      \
    ENUMERATOR(STAnticirceCouscousDetermineRevelantPiece),              \
    ENUMERATOR(STAnticirceDetermineRebirthSquare),                      \
    ENUMERATOR(STAnticirceCheylanFilter),                               \
    ENUMERATOR(STAnticircePlaceRebornRelaxed),                          \
    ENUMERATOR(STAnticircePlaceRebornStrict),                           \
    ENUMERATOR(STAnticirceRebornPromoter),                              \
    ENUMERATOR(STAntisupercirceDetermineRebirthSquare),                 \
    ENUMERATOR(STSentinellesInserter),                                  \
    ENUMERATOR(STMagicViewsInitialiser),                                \
    ENUMERATOR(STMagicPiecesRecolorer),                                 \
    ENUMERATOR(STSingleboxType2LatentPawnSelector),                     \
    ENUMERATOR(STSingleboxType2LatentPawnPromoter),                     \
    ENUMERATOR(STDuellistsRememberDuellist),                            \
    ENUMERATOR(STHauntedChessGhostSummoner),                            \
    ENUMERATOR(STHauntedChessGhostRememberer),                          \
    ENUMERATOR(STGhostChessGhostRememberer),                            \
    ENUMERATOR(STKobulKingSubstitutor),                                 \
    ENUMERATOR(STAndernachSideChanger),                                 \
    ENUMERATOR(STAntiAndernachSideChanger),                             \
    ENUMERATOR(STChameleonPursuitSideChanger),                          \
    ENUMERATOR(STNorskRemoveIllegalCaptures),                           \
    ENUMERATOR(STNorskArrivingAdjuster),                                \
    ENUMERATOR(STProteanPawnAdjuster),                                  \
    ENUMERATOR(STEinsteinArrivingAdjuster),                             \
    ENUMERATOR(STReverseEinsteinArrivingAdjuster),                      \
    ENUMERATOR(STAntiEinsteinArrivingAdjuster),                         \
    ENUMERATOR(STEinsteinEnPassantAdjuster),                            \
    ENUMERATOR(STTraitorSideChanger),                                   \
    ENUMERATOR(STVolageSideChanger),                                    \
    ENUMERATOR(STMagicSquareSideChanger),                               \
    ENUMERATOR(STMagicSquareType2SideChanger),                          \
    ENUMERATOR(STMagicSquareType2AnticirceRelevantSideAdapter),         \
    ENUMERATOR(STCirceTraitorSideChanger),                              \
    ENUMERATOR(STTibetSideChanger),                                     \
    ENUMERATOR(STDoubleTibetSideChanger),                               \
    ENUMERATOR(STDegradierungDegrader),                                 \
    ENUMERATOR(STPromoteMovingIntoChameleon),                           \
    ENUMERATOR(STPromoteCirceRebornIntoChameleon),                      \
    ENUMERATOR(STPromoteAnticirceRebornIntoChameleon),                  \
    ENUMERATOR(STChameleonArrivingAdjuster),                            \
    ENUMERATOR(STChameleonChessArrivingAdjuster),                       \
    ENUMERATOR(STLineChameleonArrivingAdjuster),                        \
    ENUMERATOR(STFrischaufPromoteeMarker),                              \
    ENUMERATOR(STPhantomChessEnPassantAdjuster),                        \
    ENUMERATOR(STPhantomMovesForPieceGenerator),                        \
    ENUMERATOR(STPlusMovesForPieceGenerator),                           \
    ENUMERATOR(STMarsCirceMovesForPieceGenerator),                      \
    ENUMERATOR(STAntiMarsCirceMovesForPieceGenerator),                  \
    ENUMERATOR(STAntiMarsCirceEnPassantAdjuster),                       \
    ENUMERATOR(STKamikazeCapturingPieceRemover),                        \
    ENUMERATOR(STHaanChessHoleInserter),                                \
    ENUMERATOR(STCastlingChessMovePlayer),                              \
    ENUMERATOR(STExchangeCastlingMovePlayer),                           \
    ENUMERATOR(STSuperTransmutingKingTransmuter),                       \
    ENUMERATOR(STReflectiveKingsMovesForPieceGenerator),                \
    ENUMERATOR(STVaultingKingsMovesForPieceGenerator),                  \
    ENUMERATOR(STTransmutingKingsMovesForPieceGenerator),               \
    ENUMERATOR(STSuperTransmutingKingsMovesForPieceGenerator),          \
    ENUMERATOR(STStingMovesForPieceGenerator),                          \
    ENUMERATOR(STStingIsSquareObserved),                                \
    ENUMERATOR(STCastlingChessMovesForPieceGenerator),                  \
    ENUMERATOR(STPlatzwechselRochadeMovesForPieceGenerator),            \
    ENUMERATOR(STMessignyMovesForPieceGenerator),                       \
    ENUMERATOR(STAnnanMovesForPieceGenerator),                          \
    ENUMERATOR(STAMUAttackCounter),                                     \
    ENUMERATOR(STMutualCastlingRightsAdjuster),                         \
    ENUMERATOR(STImitatorMover),                                        \
    ENUMERATOR(STImitatorRemoveIllegalMoves),                           \
    ENUMERATOR(STBlackChecksNullMoveGenerator),                         \
    /* output slices */                                                 \
    ENUMERATOR(STOutputModeSelector), /* select an output mode for the subsequent play */ \
    ENUMERATOR(STIllegalSelfcheckWriter), /* indicate illegal self-check in the diagram position */ \
    ENUMERATOR(STEndOfPhaseWriter), /* write the end of a phase */      \
    ENUMERATOR(STEndOfSolutionWriter), /* write the end of a solution */  \
    ENUMERATOR(STThreatWriter), /* writes "threat:" */ \
    ENUMERATOR(STMoveWriter), /* writes moves */ \
    ENUMERATOR(STKeyWriter), /* write battle play keys */               \
    ENUMERATOR(STTryWriter), /* write "but" */                          \
    ENUMERATOR(STZugzwangWriter), /* writes zugzwang if appropriate */  \
    ENUMERATOR(STTrivialEndFilter), /* don't write trivial variations */  \
    ENUMERATOR(STRefutingVariationWriter), /* writes refuting variations */ \
    ENUMERATOR(STRefutationsIntroWriter), /* write "But" */  \
    ENUMERATOR(STRefutationWriter), /* writes refutations */  \
    ENUMERATOR(STOutputPlaintextTreeCheckWriter), /* plain text output, tree mode: write checks by the previous move */  \
    ENUMERATOR(STOutputPlaintextLineLineWriter), /* plain text output, line mode: write a line */  \
    ENUMERATOR(STOutputPlaintextGoalWriter), /* plain text output, tree mode: write the reached goal */  \
    ENUMERATOR(STOutputPlaintextMoveInversionCounter), /* plain text output: count move inversions */  \
    ENUMERATOR(STOutputPlaintextLineEndOfIntroSeriesMarker), /* handles the end of the intro series */  \
    /* square observation tester slices */                              \
    ENUMERATOR(STIsSquareObservedFork),                                 \
    ENUMERATOR(STTestingIfSquareIsObserved),                            \
    ENUMERATOR(STFindSquareObserverTrackingBack),                       \
    ENUMERATOR(STFindSquareObserverTrackingBackKing),                   \
    ENUMERATOR(STFindSquareObserverTrackingBackFairy),                  \
    ENUMERATOR(STLandingAfterFindSquareObserverTrackingBackKing),       \
    ENUMERATOR(STVaultingKingIsSquareObserved),                         \
    ENUMERATOR(STTransmutingKingIsSquareObserved),                      \
    ENUMERATOR(STReflectiveKingIsSquareObserved),                       \
    ENUMERATOR(STAnnanIsSquareObserved),                                \
    ENUMERATOR(STPhantomIsSquareObserved),                              \
    ENUMERATOR(STPlusIsSquareObserved),                                 \
    ENUMERATOR(STMarsIsSquareObserved),                                 \
    ENUMERATOR(STIsSquareObservedOrtho),                                \
    ENUMERATOR(STObservationGeometryTesterFork),                        \
    ENUMERATOR(STTestingObservationGeometry),                           \
    ENUMERATOR(STTestObservationGeometryGridChess),                     \
    ENUMERATOR(STTestObservationGeometryEdgeMover),                     \
    ENUMERATOR(STTestObservationGeometryMonochrome),                    \
    ENUMERATOR(STTestObservationGeometryBichrome),                      \
    ENUMERATOR(STTestObservationGeometryByPlayingMove),                 \
    /* debugging slices */                                              \
    ENUMERATOR(STMoveTracer),                                           \
    ENUMERATOR(STMoveCounter),                                          \
    ENUMERATOR(nr_slice_types),                                         \
    ASSIGNED_ENUMERATOR(no_slice_type = nr_slice_types)

#define ENUMERATION_DECLARE
#include "utilities/enumeration.h"

#endif
