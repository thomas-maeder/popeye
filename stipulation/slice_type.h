/* no #include guard here - this file is supposed to be #included multiple times
 * per translation unit */

/* To deal with the complexity of stipulations used in chess problems,
 * their representation in Popeye splits them up into "slices".
 *
 * This head is used in conjunction with pyenum.h, e.g.
#include "stipulation/slice_type.h"
#define ENUMERATION_DECLARE
#include "pyenum.h"
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
    ENUMERATOR(STMinLengthOptimiser), /* don't even try attacks in less than min_length moves */ \
    /* help play structural slices */                                   \
    ENUMERATOR(STHelpAdapter), /* switch from generic play to help play */ \
    ENUMERATOR(STReadyForHelpMove),                                     \
    /* other structural slices */                                       \
    ENUMERATOR(STSetplayFork),                                          \
    ENUMERATOR(STEndOfBranch), /* end of branch, general case (not reflex, not goal) */ \
    ENUMERATOR(STEndOfBranchForced),  /* side at the move is forced to solve fork if possible */ \
    ENUMERATOR(STEndOfBranchGoal), /* end of branch leading to immediate goal */ \
    ENUMERATOR(STConstraint),  /* stop unless some condition is met */ \
    ENUMERATOR(STEndOfRoot), /* proxy slice marking the end of the root branch */ \
    ENUMERATOR(STEndOfIntro), /* proxy slice marking the end of the intro branch */ \
    ENUMERATOR(STDeadEnd), /* stop solving if there are no moves left to be played */ \
    ENUMERATOR(STMove),                                                \
    ENUMERATOR(STDummyMove),    /* dummy move */                       \
    ENUMERATOR(STReadyForDummyMove),                                   \
    ENUMERATOR(STShortSolutionsStart), /* proxy slice marking where we start looking for short battle solutions in line mode */ \
    ENUMERATOR(STCheckZigzagJump),                                     \
    ENUMERATOR(STCheckZigzagLanding),                                  \
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
    ENUMERATOR(STGoalImmobileReachedTester), /* auxiliary slice testing whether a side is immobile */ \
    ENUMERATOR(STGoalNotCheckReachedTester), /* auxiliary slice enforcing that a side is not in check */ \
    /* boolean logic */                                                 \
    ENUMERATOR(STTrue),            /* true leaf slice */                \
    ENUMERATOR(STFalse),           /* false leaf slice */               \
    ENUMERATOR(STAnd),      /* logical AND */                           \
    ENUMERATOR(STOr),       /* logical OR */                            \
    ENUMERATOR(STNot),             /* logical NOT */                    \
    /* auxiliary slices */                                              \
    ENUMERATOR(STCheckDetector), /* detect check delivered by previous move */ \
    ENUMERATOR(STSelfCheckGuard),  /* stop when a side exposes its king */ \
    ENUMERATOR(STMoveInverter),    /* inverts side to move */           \
    ENUMERATOR(STMinLengthGuard), /* make sure that the minimum length of a branch is respected */  \
    ENUMERATOR(STForkOnRemaining),     /* fork depending on the number of remaining moves */ \
    /* solver slices */                                                 \
    ENUMERATOR(STFindShortest), /* find the shortest continuation(s) */                                  \
    ENUMERATOR(STFindByIncreasingLength), /* find all solutions */      \
    ENUMERATOR(STMoveGenerator), /* unoptimised move generator */       \
    ENUMERATOR(STKingMoveGenerator), /* move generator for king moves */ \
    ENUMERATOR(STNonKingMoveGenerator), /* move generator for other moves */ \
    ENUMERATOR(STRefutationsAllocator), /* (de)allocate the table holding the refutations */ \
    ENUMERATOR(STTrySolver), /* find battle play tries */               \
    ENUMERATOR(STRefutationsSolver), /* find battle play refutations */ \
    ENUMERATOR(STPlaySuppressor), /* suppresses play */                 \
    ENUMERATOR(STContinuationSolver), /* solves battle play continuations */ \
    ENUMERATOR(STThreatSolver), /* solves threats */                    \
    ENUMERATOR(STThreatEnforcer), /* filters out defense that don't defend against the threat(s) */ \
    ENUMERATOR(STThreatStart), /* proxy slice marking where to start solving threats */ \
    ENUMERATOR(STThreatCollector), /* collects threats */               \
    ENUMERATOR(STRefutationsCollector), /* collects refutations */      \
    ENUMERATOR(STLegalMoveCounter), /* counts legal moves */            \
    ENUMERATOR(STCaptureCounter), /* counts captures */            \
    /* slices enforcing prerequisites of some stipulations */           \
    ENUMERATOR(STDoubleMateFilter),  /* enforces precondition for doublemate */ \
    ENUMERATOR(STCounterMateFilter),  /* enforces precondition for counter-mate */ \
    ENUMERATOR(STPrerequisiteOptimiser), /* optimise if prerequisites are not met */ \
    /* slices implementing user options */                              \
    ENUMERATOR(STNoShortVariations), /* filters out short variations */ \
    ENUMERATOR(STRestartGuard),    /* write move numbers */             \
    ENUMERATOR(STMaxTimeGuard), /* deals with option maxtime */         \
    ENUMERATOR(STMaxSolutionsInitialiser), /* initialise solution counter for option maxsolutions */  \
    ENUMERATOR(STMaxSolutionsGuard), /* deals with option maxsolutions */  \
    /* slices implementing optimisations */                             \
    ENUMERATOR(STEndOfBranchGoalImmobile), /* end of branch leading to "immobile goal" (#, =, ...) */ \
    ENUMERATOR(STDeadEndGoal), /* like STDeadEnd, but all ends are goals */ \
    ENUMERATOR(STOrthodoxMatingMoveGenerator),                          \
    ENUMERATOR(STKillerMoveCollector), /* remember killer moves */      \
    ENUMERATOR(STKillerMoveMoveGenerator), /* generate attack moves, prioritise killer move (if any) */ \
    ENUMERATOR(STKillerMoveFinalDefenseMove), /* priorise killer move */ \
    ENUMERATOR(STCountNrOpponentMovesMoveGenerator), \
    ENUMERATOR(STEnPassantFilter),  /* enforces precondition for goal ep */ \
    ENUMERATOR(STCastlingFilter),  /* enforces precondition for goal castling */ \
    ENUMERATOR(STAttackHashed),    /* hash table support for attack */  \
    ENUMERATOR(STHelpHashed),      /* help play with hash table */      \
    ENUMERATOR(STIntelligentFilter), /* initialises intelligent mode */ \
    ENUMERATOR(STGoalReachableGuardFilter), /* goal still reachable in intelligent mode? */ \
    ENUMERATOR(STIntelligentDuplicateAvoider), /* avoid double solutions in intelligent mode */ \
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
    ENUMERATOR(STUltraschachzwangGoalFilter), /* suspend Ultraschachzwang when testing for mate */ \
    ENUMERATOR(STCirceSteingewinnFilter), /* is 'won' piece reborn? */ \
    ENUMERATOR(STCirceCircuitSpecial), /* has reborn capturee made a circuit? */ \
    ENUMERATOR(STCirceExchangeSpecial), /* has reborn capturee made an exchange? */ \
    ENUMERATOR(STAnticirceTargetSquareFilter), /* target square is not reached by capture */ \
    ENUMERATOR(STAnticirceCircuitSpecial), /* special circuit by rebirth */ \
    ENUMERATOR(STAnticirceExchangeSpecial), /* special exchange by rebirth */ \
    ENUMERATOR(STAnticirceExchangeFilter), /* only apply special test after capture in Anticirce */ \
    ENUMERATOR(STPiecesParalysingMateFilter), /* goal not reached because of special rule? */ \
    ENUMERATOR(STPiecesParalysingStalemateSpecial), /* stalemate by special rule? */ \
    ENUMERATOR(STPiecesKamikazeTargetSquareFilter), /* target square not reached because of capture by Kamikaze piece? */ \
    ENUMERATOR(STImmobilityTester), \
    ENUMERATOR(STImmobilityTesterKing), \
    ENUMERATOR(STImmobilityTesterNonKing), \
    ENUMERATOR(STOhneschachImmobilityTesterNonchecking), \
    ENUMERATOR(STOhneschachImmobilityTesterAny), \
    ENUMERATOR(STOhneschachSuspender), \
    ENUMERATOR(STMaffImmobilityTesterKing), \
    ENUMERATOR(STOWUImmobilityTesterKing), \
    /* output slices */                                                 \
    ENUMERATOR(STOutputModeSelector), /* select an output mode for the subsequent play */ \
    ENUMERATOR(STIllegalSelfcheckWriter), /* indicate illegal self-check in the diagram position */ \
    ENUMERATOR(STEndOfPhaseWriter), /* write the end of a phase */      \
    ENUMERATOR(STEndOfSolutionWriter), /* write the end of a solution */  \
    ENUMERATOR(STMoveWriter), /* writes moves */ \
    ENUMERATOR(STKeyWriter), /* write battle play keys */               \
    ENUMERATOR(STTryWriter), /* write "but" */                          \
    ENUMERATOR(STZugzwangWriter), /* writes zugzwang if appropriate */  \
    ENUMERATOR(STTrivialEndFilter), /* don't write trivial variations */  \
    ENUMERATOR(STRefutingVariationWriter), /* writes refuting variations */ \
    ENUMERATOR(STRefutationWriter), /* writes refutations */  \
    ENUMERATOR(STOutputPlaintextTreeCheckWriter), /* plain text output, tree mode: write checks by the previous move */  \
    ENUMERATOR(STOutputPlaintextTreeDecorationWriter), /* plain text output, tree mode: write checks by the previous move */  \
    ENUMERATOR(STOutputPlaintextLineLineWriter), /* plain text output, line mode: write a line */  \
    ENUMERATOR(STOutputPlaintextTreeGoalWriter), /* plain text output, tree mode: write the reached goal */  \
    ENUMERATOR(STOutputPlaintextMoveInversionCounter), /* plain text output: count move inversions */  \
    ENUMERATOR(STOutputPlaintextLineEndOfIntroSeriesMarker), /* handles the end of the intro series */  \
    ENUMERATOR(nr_slice_types),                                         \
    ASSIGNED_ENUMERATOR(no_slice_type = nr_slice_types)
