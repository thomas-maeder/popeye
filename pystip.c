#include "pystip.h"
#include "pydata.h"
#include "pybrafrk.h"
#include "pyhash.h"
#include "pymovein.h"
#include "pykeepmt.h"
#include "pyselfcg.h"
#include "pypipe.h"
#include "pythreat.h"
#include "stipulation/testing_pipe.h"
#include "stipulation/conditional_pipe.h"
#include "stipulation/branch.h"
#include "stipulation/branch.h"
#include "stipulation/setplay_fork.h"
#include "stipulation/dead_end.h"
#include "stipulation/end_of_branch.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/or.h"
#include "stipulation/boolean/and.h"
#include "stipulation/boolean/or.h"
#include "stipulation/boolean/not.h"
#include "stipulation/boolean/binary.h"
#include "stipulation/boolean/true.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/attack_adapter.h"
#include "stipulation/battle_play/defense_adapter.h"
#include "stipulation/move_played.h"
#include "stipulation/goals/immobile/reached_tester.h"
#include "stipulation/help_play/adapter.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/proxy.h"
#include "solving/fork_on_remaining.h"
#include "solving/find_shortest.h"
#include "solving/move_generator.h"
#include "solving/battle_play/try.h"
#include "solving/battle_play/continuation.h"
#include "solving/battle_play/threat.h"
#include "solving/find_by_increasing_length.h"
#include "options/maxsolutions/initialiser.h"
#include "optimisations/goals/enpassant/filter.h"
#include "optimisations/intelligent/mate/filter.h"
#include "optimisations/intelligent/stalemate/filter.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

#include "stipulation/slice_type.h"
#define ENUMERATION_MAKESTRINGS
#include "pyenum.h"


#define ENUMERATION_TYPENAME branch_level
#define ENUMERATORS \
  ENUMERATOR(toplevel_branch),                  \
    ENUMERATOR(nested_branch)

#define ENUMERATION_MAKESTRINGS

#include "pyenum.h"


Slice slices[max_nr_slices];


/* Keep track of allocated slice indices
 */
static boolean is_slice_index_free[max_nr_slices];


#define ENUMERATION_TYPENAME slice_structural_type
#define ENUMERATORS                             \
  ENUMERATOR(slice_structure_leaf),                              \
    ENUMERATOR(slice_structure_binary),                          \
    ENUMERATOR(slice_structure_pipe),                            \
    ENUMERATOR(slice_structure_branch),                          \
    ENUMERATOR(slice_structure_fork),                            \
    ENUMERATOR(nr_slice_structure_types)

#define ENUMERATION_MAKESTRINGS

#include "pyenum.h"


#define ENUMERATION_TYPENAME slice_functional_type
#define ENUMERATORS                             \
  ENUMERATOR(slice_function_unspecified),                        \
    ENUMERATOR(slice_function_move_generator),                   \
    ENUMERATOR(nr_slice_functional_types)

#define ENUMERATION_MAKESTRINGS

#include "pyenum.h"


static slice_structural_type highest_structural_type[nr_slice_types] =
{
  slice_structure_pipe,         /* STProxy */
  slice_structure_fork,         /* STTemporaryHackFork */
  slice_structure_branch,       /* STAttackAdapter */
  slice_structure_branch,       /* STDefenseAdapter */
  slice_structure_branch,       /* STReadyForAttack */
  slice_structure_branch,       /* STReadyForDefense */
  slice_structure_pipe,         /* STNotEndOfBranchGoal */
  slice_structure_pipe,         /* STNotEndOfBranch */
  slice_structure_branch,       /* STMinLengthOptimiser */
  slice_structure_branch,       /* STHelpAdapter */
  slice_structure_branch,       /* STReadyForHelpMove */
  slice_structure_fork,         /* STSetplayFork */
  slice_structure_fork,         /* STEndOfBranch */
  slice_structure_fork,         /* STEndOfBranchForced */
  slice_structure_binary,       /* STEndOfBranchGoal */
  slice_structure_fork,         /* STEndOfBranchTester */
  slice_structure_binary,       /* STAvoidUnsolvable */
  slice_structure_pipe,         /* STResetUnsolvable */
  slice_structure_pipe,         /* STLearnUnsolvable */
  slice_structure_fork,         /* STConstraintSolver */
  slice_structure_fork,         /* STConstraintTester */
  slice_structure_pipe,         /* STEndOfRoot */
  slice_structure_pipe,         /* STEndOfIntro */
  slice_structure_pipe,         /* STDeadEnd */
  slice_structure_pipe,         /* STMove */
  slice_structure_pipe,         /* STForEachMove */
  slice_structure_pipe,         /* STFindMove */
  slice_structure_pipe,         /* STMovePlayed */
  slice_structure_pipe,         /* STHelpMovePlayed */
  slice_structure_pipe,         /* STDummyMove */
  slice_structure_branch,       /* STReadyForDummyMove */
  slice_structure_pipe,         /* STShortSolutionsStart*/
  slice_structure_binary,       /* STCheckZigzagJump */
  slice_structure_pipe,         /* STCheckZigzagLanding */
  slice_structure_fork,         /* STGoalReachedTester */
  slice_structure_pipe,         /* STGoalMateReachedTester */
  slice_structure_pipe,         /* STGoalStalemateReachedTester */
  slice_structure_pipe,         /* STGoalDoubleStalemateReachedTester */
  slice_structure_pipe,         /* STGoalTargetReachedTester */
  slice_structure_pipe,         /* STGoalCheckReachedTester */
  slice_structure_pipe,         /* STGoalCaptureReachedTester */
  slice_structure_pipe,         /* STGoalSteingewinnReachedTester */
  slice_structure_pipe,         /* STGoalEnpassantReachedTester */
  slice_structure_pipe,         /* STGoalDoubleMateReachedTester */
  slice_structure_pipe,         /* STGoalCounterMateReachedTester */
  slice_structure_pipe,         /* STGoalCastlingReachedTester */
  slice_structure_pipe,         /* STGoalAutoStalemateReachedTester */
  slice_structure_pipe,         /* STGoalCircuitReachedTester */
  slice_structure_pipe,         /* STGoalExchangeReachedTester */
  slice_structure_pipe,         /* STGoalCircuitByRebirthReachedTester */
  slice_structure_pipe,         /* STGoalExchangeByRebirthReachedTester */
  slice_structure_pipe,         /* STGoalAnyReachedTester */
  slice_structure_pipe,         /* STGoalProofgameReachedTester */
  slice_structure_pipe,         /* STGoalAToBReachedTester */
  slice_structure_pipe,         /* STGoalMateOrStalemateReachedTester */
  slice_structure_pipe,         /* STGoalChess81ReachedTester */
  slice_structure_fork,         /* STGoalImmobileReachedTester */
  slice_structure_pipe,         /* STGoalNotCheckReachedTester */
  slice_structure_leaf,         /* STTrue */
  slice_structure_leaf,         /* STFalse */
  slice_structure_binary,       /* STAnd */
  slice_structure_binary,       /* STOr */
  slice_structure_pipe,         /* STCheckDetector */
  slice_structure_pipe,         /* STNot */
  slice_structure_pipe,         /* STSelfCheckGuard */
  slice_structure_pipe,         /* STOhneschachCheckGuard */
  slice_structure_pipe,         /* STMoveInverter */
  slice_structure_branch,       /* STMinLengthGuard */
  slice_structure_binary,       /* STForkOnRemaining */
  slice_structure_branch,       /* STFindShortest */
  slice_structure_branch,       /* STFindByIncreasingLength */
  slice_structure_pipe,         /* STGeneratingMoves */
  slice_structure_pipe,         /* STMoveGenerator */
  slice_structure_pipe,         /* STKingMoveGenerator */
  slice_structure_pipe,         /* STNonKingMoveGenerator */
  slice_structure_pipe,         /* STCastlingIntermediateMoveGenerator */
  slice_structure_fork,         /* STCastlingIntermediateMoveLegalityTester */
  slice_structure_pipe,         /* STRefutationsAllocator */
  slice_structure_binary,       /* STRefutationsSolver */
  slice_structure_pipe,         /* STRefutationsFilter */
  slice_structure_pipe,         /* STEndOfRefutationSolvingBranch */
  slice_structure_pipe,         /* STPlaySuppressor */
  slice_structure_fork,         /* STContinuationSolver */
  slice_structure_pipe,         /* STSolvingContinuation */
  slice_structure_fork,         /* STThreatSolver */
  slice_structure_fork,         /* STThreatEnforcer */
  slice_structure_pipe,         /* STThreatStart */
  slice_structure_pipe,         /* STThreatEnd */
  slice_structure_pipe,         /* STThreatCollector */
  slice_structure_pipe,         /* STThreatDefeatedTester */
  slice_structure_pipe,         /* STRefutationsCollector */
  slice_structure_pipe,         /* STRefutationsAvoider */
  slice_structure_pipe,         /* STLegalMoveCounter */
  slice_structure_pipe,         /* STAnyMoveCounter */
  slice_structure_pipe,         /* STCaptureCounter */
  slice_structure_pipe,         /* STTestingPrerequisites */
  slice_structure_fork,         /* STDoubleMateFilter */
  slice_structure_fork,         /* STCounterMateFilter */
  slice_structure_pipe,         /* STPrerequisiteOptimiser */
  slice_structure_fork,         /* STNoShortVariations */
  slice_structure_pipe,         /* STRestartGuard */
  slice_structure_pipe,         /* STRestartGuardIntelligent */
  slice_structure_pipe,         /* STIntelligentTargetCounter */
  slice_structure_pipe,         /* STMaxTimeGuard */
  slice_structure_pipe,         /* STMaxSolutionsInitialiser */
  slice_structure_pipe,         /* STMaxSolutionsGuard */
  slice_structure_pipe,         /* STMaxSolutionsCounter */
  slice_structure_fork,         /* STEndOfBranchGoalImmobile */
  slice_structure_pipe,         /* STDeadEndGoal */
  slice_structure_pipe,         /* STOrthodoxMatingMoveGenerator */
  slice_structure_pipe,         /* STKillerMoveCollector */
  slice_structure_pipe,         /* STKillerMoveFinalDefenseMove */
  slice_structure_pipe,         /* STEnPassantFilter */
  slice_structure_pipe,         /* STCastlingFilter */
  slice_structure_branch,       /* STAttackHashed */
  slice_structure_pipe,         /* STAttackHashedTester */
  slice_structure_branch,       /* STHelpHashed */
  slice_structure_pipe,         /* STHelpHashedTester */
  slice_structure_pipe,         /* STIntelligentMovesLeftInitialiser */
  slice_structure_fork,         /* STIntelligentMateFilter */
  slice_structure_fork,         /* STIntelligentStalemateFilter */
  slice_structure_pipe,         /* STIntelligentProof */
  slice_structure_pipe,         /* STGoalReachableGuardFilterMate */
  slice_structure_pipe,         /* STGoalReachableGuardFilterStalemate */
  slice_structure_pipe,         /* STGoalReachableGuardFilterProof */
  slice_structure_pipe,         /* STGoalReachableGuardFilterProofFairy */
  slice_structure_pipe,         /* STIntelligentSolutionsPerTargetPosCounter */
  slice_structure_pipe,         /* STIntelligentLimitNrSolutionsPerTargetPos */
  slice_structure_pipe,         /* STIntelligentDuplicateAvoider */
  slice_structure_pipe,         /* STIntelligentImmobilisationCounter */
  slice_structure_pipe,         /* STKeepMatingFilter */
  slice_structure_fork,         /* STMaxFlightsquares */
  slice_structure_pipe,         /* STFlightsquaresCounter */
  slice_structure_branch,       /* STDegenerateTree */
  slice_structure_fork,         /* STMaxNrNonTrivial */
  slice_structure_pipe,         /* STMaxNrNonTrivialCounter */
  slice_structure_fork,         /* STMaxThreatLength */
  slice_structure_pipe,         /* STMaxThreatLengthStart */
  slice_structure_pipe,         /* STStopOnShortSolutionsInitialiser */
  slice_structure_branch,       /* STStopOnShortSolutionsFilter */
  slice_structure_pipe,         /* STAmuMateFilter */
  slice_structure_pipe,         /* STUltraschachzwangGoalFilter */
  slice_structure_pipe,         /* STCirceSteingewinnFilter */
  slice_structure_pipe,         /* STCirceCircuitSpecial */
  slice_structure_pipe,         /* STCirceExchangeSpecial */
  slice_structure_pipe,         /* STAnticirceTargetSquareFilter */
  slice_structure_pipe,         /* STAnticirceCircuitSpecial */
  slice_structure_pipe,         /* STAnticirceExchangeSpecial */
  slice_structure_pipe,         /* STAnticirceExchangeFilter */
  slice_structure_pipe,         /* STPiecesParalysingMateFilter */
  slice_structure_pipe,         /* STPiecesParalysingMateFilterTester */
  slice_structure_pipe,         /* STPiecesParalysingStalemateSpecial */
  slice_structure_pipe,         /* STPiecesKamikazeTargetSquareFilter */
  slice_structure_pipe,         /* STImmobilityTester */
  slice_structure_fork,         /* STOpponentMovesCounterFork */
  slice_structure_pipe,         /* STOpponentMovesCounter */
  slice_structure_pipe,         /* STOhneschachSuspender */
  slice_structure_fork,         /* STExclusiveChessMatingMoveCounter */
  slice_structure_pipe,         /* STExclusiveChessUnsuspender */
  slice_structure_pipe,         /* STMaffImmobilityTesterKing */
  slice_structure_pipe,         /* STOWUImmobilityTesterKing) */
  slice_structure_pipe,         /* STSingleMoveGeneratorWithKingCapture */
  slice_structure_fork,         /* STBrunnerDefenderFinder */
  slice_structure_fork,         /* STIsardamDefenderFinder */
  slice_structure_fork,         /* STCageCirceNonCapturingMoveFinder */
  slice_structure_pipe,         /* STSinglePieceMoveGenerator */
  slice_structure_pipe,         /* STSingleMoveGenerator */
  slice_structure_fork,         /* STMaximummerCandidateMoveTester */
  slice_structure_pipe,         /* STBGLFilter */
  slice_structure_pipe,         /* STOutputModeSelector */
  slice_structure_pipe,         /* STIllegalSelfcheckWriter */
  slice_structure_pipe,         /* STEndOfPhaseWriter */
  slice_structure_pipe,         /* STEndOfSolutionWriter */
  slice_structure_pipe,         /* STThreatWriter */
  slice_structure_pipe,         /* STMoveWriter */
  slice_structure_pipe,         /* STKeyWriter */
  slice_structure_pipe,         /* STTryWriter */
  slice_structure_pipe,         /* STZugzwangWriter */
  slice_structure_fork,         /* STTrivialEndFilter */
  slice_structure_pipe,         /* STRefutingVariationWriter */
  slice_structure_pipe,         /* STRefutationsIntroWriter */
  slice_structure_pipe,         /* STRefutationWriter */
  slice_structure_pipe,         /* STOutputPlaintextTreeCheckWriter */
  slice_structure_pipe,         /* STOutputPlaintextLineLineWriter */
  slice_structure_pipe,         /* STOutputPlaintextTreeGoalWriter */
  slice_structure_pipe,         /* STOutputPlaintextMoveInversionCounter */
  slice_structure_pipe,         /* STOutputPlaintextLineEndOfIntroSeriesMarker */
  slice_structure_pipe          /* STMoveTracer */
};

slice_functional_type functional_type[nr_slice_types] =
{
  slice_function_proxy,            /* STProxy */
  slice_function_conditional_pipe, /* STTemporaryHackFork */
  slice_function_unspecified,      /* STAttackAdapter */
  slice_function_unspecified,      /* STDefenseAdapter */
  slice_function_proxy,            /* STReadyForAttack */
  slice_function_proxy,            /* STReadyForDefense */
  slice_function_proxy,            /* STNotEndOfBranchGoal */
  slice_function_proxy,            /* STNotEndOfBranch */
  slice_function_unspecified,      /* STMinLengthOptimiser */
  slice_function_unspecified,      /* STHelpAdapter */
  slice_function_proxy,            /* STReadyForHelpMove */
  slice_function_unspecified,      /* STSetplayFork */
  slice_function_unspecified,      /* STEndOfBranch */
  slice_function_unspecified,      /* STEndOfBranchForced */
  slice_function_unspecified,      /* STEndOfBranchGoal */
  slice_function_conditional_pipe, /* STEndOfBranchTester */
  slice_function_unspecified,      /* STAvoidUnsolvable */
  slice_function_unspecified,      /* STResetUnsolvable */
  slice_function_unspecified,      /* STLearnUnsolvable */
  slice_function_unspecified,      /* STConstraintSolver */
  slice_function_conditional_pipe, /* STConstraintTester */
  slice_function_proxy,            /* STEndOfRoot */
  slice_function_proxy,            /* STEndOfIntro */
  slice_function_unspecified,      /* STDeadEnd */
  slice_function_proxy,            /* STMove */
  slice_function_unspecified,      /* STForEachMove */
  slice_function_unspecified,      /* STFindMove */
  slice_function_unspecified,      /* STMovePlayed */
  slice_function_unspecified,      /* STHelpMovePlayed */
  slice_function_unspecified,      /* STDummyMove */
  slice_function_proxy,            /* STReadyForDummyMove */
  slice_function_proxy,            /* STShortSolutionsStart*/
  slice_function_unspecified,      /* STCheckZigzagJump */
  slice_function_proxy,            /* STCheckZigzagLanding */
  slice_function_conditional_pipe, /* STGoalReachedTester */
  slice_function_proxy,            /* STGoalMateReachedTester */
  slice_function_proxy,            /* STGoalStalemateReachedTester */
  slice_function_proxy,            /* STGoalDoubleStalemateReachedTester */
  slice_function_unspecified,      /* STGoalTargetReachedTester */
  slice_function_unspecified,      /* STGoalCheckReachedTester */
  slice_function_unspecified,      /* STGoalCaptureReachedTester */
  slice_function_unspecified,      /* STGoalSteingewinnReachedTester */
  slice_function_unspecified,      /* STGoalEnpassantReachedTester */
  slice_function_unspecified,      /* STGoalDoubleMateReachedTester */
  slice_function_unspecified,      /* STGoalCounterMateReachedTester */
  slice_function_unspecified,      /* STGoalCastlingReachedTester */
  slice_function_proxy,            /* STGoalAutoStalemateReachedTester */
  slice_function_unspecified,      /* STGoalCircuitReachedTester */
  slice_function_unspecified,      /* STGoalExchangeReachedTester */
  slice_function_unspecified,      /* STGoalCircuitByRebirthReachedTester */
  slice_function_unspecified,      /* STGoalExchangeByRebirthReachedTester */
  slice_function_unspecified,      /* STGoalAnyReachedTester */
  slice_function_unspecified,      /* STGoalProofgameReachedTester */
  slice_function_unspecified,      /* STGoalAToBReachedTester */
  slice_function_unspecified,      /* STGoalMateOrStalemateReachedTester */
  slice_function_unspecified,      /* STGoalChess81ReachedTester */
  slice_function_conditional_pipe, /* STGoalImmobileReachedTester */
  slice_function_unspecified,      /* STGoalNotCheckReachedTester */
  slice_function_unspecified,      /* STTrue */
  slice_function_unspecified,      /* STFalse */
  slice_function_unspecified,      /* STAnd */
  slice_function_unspecified,      /* STOr */
  slice_function_unspecified,      /* STCheckDetector */
  slice_function_unspecified,      /* STNot */
  slice_function_unspecified,      /* STSelfCheckGuard */
  slice_function_unspecified,      /* STOhneschachCheckGuard */
  slice_function_unspecified,      /* STMoveInverter */
  slice_function_unspecified,      /* STMinLengthGuard */
  slice_function_unspecified,      /* STForkOnRemaining */
  slice_function_unspecified,      /* STFindShortest */
  slice_function_unspecified,      /* STFindByIncreasingLength */
  slice_function_proxy,            /* STGeneratingMoves */
  slice_function_move_generator,   /* STMoveGenerator */
  slice_function_move_generator,   /* STKingMoveGenerator */
  slice_function_move_generator,   /* STNonKingMoveGenerator */
  slice_function_move_generator,   /* STCastlingIntermediateMoveGenerator */
  slice_function_conditional_pipe, /* STCastlingIntermediateMoveLegalityTester */
  slice_function_unspecified,      /* STRefutationsAllocator */
  slice_function_unspecified,      /* STRefutationsSolver */
  slice_function_unspecified,      /* STRefutationsFilter */
  slice_function_proxy,            /* STEndOfRefutationSolvingBranch */
  slice_function_unspecified,      /* STPlaySuppressor */
  slice_function_testing_pipe,     /* STContinuationSolver */
  slice_function_proxy,            /* STSolvingContinuation */
  slice_function_unspecified,      /* STThreatSolver */
  slice_function_testing_pipe,     /* STThreatEnforcer */
  slice_function_proxy,            /* STThreatStart */
  slice_function_proxy,            /* STThreatEnd */
  slice_function_unspecified,      /* STThreatCollector */
  slice_function_unspecified,      /* STThreatDefeatedTester */
  slice_function_unspecified,      /* STRefutationsCollector */
  slice_function_unspecified,      /* STRefutationsAvoider */
  slice_function_unspecified,      /* STLegalMoveCounter */
  slice_function_unspecified,      /* STAnyMoveCounter */
  slice_function_unspecified,      /* STCaptureCounter */
  slice_function_proxy,            /* STTestingPrerequisites */
  slice_function_conditional_pipe, /* STDoubleMateFilter */
  slice_function_conditional_pipe, /* STCounterMateFilter */
  slice_function_unspecified,      /* STPrerequisiteOptimiser */
  slice_function_testing_pipe,     /* STNoShortVariations */
  slice_function_unspecified,      /* STRestartGuard */
  slice_function_unspecified,      /* STRestartGuardIntelligent */
  slice_function_unspecified,      /* STIntelligentTargetCounter */
  slice_function_unspecified,      /* STMaxTimeGuard */
  slice_function_unspecified,      /* STMaxSolutionsInitialiser */
  slice_function_unspecified,      /* STMaxSolutionsGuard */
  slice_function_unspecified,      /* STMaxSolutionsCounter */
  slice_function_unspecified,      /* STEndOfBranchGoalImmobile */
  slice_function_unspecified,      /* STDeadEndGoal */
  slice_function_move_generator,   /* STOrthodoxMatingMoveGenerator */
  slice_function_unspecified,      /* STKillerMoveCollector */
  slice_function_move_generator,   /* STKillerMoveFinalDefenseMove */
  slice_function_unspecified,      /* STEnPassantFilter */
  slice_function_unspecified,      /* STCastlingFilter */
  slice_function_unspecified,      /* STAttackHashed */
  slice_function_unspecified,      /* STAttackHashedTester */
  slice_function_unspecified,      /* STHelpHashed */
  slice_function_unspecified,      /* STHelpHashedTester */
  slice_function_unspecified,      /* STIntelligentMovesLeftInitialiser */
  slice_function_conditional_pipe, /* STIntelligentMateFilter */
  slice_function_conditional_pipe, /* STIntelligentStalemateFilter */
  slice_function_unspecified,      /* STIntelligentProof */
  slice_function_unspecified,      /* STGoalReachableGuardFilterMate */
  slice_function_unspecified,      /* STGoalReachableGuardFilterStalemate */
  slice_function_unspecified,      /* STGoalReachableGuardFilterProof */
  slice_function_unspecified,      /* STGoalReachableGuardFilterProofFairy */
  slice_function_unspecified,      /* STIntelligentSolutionsPerTargetPosCounter */
  slice_function_unspecified,      /* STIntelligentLimitNrSolutionsPerTargetPos */
  slice_function_unspecified,      /* STIntelligentDuplicateAvoider */
  slice_function_unspecified,      /* STIntelligentImmobilisationCounter */
  slice_function_unspecified,      /* STKeepMatingFilter */
  slice_function_conditional_pipe, /* STMaxFlightsquares */
  slice_function_unspecified,      /* STFlightsquaresCounter */
  slice_function_unspecified,      /* STDegenerateTree */
  slice_function_testing_pipe,     /* STMaxNrNonTrivial */
  slice_function_unspecified,      /* STMaxNrNonTrivialCounter */
  slice_function_testing_pipe,     /* STMaxThreatLength */
  slice_function_proxy,            /* STMaxThreatLengthStart */
  slice_function_unspecified,      /* STStopOnShortSolutionsInitialiser */
  slice_function_unspecified,      /* STStopOnShortSolutionsFilter */
  slice_function_unspecified,      /* STAmuMateFilter */
  slice_function_unspecified,      /* STUltraschachzwangGoalFilter */
  slice_function_unspecified,      /* STCirceSteingewinnFilter */
  slice_function_unspecified,      /* STCirceCircuitSpecial */
  slice_function_unspecified,      /* STCirceExchangeSpecial */
  slice_function_unspecified,      /* STAnticirceTargetSquareFilter */
  slice_function_unspecified,      /* STAnticirceCircuitSpecial */
  slice_function_unspecified,      /* STAnticirceExchangeSpecial */
  slice_function_unspecified,      /* STAnticirceExchangeFilter */
  slice_function_unspecified,      /* STPiecesParalysingMateFilter */
  slice_function_unspecified,      /* STPiecesParalysingMateFilterTester */
  slice_function_unspecified,      /* STPiecesParalysingStalemateSpecial */
  slice_function_unspecified,      /* STPiecesKamikazeTargetSquareFilter */
  slice_function_unspecified,      /* STImmobilityTester */
  slice_function_conditional_pipe, /* STOpponentMovesCounterFork */
  slice_function_unspecified,      /* STOpponentMovesCounter */
  slice_function_unspecified,      /* STOhneschachSuspender */
  slice_function_conditional_pipe, /* STExclusiveChessMatingMoveCounter */
  slice_function_unspecified,      /* STExclusiveChessUnsuspender */
  slice_function_unspecified,      /* STMaffImmobilityTesterKing */
  slice_function_unspecified,      /* STOWUImmobilityTesterKing */
  slice_function_move_generator,   /* STSingleMoveGeneratorWithKingCapture */
  slice_function_conditional_pipe, /* STBrunnerDefenderFinder */
  slice_function_conditional_pipe, /* STIsardamDefenderFinder */
  slice_function_conditional_pipe, /* STCageCirceNonCapturingMoveFinder */
  slice_function_unspecified,      /* STSinglePieceMoveGenerator */
  slice_function_move_generator,   /* STSingleMoveGenerator */
  slice_function_conditional_pipe, /* STMaximummerCandidateMoveTester */
  slice_function_unspecified,      /* STBGLFilter */
  slice_function_proxy,            /* STOutputModeSelector */
  slice_function_writer,           /* STIllegalSelfcheckWriter */
  slice_function_writer,           /* STEndOfPhaseWriter */
  slice_function_writer,           /* STEndOfSolutionWriter */
  slice_function_writer,           /* STThreatWriter */
  slice_function_writer,           /* STMoveWriter */
  slice_function_writer,           /* STKeyWriter */
  slice_function_writer,           /* STTryWriter */
  slice_function_writer,           /* STZugzwangWriter */
  slice_function_testing_pipe,     /* STTrivialEndFilter */
  slice_function_writer,           /* STRefutingVariationWriter */
  slice_function_writer,           /* STRefutationsIntroWriter */
  slice_function_writer,           /* STRefutationWriter */
  slice_function_writer,           /* STOutputPlaintextTreeCheckWriter */
  slice_function_writer,           /* STOutputPlaintextLineLineWriter */
  slice_function_writer,           /* STOutputPlaintextTreeGoalWriter */
  slice_function_unspecified,      /* STOutputPlaintextMoveInversionCounter */
  slice_function_unspecified,      /* STOutputPlaintextLineEndOfIntroSeriesMarker */
  slice_function_unspecified       /* STMoveTracer */
};

/* Retrieve the structural type of a slice
 * @param si identifies slice of which to retrieve structural type
 * @return structural type of slice si
 */
slice_structural_type slice_get_structural_type(slice_index si)
{
  /* no Trace instrumentation here - this is used by the Trace machinery! */
  assert(slices[si].type<=nr_slice_types);
  return highest_structural_type[slices[si].type];
}

/* Retrieve the functional type of a slice
 * @param si identifies slice of which to retrieve structural type
 * @return structural type of slice si
 */
slice_functional_type slice_get_functional_type(slice_index si)
{
  assert(slices[si].type<=nr_slice_types);
  return functional_type[slices[si].type];
}

/* Make sure that there are now allocated slices that are not
 * reachable
 */
void assert_no_leaked_slices(void)
{
  slice_index i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 0; i!=max_nr_slices; ++i)
  {
    if (!is_slice_index_free[i])
    {
      TraceValue("leaked:%u",i);
      TraceEnumerator(slice_type,slices[i].type,"\n");
    }
    assert(is_slice_index_free[i]);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialize the slice allocation machinery. To be called once at
 * program start
 */
void init_slice_allocator(void)
{
  slice_index si;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (si = 0; si!=max_nr_slices; ++si)
    is_slice_index_free[si] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a slice index
 * @param type which type
 * @return a so far unused slice index
 */
slice_index alloc_slice(slice_type type)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParamListEnd();

  for (result = 0; result!=max_nr_slices; ++result)
    if (is_slice_index_free[result])
      break;

  assert(result<max_nr_slices);

  is_slice_index_free[result] = false;

  slices[result].type = type;
  slices[result].starter = no_side;
  slices[result].prev = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Dellocate a slice index
 * @param si slice index deallocated
 */
void dealloc_slice(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(slice_type,slices[si].type,"\n");
  assert(!is_slice_index_free[si]);
  is_slice_index_free[si] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a slice as copy of an existing slice
 * @param index of original slice
 * @return index of allocated slice
 */
slice_index copy_slice(slice_index original)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",original);
  TraceFunctionParamListEnd();

  result = alloc_slice(slices[original].type);

  slices[result] = slices[original];
  slice_set_predecessor(result,no_slice);

  TraceEnumerator(Side,slices[original].starter,"");
  TraceEnumerator(Side,slices[result].starter,"\n");

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Make a slice the predecessor of a slice
 * @param slice identifies the slice
 * @param pred identifies the slice to be made the predecessor of slice
 */
void slice_set_predecessor(slice_index slice, slice_index pred)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",slice);
  TraceFunctionParam("%u",pred);
  TraceFunctionParamListEnd();

  slices[slice].prev = pred;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Deallocate slices reachable from a slice
 * @param si slice where to start deallocating
 */
void dealloc_slices(slice_index si)
{
  slice_index i;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_traverse_structure(si,&st);

  for (i = 0; i!=max_nr_slices; ++i)
    if (st.traversed[i]==slice_traversed)
      dealloc_slice(i);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void move_to_root(slice_index si, stip_structure_traversal *st)
{
  spin_off_state_type * const state = st->param;
  slice_index const save_next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  link_to_branch(si,state->spun_off[save_next]);
  state->spun_off[si] = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors root_slice_inserters[] =
{
  { STAttackAdapter,  &attack_adapter_make_root  },
  { STDefenseAdapter, &defense_adapter_make_root },
  { STHelpAdapter,    &help_adapter_make_root    },
  { STAnd,            &binary_make_root          },
  { STOr,             &binary_make_root          }
};

enum
{
  nr_root_slice_inserters = (sizeof root_slice_inserters
                             / sizeof root_slice_inserters[0])
};

/* Initialise a spin_off_state_type object
 */
static void spin_off_state_init(spin_off_state_type *state)
{
  slice_index i;
  for (i = 0; i!=max_nr_slices; ++i)
    state->spun_off[i] = no_slice;
}

/* Wrap the slices representing the initial moves of the solution with
 * slices of appropriately equipped slice types
 * @param si identifies slice where to start
 */
void stip_insert_root_slices(slice_index si)
{
  stip_structure_traversal st;
  spin_off_state_type state;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  assert(slices[si].type==STProxy);

  spin_off_state_init(&state);
  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_pipe,
                                                 &move_to_root);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_branch,
                                                 &move_to_root);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_fork,
                                                 &move_to_root);
  stip_structure_traversal_override(&st,
                                    root_slice_inserters,
                                    nr_root_slice_inserters);
  stip_traverse_structure(next,&st);

  pipe_link(si,state.spun_off[next]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void link_to_intro(slice_index si, stip_structure_traversal *st)
{
  stip_traverse_structure_children(si,st);

  /* make sure that the entry slices into the intro have a correct .prev value */
  link_to_branch(si,slices[si].u.pipe.next);
}

void hack_fork_make_intro(slice_index fork, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",fork);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(fork,st);

  st->level = structure_traversal_level_nested;
  stip_traverse_structure(slices[fork].u.fork.fork,st);
  st->level = structure_traversal_level_root;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors intro_slice_inserters[] =
{
  { STAttackAdapter,     &attack_adapter_make_intro   },
  { STDefenseAdapter,    &defense_adapter_make_intro  },
  { STHelpAdapter,       &help_adapter_make_intro     },
  { STTemporaryHackFork, &hack_fork_make_intro        }
};

enum
{
  nr_intro_slice_inserters = (sizeof intro_slice_inserters
                              / sizeof intro_slice_inserters[0])
};

/* Wrap the slices representing the initial moves of nested slices
 * @param si identifies slice where to start
 */
void stip_insert_intro_slices(slice_index si)
{
  spin_off_state_type state;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  assert(slices[si].type==STProxy);

  spin_off_state_init(&state);
  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_pipe,
                                                 &link_to_intro);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_branch,
                                                 &link_to_intro);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_fork,
                                                 &link_to_intro);
  stip_structure_traversal_override(&st,
                                    intro_slice_inserters,
                                    nr_intro_slice_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine contribution of slice subtree to maximum number of moves
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void get_max_nr_moves_move(slice_index si, stip_moves_traversal *st)
{
  stip_length_type * const result = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  ++*result;
  TraceValue("%u\n",*result);

  stip_traverse_moves_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine the maximally possible number of half-moves until the
 * goal has to be reached.
 * @param si root of subtree
 * @param maximally possible number of half-moves
 */
stip_length_type get_max_nr_moves(slice_index si)
{
  stip_moves_traversal st;
  stip_length_type result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);
  stip_moves_traversal_init(&st,&result);
  stip_moves_traversal_override_by_structure(&st,
                                             slice_structure_binary,
                                             &get_max_nr_moves_binary);
  stip_moves_traversal_override_single(&st,STMove,&get_max_nr_moves_move);
  stip_traverse_moves(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

typedef struct
{
  Goal unique_goal;
  boolean is_unique;
} find_unique_goal_state;

static void find_unique_goal_goal(slice_index si,
                                  stip_structure_traversal *st)
{
  find_unique_goal_state * const state = st->param;
  goal_type const goal = slices[si].u.goal_handler.goal.type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->unique_goal.type==no_goal)
    state->unique_goal.type = goal;
  else if (state->is_unique && state->unique_goal.type!=goal)
    state->is_unique = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether the current stipulation has a unique goal, and
 * return it.
 * @param si root of subtree where to look for unique goal
 * @return no_slice if goal is not unique; index of a slice with the
 * unique goal otherwise
 */
Goal find_unique_goal(slice_index si)
{
  stip_structure_traversal st;
  find_unique_goal_state result = { { no_goal, initsquare }, true };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&result);
  stip_structure_traversal_override_single(&st,
                                           STGoalReachedTester,
                                           &find_unique_goal_goal);
  stip_traverse_structure(si,&st);

  if (!result.is_unique)
    result.unique_goal.type = no_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result.unique_goal.type);
  TraceFunctionResultEnd();
  return result.unique_goal;
}

static void copy_and_remember(slice_index si, stip_deep_copies_type *copies)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert((*copies)[si]==no_slice);
  (*copies)[si] = copy_slice(si);
  slices[(*copies)[si]].starter = no_side;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void deep_copy_leaf(slice_index si, stip_structure_traversal *st)
{
  stip_deep_copies_type * const copies = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  copy_and_remember(si,copies);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void deep_copy_pipe(slice_index si, stip_structure_traversal *st)
{
  stip_deep_copies_type * const copies = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  copy_and_remember(si,copies);

  stip_traverse_structure_children(si,st);

  if (slices[si].u.pipe.next!=no_slice)
    link_to_branch((*copies)[si],(*copies)[slices[si].u.pipe.next]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void deep_copy_fork(slice_index si, stip_structure_traversal *st)
{
  stip_deep_copies_type * const copies = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  copy_and_remember(si,copies);

  stip_traverse_structure_children(si,st);

  if (slices[si].u.fork.fork!=no_slice)
    slices[(*copies)[si]].u.fork.fork = (*copies)[slices[si].u.fork.fork];

  if (slices[si].u.fork.next!=no_slice)
    link_to_branch((*copies)[si],(*copies)[slices[si].u.fork.next]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void deep_copy_binary(slice_index si, stip_structure_traversal *st)
{
  stip_deep_copies_type * const copies = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  copy_and_remember(si,copies);

  stip_traverse_structure_children(si,st);

  if (slices[si].u.binary.op1!=no_slice)
    slices[(*copies)[si]].u.binary.op1 = (*copies)[slices[si].u.binary.op1];

  if (slices[si].u.binary.op2!=no_slice)
    slices[(*copies)[si]].u.binary.op2 = (*copies)[slices[si].u.binary.op2];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise a structure traversal for a deep copy operation
 * @param st address of the structure to be initialised
 * @param copies address of an array mapping indices of originals
 *        to indices of copies
 * @note initialises all elements of *copies to no_slice
 * @note after this initialisation, *st can be used for a deep copy operation;
 *       or st can be further modified for some special copy operation
 */
void init_deep_copy(stip_structure_traversal *st, stip_deep_copies_type *copies)
{
  slice_index i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 0; i!=max_nr_slices; ++i)
    (*copies)[i] = no_slice;

  stip_structure_traversal_init(st,copies);
  stip_structure_traversal_override_by_structure(st,
                                                 slice_structure_leaf,
                                                 &deep_copy_leaf);
  stip_structure_traversal_override_by_structure(st,
                                                 slice_structure_pipe,
                                                 &deep_copy_pipe);
  stip_structure_traversal_override_by_structure(st,
                                                 slice_structure_branch,
                                                 &deep_copy_pipe);
  stip_structure_traversal_override_by_structure(st,
                                                 slice_structure_fork,
                                                 &deep_copy_fork);
  stip_structure_traversal_override_by_structure(st,
                                                 slice_structure_binary,
                                                 &deep_copy_binary);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* in-place deep copying a stipulation sub-tree
 * @param si root of sub-tree
 * @return index of root of copy
 */
slice_index stip_deep_copy(slice_index si)
{
  stip_deep_copies_type copies;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  init_deep_copy(&st,&copies);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",copies[si]);
  TraceFunctionResultEnd();
  return copies[si];
}

typedef struct
{
  slice_index to_goal;
  boolean has_attack_ended;
} quodlibet_transformation_state;

static void remember_end_of_attack(slice_index si, stip_structure_traversal *st)
{
  quodlibet_transformation_state * const state = st->param;
  boolean const save_has_attack_ended = state->has_attack_ended;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->has_attack_ended = true;
  stip_traverse_structure_children(si,st);
  state->has_attack_ended = save_has_attack_ended;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void transform_to_quodlibet_end_of_branch(slice_index si,
                                                stip_structure_traversal *st)
{
  quodlibet_transformation_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->has_attack_ended)
    state->to_goal = stip_deep_copy(slices[si].u.fork.fork);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_direct_guards(slice_index si,
                                 stip_structure_traversal *st)
{
  quodlibet_transformation_state const * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (st->level==structure_traversal_level_root
      && slices[si].u.branch.length>slack_length
      && state->to_goal!=no_slice)
    battle_branch_insert_direct_end_of_branch_goal(si,state->to_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors to_quodlibet_transformers[] =
{
  { STAttackAdapter,   &insert_direct_guards                 },
  { STReadyForDefense, &remember_end_of_attack               },
  { STEndOfBranchGoal, &transform_to_quodlibet_end_of_branch }
};

enum
{
  nr_to_quodlibet_transformers = (sizeof to_quodlibet_transformers
                                  / sizeof to_quodlibet_transformers[0])
};

/* Transform a stipulation tree to "traditional quodlibet form",
 * i.e. a logical OR of direct and self goal.
 * @param si identifies slice where to start
 * @return true iff quodlibet could be applied
 */
boolean transform_to_quodlibet(slice_index si)
{
  stip_structure_traversal st;
  quodlibet_transformation_state state = { no_slice, false };
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override(&st,
                                    to_quodlibet_transformers,
                                    nr_to_quodlibet_transformers);
  stip_traverse_structure(si,&st);

  result = state.to_goal!=no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}

static void hack_fork_apply_setplay(slice_index si, stip_structure_traversal *st)
{
  spin_off_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);
  TraceValue("%u\n",state->spun_off[slices[si].u.fork.next]);

  state->spun_off[si] = state->spun_off[slices[si].u.fork.next];
  TraceValue("%u\n",state->spun_off[si]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


static structure_traversers_visitors setplay_appliers[] =
{
  { STMoveInverter,      &pipe_spin_off_copy           },
  { STConstraintSolver,  &stip_traverse_structure_pipe },
  { STAttackAdapter,     &attack_adapter_apply_setplay },
  { STDefenseAdapter,    &stip_structure_visitor_noop  },
  { STHelpAdapter,       &help_adapter_apply_setplay   },
  { STTemporaryHackFork, &hack_fork_apply_setplay      }
};

enum
{
  nr_setplay_appliers = (sizeof setplay_appliers / sizeof setplay_appliers[0])
};

/* Combine the set play slices into the current stipulation
 * @param setplay slice index of set play
 */
static void insert_set_play(slice_index si, slice_index setplay_slice)
{
  slice_index proxy;
  slice_index regular;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",setplay_slice);
  TraceFunctionParamListEnd();

  proxy = alloc_proxy_slice();
  link_to_branch(proxy,setplay_slice);

  if (slices[next].prev==si)
    regular = next;
  else
  {
    regular = alloc_proxy_slice();
    pipe_set_successor(regular,next);
  }

  pipe_link(si,regular);

  {
    slice_index const set_fork = alloc_setplay_fork_slice(proxy);
    root_branch_insert_slices(si,&set_fork,1);
  }

  pipe_append(proxy,alloc_move_inverter_slice());

  TraceFunctionExit(__func__);
  TraceFunctionParamListEnd();
}

/* Attempt to add set play to the stipulation
 * @param si identifies the root from which to apply set play
 * @return true iff set play could be added
 */
boolean stip_apply_setplay(slice_index si)
{
  boolean result;
  spin_off_state_type state;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  spin_off_state_init(&state);
  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_pipe,
                                                 &pipe_spin_off_skip);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_branch,
                                                 &pipe_spin_off_skip);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_fork,
                                                 &pipe_spin_off_skip);
  stip_structure_traversal_override(&st,setplay_appliers,nr_setplay_appliers);
  stip_traverse_structure(si,&st);

  if (state.spun_off[si]==no_slice)
    result = false;
  else
  {
    insert_set_play(si,state.spun_off[si]);
    result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionParam("%u",result);
  TraceFunctionParamListEnd();
  return result;
}

typedef struct
{
    goal_type const goal_type;
    boolean result;
} goal_search;

static void ends_in_goal(slice_index si, stip_structure_traversal *st)
{
  goal_search * const search = st->param;
  goal_type const goal = slices[si].u.goal_handler.goal.type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  search->result = search->result || search->goal_type==goal;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Does the current stipulation end in a specific goal?
 * @param si identifies slice where to start
 * @param goal identifies the goal
 * @return true iff one of the goals of the stipulation is goal
 */
boolean stip_ends_in(slice_index si, goal_type goal)
{
  goal_search search = { goal, false };
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&search);
  stip_structure_traversal_override_single(&st,
                                           STGoalReachedTester,
                                           &ends_in_goal);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",search.result);
  TraceFunctionResultEnd();
  return search.result;
}

static structure_traversers_visitors starter_detectors[] =
{
  { STMovePlayed,     &move_played_detect_starter   },
  { STHelpMovePlayed, &move_played_detect_starter   },
  { STDummyMove,      &move_played_detect_starter   },
  { STMoveInverter,   &move_inverter_detect_starter }
};

enum
{
  nr_starter_detectors = (sizeof starter_detectors
                          / sizeof starter_detectors[0])
};

/* Detect the starting side from the stipulation
 * @param si identifies slice whose starter to find
 */
void stip_detect_starter(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,NULL);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_binary,
                                                 &binary_detect_starter);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_fork,
                                                 &branch_fork_detect_starter);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_pipe,
                                                 &pipe_detect_starter);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_testing_pipe,
                                                &pipe_detect_starter);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_conditional_pipe,
                                                &pipe_detect_starter);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_branch,
                                                 &pipe_detect_starter);
  stip_structure_traversal_override(&st,
                                    starter_detectors,
                                    nr_starter_detectors);
  stip_traverse_structure(si,&st);

  TraceStipulation(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Impose the starting side on a stipulation
 * @param si identifies slice
 * @param st address of structure that holds the state of the traversal
 */
static void default_impose_starter(slice_index si,
                                   stip_structure_traversal *st)
{
  Side const * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,*starter,"");
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;
  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Impose the starting side on a stipulation.
 * @param si identifies slice
 * @param st address of structure that holds the state of the traversal
 */
static void impose_inverted_starter(slice_index si,
                                    stip_structure_traversal *st)
{
  Side * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",*starter);
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;

  *starter = advers(*starter);
  stip_traverse_structure_children(si,st);
  *starter = slices[si].starter;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Slice types that change the starting side
 */
static slice_type starter_inverters[] =
{
  STMovePlayed,
  STHelpMovePlayed,
  STDummyMove,
  STMoveInverter
};

enum
{
  nr_starter_inverters = (sizeof starter_inverters
                          / sizeof starter_inverters[0])
};

static void stip_impose_starter_impl(slice_index si,
                                     Side starter,
                                     stip_structure_traversal *st)
{
  unsigned int i;
  slice_type type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,starter,"");
  TraceFunctionParamListEnd();

  for (type = 0; type!=nr_slice_types; ++type)
    stip_structure_traversal_override_single(st,
                                             type,
                                             &default_impose_starter);

  for (i = 0; i!=nr_starter_inverters; ++i)
    stip_structure_traversal_override_single(st,
                                             starter_inverters[i],
                                             &impose_inverted_starter);
  stip_structure_traversal_override_single(st,
                                           STIntelligentMateFilter,
                                           &impose_starter_intelligent_mate_filter);
  stip_structure_traversal_override_single(st,
                                           STIntelligentStalemateFilter,
                                           &impose_starter_intelligent_stalemate_filter);
  stip_structure_traversal_override_single(st,
                                           STGoalImmobileReachedTester,
                                           &impose_starter_immobility_tester);

  stip_traverse_structure(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Set the starting side of the stipulation
 * @param si identifies slice where to start
 * @param starter starting side at the root of the stipulation
 */
void stip_impose_starter(slice_index si, Side starter)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,starter,"");
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&starter);
  stip_impose_starter_impl(si,starter,&st);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Set the starting side of the stipulation from within an ongoing iteration
 * @param si identifies slice where to start
 * @param starter starting side at the root of the stipulation
 * @param st address of structure representing ongoing iteration
 */
void stip_impose_starter_nested(slice_index si,
                                Side starter,
                                stip_structure_traversal *st)
{
  stip_structure_traversal st_nested;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,starter,"");
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st_nested,&starter);
  st_nested.context = st->context;
  stip_impose_starter_impl(si,starter,&st_nested);
  stip_traverse_structure(si,&st_nested);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Slice operation doing nothing. Makes it easier to intialise
 * operations table
 * @param si identifies slice on which to invoke noop
 * @param st address of structure defining traversal
 */
void stip_structure_visitor_noop(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Dispatch a slice structure operation to a slice based on its type
 * @param si identifies slice
 * @param ops contains addresses of visitors per slice type
 * @param st address of structure defining traversal
 */
static
void stip_structure_visit_slice(slice_index si,
                                stip_structure_visitor (*ops)[nr_slice_types],
                                stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  TraceEnumerator(slice_type,slices[si].type,"\n");
  assert(slices[si].type<=nr_slice_types);

  {
    stip_structure_visitor const operation = (*ops)[slices[si].type];
    assert(operation!=0);
    (*operation)(si,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Query the traversal state of a slice
 * @param si identifies slice for which to query traversal state
 * @param st address of structure defining traversal
 * @return state of si in traversal *st
 */
stip_structure_traversal_state
get_stip_structure_traversal_state(slice_index si,
                                   stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",st->traversed[si]);
  TraceFunctionResultEnd();
  return st->traversed[si];
}

/* (Approximately) depth-first traversal of the stipulation
 * @param root entry slice into stipulation
 * @param st address of data structure holding parameters for the operation
 */
void stip_traverse_structure(slice_index root, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",st->context);

  if (root!=no_slice)
    if (st->traversed[root]==slice_not_traversed)
    {
      /* avoid infinite recursion */
      st->traversed[root] = slice_being_traversed;
      stip_structure_visit_slice(root,&st->map.visitors,st);
      st->traversed[root] = slice_traversed;
    }

  TraceValue("%u\n",st->context);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor structure_children_traversers[] =
{
  &stip_traverse_structure_pipe,              /* STProxy */
  &stip_traverse_structure_fork,              /* STTemporaryHackFork */
  &stip_traverse_structure_attack_adpater,    /* STAttackAdapter */
  &stip_traverse_structure_defense_adapter,   /* STDefenseAdapter */
  &stip_traverse_structure_ready_for_attack,  /* STReadyForAttack */
  &stip_traverse_structure_ready_for_defense, /* STReadyForDefense */
  &stip_traverse_structure_pipe,              /* STNotEndOfBranchGoal */
  &stip_traverse_structure_pipe,              /* STNotEndOfBranch */
  &stip_traverse_structure_pipe,              /* STMinLengthOptimiser */
  &stip_traverse_structure_help_adpater,      /* STHelpAdapter */
  &stip_traverse_structure_pipe,              /* STReadyForHelpMove */
  &stip_traverse_structure_setplay_fork,      /* STSetplayFork */
  &stip_traverse_structure_fork,              /* STEndOfBranch */
  &stip_traverse_structure_fork,              /* STEndOfBranchForced */
  &stip_traverse_structure_binary,            /* STEndOfBranchGoal */
  &stip_traverse_structure_fork,              /* STEndOfBranchTester */
  &stip_traverse_structure_binary,            /* STAvoidUnsolvable */
  &stip_traverse_structure_pipe,              /* STResetUnsolvable */
  &stip_traverse_structure_pipe,              /* STLearnUnsolvable */
  &stip_traverse_structure_fork,              /* STConstraintSolver */
  &stip_traverse_structure_fork,              /* STConstraintTester */
  &stip_traverse_structure_pipe,              /* STEndOfRoot */
  &stip_traverse_structure_pipe,              /* STEndOfIntro */
  &stip_traverse_structure_pipe,              /* STDeadEnd */
  &stip_traverse_structure_pipe,              /* STMove */
  &stip_traverse_structure_pipe,              /* STForEachMove */
  &stip_traverse_structure_pipe,              /* STFindMove */
  &stip_traverse_structure_move_played,       /* STMovePlayed */
  &stip_traverse_structure_move_played,       /* STHelpMovePlayed */
  &stip_traverse_structure_move_played,       /* STDummyMove */
  &stip_traverse_structure_pipe,              /* STReadyForDummyMove */
  &stip_traverse_structure_pipe,              /* STShortSolutionsStart*/
  &stip_traverse_structure_binary,            /* STCheckZigzagJump */
  &stip_traverse_structure_pipe,              /* STCheckZigzagLanding */
  &stip_traverse_structure_fork,              /* STGoalReachedTester */
  &stip_traverse_structure_pipe,              /* STGoalMateReachedTester */
  &stip_traverse_structure_pipe,              /* STGoalStalemateReachedTester */
  &stip_traverse_structure_pipe,              /* STGoalDoubleStalemateReachedTester */
  &stip_traverse_structure_pipe,              /* STGoalTargetReachedTester */
  &stip_traverse_structure_pipe,              /* STGoalCheckReachedTester */
  &stip_traverse_structure_pipe,              /* STGoalCaptureReachedTester */
  &stip_traverse_structure_pipe,              /* STGoalSteingewinnReachedTester */
  &stip_traverse_structure_pipe,              /* STGoalEnpassantReachedTester */
  &stip_traverse_structure_pipe,              /* STGoalDoubleMateReachedTester */
  &stip_traverse_structure_pipe,              /* STGoalCounterMateReachedTester */
  &stip_traverse_structure_pipe,              /* STGoalCastlingReachedTester */
  &stip_traverse_structure_pipe,              /* STGoalAutoStalemateReachedTester */
  &stip_traverse_structure_pipe,              /* STGoalCircuitReachedTester */
  &stip_traverse_structure_pipe,              /* STGoalExchangeReachedTester */
  &stip_traverse_structure_pipe,              /* STGoalCircuitByRebirthReachedTester */
  &stip_traverse_structure_pipe,              /* STGoalExchangeByRebirthReachedTester */
  &stip_traverse_structure_pipe,              /* STGoalAnyReachedTester */
  &stip_traverse_structure_pipe,              /* STGoalProofgameReachedTester */
  &stip_traverse_structure_pipe,              /* STGoalAToBReachedTester */
  &stip_traverse_structure_pipe,              /* STGoalMateOrStalemateReachedTester */
  &stip_traverse_structure_pipe,              /* STGoalChess81ReachedTester */
  &stip_traverse_structure_fork,              /* STGoalImmobileReachedTester */
  &stip_traverse_structure_pipe,              /* STGoalNotCheckReachedTester */
  &stip_structure_visitor_noop,               /* STTrue */
  &stip_structure_visitor_noop,               /* STFalse */
  &stip_traverse_structure_binary,            /* STAnd */
  &stip_traverse_structure_binary,            /* STOr */
  &stip_traverse_structure_pipe,              /* STCheckDetector */
  &stip_traverse_structure_pipe,              /* STNot */
  &stip_traverse_structure_pipe,              /* STSelfCheckGuard */
  &stip_traverse_structure_pipe,              /* STOhneschachCheckGuard */
  &stip_traverse_structure_pipe,              /* STMoveInverter */
  &stip_traverse_structure_pipe,              /* STMinLengthGuard */
  &stip_traverse_structure_fork_on_remaining, /* STForkOnRemaining */
  &stip_traverse_structure_pipe,              /* STFindShortest */
  &stip_traverse_structure_pipe,              /* STFindByIncreasingLength */
  &stip_traverse_structure_pipe,              /* STGeneratingMoves */
  &stip_traverse_structure_pipe,              /* STMoveGenerator */
  &stip_traverse_structure_pipe,              /* STKingMoveGenerator */
  &stip_traverse_structure_pipe,              /* STNonKingMoveGenerator */
  &stip_traverse_structure_pipe,              /* STCastlingIntermediateMoveGenerator */
  &stip_traverse_structure_fork,              /* STCastlingIntermediateMoveLegalityTester */
  &stip_traverse_structure_pipe,              /* STRefutationsAllocator */
  &stip_traverse_structure_binary,            /* STRefutationsSolver */
  &stip_traverse_structure_pipe,              /* STRefutationsFilter */
  &stip_traverse_structure_pipe,              /* STEndOfRefutationSolvingBranch */
  &stip_traverse_structure_pipe,              /* STPlaySuppressor */
  &stip_traverse_structure_testing_pipe,      /* STContinuationSolver */
  &stip_traverse_structure_pipe,              /* STSolvingContinuation */
  &stip_traverse_structure_testing_pipe,      /* STThreatSolver */
  &stip_traverse_structure_testing_pipe,      /* STThreatEnforcer */
  &stip_traverse_structure_pipe,              /* STThreatStart */
  &stip_traverse_structure_pipe,              /* STThreatEnd */
  &stip_traverse_structure_pipe,              /* STThreatCollector */
  &stip_traverse_structure_pipe,              /* STThreatDefeatedTester */
  &stip_traverse_structure_pipe,              /* STRefutationsCollector */
  &stip_traverse_structure_pipe,              /* STRefutationsAvoider */
  &stip_traverse_structure_pipe,              /* STLegalMoveCounter */
  &stip_traverse_structure_pipe,              /* STAnyMoveCounter */
  &stip_traverse_structure_pipe,              /* STCaptureCounter */
  &stip_traverse_structure_pipe,              /* STTestingPrerequisites */
  &stip_traverse_structure_fork,              /* STDoubleMateFilter */
  &stip_traverse_structure_fork,              /* STCounterMateFilter */
  &stip_traverse_structure_pipe,              /* STPrerequisiteOptimiser */
  &stip_traverse_structure_testing_pipe,      /* STNoShortVariations */
  &stip_traverse_structure_pipe,              /* STRestartGuard */
  &stip_traverse_structure_pipe,              /* STRestartGuardIntelligent */
  &stip_traverse_structure_pipe,              /* STIntelligentTargetCounter */
  &stip_traverse_structure_pipe,              /* STMaxTimeGuard */
  &stip_traverse_structure_pipe,              /* STMaxSolutionsInitialiser */
  &stip_traverse_structure_pipe,              /* STMaxSolutionsGuard */
  &stip_traverse_structure_pipe,              /* STMaxSolutionsCounter */
  &stip_traverse_structure_fork,              /* STEndOfBranchGoalImmobile */
  &stip_traverse_structure_pipe,              /* STDeadEndGoal */
  &stip_traverse_structure_pipe,              /* STOrthodoxMatingMoveGenerator */
  &stip_traverse_structure_pipe,              /* STKillerMoveCollector */
  &stip_traverse_structure_pipe,              /* STKillerMoveFinalDefenseMove */
  &stip_traverse_structure_pipe,              /* STEnPassantFilter */
  &stip_traverse_structure_pipe,              /* STCastlingFilter */
  &stip_traverse_structure_pipe,              /* STAttackHashed */
  &stip_traverse_structure_pipe,              /* STAttackHashedTester */
  &stip_traverse_structure_pipe,              /* STHelpHashed */
  &stip_traverse_structure_pipe,              /* STHelpHashedTester */
  &stip_traverse_structure_pipe,              /* STIntelligentMovesLeftInitialiser */
  &stip_traverse_structure_fork,              /* STIntelligentMateFilter */
  &stip_traverse_structure_fork,              /* STIntelligentStalemateFilter */
  &stip_traverse_structure_pipe,              /* STIntelligentProof */
  &stip_traverse_structure_pipe,              /* STGoalReachableGuardFilterMate */
  &stip_traverse_structure_pipe,              /* STGoalReachableGuardFilterStalemate */
  &stip_traverse_structure_pipe,              /* STGoalReachableGuardFilterProof */
  &stip_traverse_structure_pipe,              /* STGoalReachableGuardFilterProofFairy */
  &stip_traverse_structure_pipe,              /* STIntelligentSolutionsPerTargetPosCounter */
  &stip_traverse_structure_pipe,              /* STIntelligentLimitNrSolutionsPerTargetPos */
  &stip_traverse_structure_pipe,              /* STIntelligentDuplicateAvoider */
  &stip_traverse_structure_pipe,              /* STIntelligentImmobilisationCounter */
  &stip_traverse_structure_pipe,              /* STKeepMatingFilter */
  &stip_traverse_structure_fork,              /* STMaxFlightsquares */
  &stip_traverse_structure_pipe,              /* STFlightsquaresCounter */
  &stip_traverse_structure_pipe,              /* STDegenerateTree */
  &stip_traverse_structure_testing_pipe,      /* STMaxNrNonTrivial */
  &stip_traverse_structure_pipe,              /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_testing_pipe,      /* STMaxThreatLength */
  &stip_traverse_structure_pipe,              /* STMaxThreatLengthStart */
  &stip_traverse_structure_pipe,              /* STStopOnShortSolutionsInitialiser */
  &stip_traverse_structure_pipe,              /* STStopOnShortSolutionsFilter */
  &stip_traverse_structure_pipe,              /* STAmuMateFilter */
  &stip_traverse_structure_pipe,              /* STUltraschachzwangGoalFilter */
  &stip_traverse_structure_pipe,              /* STCirceSteingewinnFilter */
  &stip_traverse_structure_pipe,              /* STCirceCircuitSpecial */
  &stip_traverse_structure_pipe,              /* STCirceExchangeSpecial */
  &stip_traverse_structure_pipe,              /* STAnticirceTargetSquareFilter */
  &stip_traverse_structure_pipe,              /* STAnticirceCircuitSpecial */
  &stip_traverse_structure_pipe,              /* STAnticirceExchangeSpecial */
  &stip_traverse_structure_pipe,              /* STAnticirceExchangeFilter */
  &stip_traverse_structure_pipe,              /* STPiecesParalysingMateFilter */
  &stip_traverse_structure_pipe,              /* STPiecesParalysingMateFilterTester */
  &stip_traverse_structure_pipe,              /* STPiecesParalysingStalemateSpecial */
  &stip_traverse_structure_pipe,              /* STPiecesKamikazeTargetSquareFilter */
  &stip_traverse_structure_pipe,              /* STImmobilityTester */
  &stip_traverse_structure_fork,              /* STOpponentMovesCounterFork */
  &stip_traverse_structure_pipe,              /* STOpponentMovesCounter */
  &stip_traverse_structure_pipe,              /* STOhneschachSuspender */
  &stip_traverse_structure_fork,              /* STExclusiveChessMatingMoveCounter */
  &stip_traverse_structure_pipe,              /* STExclusiveChessUnsuspender */
  &stip_traverse_structure_pipe,              /* STMaffImmobilityTesterKing */
  &stip_traverse_structure_pipe,              /* STOWUImmobilityTesterKing */
  &stip_traverse_structure_pipe,              /* STSingleMoveGeneratorWithKingCapture */
  &stip_traverse_structure_fork,              /* STBrunnerDefenderFinder */
  &stip_traverse_structure_fork,              /* STIsardamDefenderFinder */
  &stip_traverse_structure_fork,              /* STCageCirceNonCapturingMoveFinder */
  &stip_traverse_structure_pipe,              /* STSinglePieceMoveGenerator */
  &stip_traverse_structure_pipe,              /* STSingleMoveGenerator */
  &stip_traverse_structure_fork,              /* STMaximummerCandidateMoveTester */
  &stip_traverse_structure_pipe,              /* STBGLFilter */
  &stip_traverse_structure_pipe,              /* STOutputModeSelector */
  &stip_traverse_structure_pipe,              /* STIllegalSelfcheckWriter */
  &stip_traverse_structure_pipe,              /* STEndOfPhaseWriter */
  &stip_traverse_structure_pipe,              /* STEndOfSolutionWriter */
  &stip_traverse_structure_pipe,              /* STThreatWriter */
  &stip_traverse_structure_pipe,              /* STMoveWriter */
  &stip_traverse_structure_pipe,              /* STKeyWriter */
  &stip_traverse_structure_pipe,              /* STTryWriter */
  &stip_traverse_structure_pipe,              /* STZugzwangWriter */
  &stip_traverse_structure_testing_pipe,      /* STTrivialEndFilter */
  &stip_traverse_structure_pipe,              /* STRefutingVariationWriter */
  &stip_traverse_structure_pipe,              /* STRefutationsIntroWriter */
  &stip_traverse_structure_pipe,              /* STRefutationWriter */
  &stip_traverse_structure_pipe,              /* STOutputPlaintextTreeCheckWriter */
  &stip_traverse_structure_pipe,              /* STOutputPlaintextLineLineWriter */
  &stip_traverse_structure_pipe,              /* STOutputPlaintextTreeGoalWriter */
  &stip_traverse_structure_pipe,              /* STOutputPlaintextMoveInversionCounter */
  &stip_traverse_structure_pipe,              /* STOutputPlaintextLineEndOfIntroSeriesMarker */
  &stip_traverse_structure_pipe               /* STMoveTracer */
};

/* Initialise a structure traversal structure with default visitors
 * @param st to be initialised
 * @param param parameter to be passed t operations
 */
void stip_structure_traversal_init(stip_structure_traversal *st, void *param)
{
  {
    unsigned int i;
    for (i = 0; i!=max_nr_slices; ++i)
      st->traversed[i] = slice_not_traversed;
  }

  {
    slice_type i;
    for (i = 0; i!=nr_slice_types; ++i)
      st->map.visitors[i] = structure_children_traversers[i];
  }

  st->level = structure_traversal_level_root;
  st->context = stip_traversal_context_global;

  st->param = param;
}

/* Override the behavior of a structure traversal at slices of a structural type
 * @param st to be initialised
 * @param type type for which to override the visitor (note: subclasses of type
 *             are not affected by
 *             stip_structure_traversal_override_by_structure()! )
 * @param visitor overrider
 */
void stip_structure_traversal_override_by_structure(stip_structure_traversal *st,
                                                    slice_structural_type type,
                                                    stip_structure_visitor visitor)
{
  slice_type i;
  for (i = 0; i!=nr_slice_types; ++i)
    if (highest_structural_type[i]==type)
      st->map.visitors[i] = visitor;
}

/* Override the behavior of a structure traversal at slices of a structural type
 * @param st to be initialised
 * @param type type for which to override the visitor
 * @param visitor overrider
 */
void stip_structure_traversal_override_by_function(stip_structure_traversal *st,
                                                   slice_functional_type type,
                                                   stip_structure_visitor visitor)
{
  slice_type i;
  for (i = 0; i!=nr_slice_types; ++i)
    if (functional_type[i]==type)
      st->map.visitors[i] = visitor;
}

/* Initialise a structure traversal structure with default visitors
 * @param st to be initialised
 * @param type type for which to override the visitor
 * @param visitor overrider
 */
void stip_structure_traversal_override_single(stip_structure_traversal *st,
                                              slice_type type,
                                              stip_structure_visitor visitor)
{
  st->map.visitors[type] = visitor;
}

/* Override some of the visitors of a traversal
 * @param st to be initialised
 * @param visitors overriding visitors
 * @param nr_visitors number of visitors
 */
void
stip_structure_traversal_override(stip_structure_traversal *st,
                                  structure_traversers_visitors const visitors[],
                                  unsigned int nr_visitors)
{
  unsigned int i;
  for (i = 0; i!=nr_visitors; ++i)
    st->map.visitors[visitors[i].type] = visitors[i].visitor;
}

/* (Approximately) depth-first traversl of a stipulation sub-tree
 * @param root root of the sub-tree to traverse
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_children(slice_index si,
                                      stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_visit_slice(si,&structure_children_traversers,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static moves_visitor_map_type const moves_children_traversers =
{
  {
    &stip_traverse_moves_pipe,              /* STProxy */
    &stip_traverse_moves_conditional_pipe,  /* STTemporaryHackFork */
    &stip_traverse_moves_attack_adapter,    /* STAttackAdapter */
    &stip_traverse_moves_defense_adapter,   /* STDefenseAdapter */
    &stip_traverse_moves_ready_for_attack,  /* STReadyForAttack */
    &stip_traverse_moves_ready_for_defense, /* STReadyForDefense */
    &stip_traverse_moves_pipe,              /* STNotEndOfBranchGoal */
    &stip_traverse_moves_pipe,              /* STNotEndOfBranch */
    &stip_traverse_moves_pipe,              /* STMinLengthOptimiser */
    &stip_traverse_moves_help_adapter,      /* STHelpAdapter */
    &stip_traverse_moves_pipe,              /* STReadyForHelpMove */
    &stip_traverse_moves_setplay_fork,      /* STSetplayFork */
    &stip_traverse_moves_end_of_branch,     /* STEndOfBranch */
    &stip_traverse_moves_end_of_branch,     /* STEndOfBranchForced */
    &stip_traverse_moves_binary,            /* STEndOfBranchGoal */
    &stip_traverse_moves_conditional_pipe,  /* STEndOfBranchTester */
    &stip_traverse_moves_binary,            /* STAvoidUnsolvable */
    &stip_traverse_moves_pipe,              /* STResetUnsolvable */
    &stip_traverse_moves_pipe,              /* STLearnUnsolvable */
    &stip_traverse_moves_setplay_fork,      /* STConstraintSolver */
    &stip_traverse_moves_conditional_pipe,  /* STConstraintTester */
    &stip_traverse_moves_pipe,              /* STEndOfRoot */
    &stip_traverse_moves_pipe,              /* STEndOfIntro */
    &stip_traverse_moves_dead_end,          /* STDeadEnd */
    &stip_traverse_moves_pipe,              /* STMove */
    &stip_traverse_moves_pipe,              /* STForEachMove */
    &stip_traverse_moves_pipe,              /* STFindMove */
    &stip_traverse_moves_move_played,       /* STMovePlayed */
    &stip_traverse_moves_move_played,       /* STHelpMovePlayed */
    &stip_traverse_moves_move_played,       /* STDummyMove */
    &stip_traverse_moves_pipe,              /* STReadyForDummyMove */
    &stip_traverse_moves_pipe,              /* STShortSolutionsStart*/
    &stip_traverse_moves_binary,            /* STCheckZigzagJump */
    &stip_traverse_moves_pipe,              /* STCheckZigzagLanding */
    &stip_traverse_moves_conditional_pipe,  /* STGoalReachedTester */
    &stip_traverse_moves_pipe,              /* STGoalMateReachedTester */
    &stip_traverse_moves_pipe,              /* STGoalStalemateReachedTester */
    &stip_traverse_moves_pipe,              /* STGoalDoubleStalemateReachedTester */
    &stip_traverse_moves_pipe,              /* STGoalTargetReachedTester */
    &stip_traverse_moves_pipe,              /* STGoalCheckReachedTester */
    &stip_traverse_moves_pipe,              /* STGoalCaptureReachedTester */
    &stip_traverse_moves_pipe,              /* STGoalSteingewinnReachedTester */
    &stip_traverse_moves_pipe,              /* STGoalEnpassantReachedTester */
    &stip_traverse_moves_pipe,              /* STGoalDoubleMateReachedTester */
    &stip_traverse_moves_pipe,              /* STGoalCounterMateReachedTester */
    &stip_traverse_moves_pipe,              /* STGoalCastlingReachedTester */
    &stip_traverse_moves_pipe,              /* STGoalAutoStalemateReachedTester */
    &stip_traverse_moves_pipe,              /* STGoalCircuitReachedTester */
    &stip_traverse_moves_pipe,              /* STGoalExchangeReachedTester */
    &stip_traverse_moves_pipe,              /* STGoalCircuitByRebirthReachedTester */
    &stip_traverse_moves_pipe,              /* STGoalExchangeByRebirthReachedTester */
    &stip_traverse_moves_pipe,              /* STGoalAnyReachedTester */
    &stip_traverse_moves_pipe,              /* STGoalProofgameReachedTester */
    &stip_traverse_moves_pipe,              /* STGoalAToBReachedTester */
    &stip_traverse_moves_pipe,              /* STGoalMateOrStalemateReachedTester */
    &stip_traverse_moves_pipe,              /* STGoalChess81ReachedTester */
    &stip_traverse_moves_conditional_pipe,  /* STGoalImmobileReachedTester */
    &stip_traverse_moves_pipe,              /* STGoalNotCheckReachedTester */
    &stip_traverse_moves_noop,              /* STTrue */
    &stip_traverse_moves_noop,              /* STFalse */
    &stip_traverse_moves_binary,            /* STAnd */
    &stip_traverse_moves_binary,            /* STOr */
    &stip_traverse_moves_pipe,              /* STCheckDetector */
    &stip_traverse_moves_pipe,              /* STNot */
    &stip_traverse_moves_pipe,              /* STSelfCheckGuard */
    &stip_traverse_moves_pipe,              /* STOhneschachCheckGuard */
    &stip_traverse_moves_pipe,              /* STMoveInverter */
    &stip_traverse_moves_pipe,              /* STMinLengthGuard */
    &stip_traverse_moves_fork_on_remaining, /* STForkOnRemaining */
    &stip_traverse_moves_pipe,              /* STFindShortest */
    &stip_traverse_moves_pipe,              /* STFindByIncreasingLength */
    &stip_traverse_moves_pipe,              /* STGeneratingMoves */
    &stip_traverse_moves_pipe,              /* STMoveGenerator */
    &stip_traverse_moves_pipe,              /* STKingMoveGenerator */
    &stip_traverse_moves_pipe,              /* STNonKingMoveGenerator */
    &stip_traverse_moves_pipe,              /* STCastlingIntermediateMoveGenerator */
    &stip_traverse_moves_conditional_pipe,  /* STCastlingIntermediateMoveLegalityTester */
    &stip_traverse_moves_pipe,              /* STRefutationsAllocator */
    &stip_traverse_moves_binary,            /* STRefutationsSolver */
    &stip_traverse_moves_pipe,              /* STRefutationsFilter */
    &stip_traverse_moves_pipe,              /* STEndOfRefutationSolvingBranch */
    &stip_traverse_moves_pipe,              /* STPlaySuppressor */
    &stip_traverse_moves_pipe,              /* STContinuationSolver */
    &stip_traverse_moves_pipe,              /* STSolvingContinuation */
    &stip_traverse_moves_pipe,              /* STThreatSolver */
    &stip_traverse_moves_pipe,              /* STThreatEnforcer */
    &stip_traverse_moves_pipe,              /* STThreatStart */
    &stip_traverse_moves_pipe,              /* STThreatEnd */
    &stip_traverse_moves_pipe,              /* STThreatCollector */
    &stip_traverse_moves_pipe,              /* STThreatDefeatedTester */
    &stip_traverse_moves_pipe,              /* STRefutationsCollector */
    &stip_traverse_moves_pipe,              /* STRefutationsAvoider */
    &stip_traverse_moves_pipe,              /* STLegalMoveCounter */
    &stip_traverse_moves_pipe,              /* STAnyMoveCounter */
    &stip_traverse_moves_pipe,              /* STCaptureCounter */
    &stip_traverse_moves_pipe,              /* STTestingPrerequisites */
    &stip_traverse_moves_conditional_pipe,  /* STDoubleMateFilter */
    &stip_traverse_moves_conditional_pipe,  /* STCounterMateFilter */
    &stip_traverse_moves_pipe,              /* STPrerequisiteOptimiser */
    &stip_traverse_moves_pipe,              /* STNoShortVariations */
    &stip_traverse_moves_pipe,              /* STRestartGuard */
    &stip_traverse_moves_pipe,              /* STRestartGuardIntelligent */
    &stip_traverse_moves_pipe,              /* STIntelligentTargetCounter */
    &stip_traverse_moves_pipe,              /* STMaxTimeGuard */
    &stip_traverse_moves_pipe,              /* STMaxSolutionsInitialiser */
    &stip_traverse_moves_pipe,              /* STMaxSolutionsGuard */
    &stip_traverse_moves_pipe,              /* STMaxSolutionsCounter */
    &stip_traverse_moves_end_of_branch,     /* STEndOfBranchGoalImmobile */
    &stip_traverse_moves_dead_end,          /* STDeadEndGoal */
    &stip_traverse_moves_pipe,              /* STOrthodoxMatingMoveGenerator */
    &stip_traverse_moves_pipe,              /* STKillerMoveCollector */
    &stip_traverse_moves_pipe,              /* STKillerMoveFinalDefenseMove */
    &stip_traverse_moves_pipe,              /* STEnPassantFilter */
    &stip_traverse_moves_pipe,              /* STCastlingFilter */
    &stip_traverse_moves_pipe,              /* STAttackHashed */
    &stip_traverse_moves_pipe,              /* STAttackHashedTester */
    &stip_traverse_moves_pipe,              /* STHelpHashed */
    &stip_traverse_moves_pipe,              /* STHelpHashedTester */
    &stip_traverse_moves_pipe,              /* STIntelligentMovesLeftInitialiser */
    &stip_traverse_moves_conditional_pipe,  /* STIntelligentMateFilter */
    &stip_traverse_moves_conditional_pipe,  /* STIntelligentStalemateFilter */
    &stip_traverse_moves_pipe,              /* STIntelligentProof */
    &stip_traverse_moves_pipe,              /* STGoalReachableGuardFilterMate */
    &stip_traverse_moves_pipe,              /* STGoalReachableGuardFilterStalemate */
    &stip_traverse_moves_pipe,              /* STGoalReachableGuardFilterProof */
    &stip_traverse_moves_pipe,              /* STGoalReachableGuardFilterProofFairy */
    &stip_traverse_moves_pipe,              /* STIntelligentSolutionsPerTargetPosCounter */
    &stip_traverse_moves_pipe,              /* STIntelligentLimitNrSolutionsPerTargetPos */
    &stip_traverse_moves_pipe,              /* STIntelligentDuplicateAvoider */
    &stip_traverse_moves_pipe,              /* STIntelligentImmobilisationCounter */
    &stip_traverse_moves_pipe,              /* STKeepMatingFilter */
    &stip_traverse_moves_conditional_pipe,  /* STMaxFlightsquares */
    &stip_traverse_moves_pipe,              /* STFlightsquaresCounter */
    &stip_traverse_moves_pipe,              /* STDegenerateTree */
    &stip_traverse_moves_pipe,              /* STMaxNrNonTrivial */
    &stip_traverse_moves_pipe,              /* STMaxNrNonTrivialCounter */
    &stip_traverse_moves_pipe,              /* STMaxThreatLength */
    &stip_traverse_moves_pipe,              /* STMaxThreatLengthStart */
    &stip_traverse_moves_pipe,              /* STStopOnShortSolutionsInitialiser */
    &stip_traverse_moves_pipe,              /* STStopOnShortSolutionsFilter */
    &stip_traverse_moves_pipe,              /* STAmuMateFilter */
    &stip_traverse_moves_pipe,              /* STUltraschachzwangGoalFilter */
    &stip_traverse_moves_pipe,              /* STCirceSteingewinnFilter */
    &stip_traverse_moves_pipe,              /* STCirceCircuitSpecial */
    &stip_traverse_moves_pipe,              /* STCirceExchangeSpecial */
    &stip_traverse_moves_pipe,              /* STAnticirceTargetSquareFilter */
    &stip_traverse_moves_pipe,              /* STAnticirceCircuitSpecial */
    &stip_traverse_moves_pipe,              /* STAnticirceExchangeSpecial */
    &stip_traverse_moves_pipe,              /* STAnticirceExchangeFilter */
    &stip_traverse_moves_pipe,              /* STPiecesParalysingMateFilter */
    &stip_traverse_moves_pipe,              /* STPiecesParalysingMateFilterTester */
    &stip_traverse_moves_pipe,              /* STPiecesParalysingStalemateSpecial */
    &stip_traverse_moves_pipe,              /* STPiecesKamikazeTargetSquareFilter */
    &stip_traverse_moves_pipe,              /* STImmobilityTester */
    &stip_traverse_moves_conditional_pipe,  /* STOpponentMovesCounterFork */
    &stip_traverse_moves_pipe,              /* STOpponentMovesCounter */
    &stip_traverse_moves_pipe,              /* STOhneschachSuspender */
    &stip_traverse_moves_conditional_pipe,  /* STExclusiveChessMatingMoveCounter */
    &stip_traverse_moves_pipe,              /* STExclusiveChessUnsuspender */
    &stip_traverse_moves_pipe,              /* STMaffImmobilityTesterKing */
    &stip_traverse_moves_pipe,              /* STOWUImmobilityTesterKing */
    &stip_traverse_moves_pipe,              /* STSingleMoveGeneratorWithKingCapture */
    &stip_traverse_moves_conditional_pipe,  /* STBrunnerDefenderFinder */
    &stip_traverse_moves_conditional_pipe,  /* STIsardamDefenderFinder */
    &stip_traverse_moves_conditional_pipe,  /* STCageCirceNonCapturingMoveFinder */
    &stip_traverse_moves_pipe,              /* STSinglePieceMoveGenerator */
    &stip_traverse_moves_pipe,              /* STSingleMoveGenerator */
    &stip_traverse_moves_conditional_pipe,  /* STMaximummerCandidateMoveTester */
    &stip_traverse_moves_pipe,              /* STBGLFilter */
    &stip_traverse_moves_pipe,              /* STOutputModeSelector */
    &stip_traverse_moves_pipe,              /* STIllegalSelfcheckWriter */
    &stip_traverse_moves_pipe,              /* STEndOfPhaseWriter */
    &stip_traverse_moves_pipe,              /* STEndOfSolutionWriter */
    &stip_traverse_moves_pipe,              /* STThreatWriter */
    &stip_traverse_moves_pipe,              /* STMoveWriter */
    &stip_traverse_moves_pipe,              /* STKeyWriter */
    &stip_traverse_moves_pipe,              /* STTryWriter */
    &stip_traverse_moves_pipe,              /* STZugzwangWriter */
    &stip_traverse_moves_pipe,              /* STTrivialEndFilter */
    &stip_traverse_moves_pipe,              /* STRefutingVariationWriter */
    &stip_traverse_moves_pipe,              /* STRefutationsIntroWriter */
    &stip_traverse_moves_pipe,              /* STRefutationWriter */
    &stip_traverse_moves_pipe,              /* STOutputPlaintextTreeCheckWriter */
    &stip_traverse_moves_pipe,              /* STOutputPlaintextLineLineWriter */
    &stip_traverse_moves_pipe,              /* STOutputPlaintextTreeGoalWriter */
    &stip_traverse_moves_pipe,              /* STOutputPlaintextMoveInversionCounter */
    &stip_traverse_moves_pipe,              /* STOutputPlaintextLineEndOfIntroSeriesMarker */
    &stip_traverse_moves_pipe               /* STMoveTracer */
  }
};

/* Initialise a move traversal structure with default visitors
 * @param st to be initialised
 * @param param parameter to be passed t operations
 */
void stip_moves_traversal_init(stip_moves_traversal *st, void *param)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  st->map = moves_children_traversers;

  for (i = 0; i!=nr_slice_types; ++i)
    st->map.visitors[i] = moves_children_traversers.visitors[i];

  st->context = stip_traversal_context_global;
  st->remaining = STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED;
  st->full_length = STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED;
  st->param = param;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Set the number of moves at the start of the traversal. Normally, this is
 * determined while traversing the stipulation. Only invoke
 * stip_moves_traversal_set_remaining() when the traversal is started in the
 * middle of a stipulation.
 * @param st to be initialised
 * @param remaining number of remaining moves (without slack)
 * @param full_length full number of moves of the initial branch (without slack)
 */
void stip_moves_traversal_set_remaining(stip_moves_traversal *st,
                                        stip_length_type remaining,
                                        stip_length_type full_length)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  st->remaining = remaining;
  st->full_length = full_length;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Override the behavior of a moves traversal at some slice types
 * @param st to be initialised
 * @param moves_traversers_visitors array of alternative visitors; for
 *                                  slices with types not mentioned in
 *                                  moves_traversers_visitors, the default
 *                                  visitor will be used
 * @param nr_visitors length of moves_traversers_visitors
 */
void stip_moves_traversal_override(stip_moves_traversal *st,
                                   moves_traversers_visitors const visitors[],
                                   unsigned int nr_visitors)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 0; i<nr_visitors; ++i)
    st->map.visitors[visitors[i].type] = visitors[i].visitor;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Override the behavior of a moves traversal at slices of a structural type
 * @param st to be initialised
 * @param type type for which to override the visitor
 * @param visitor overrider
 */
void stip_moves_traversal_override_by_structure(stip_moves_traversal *st,
                                                slice_structural_type type,
                                                stip_moves_visitor visitor)
{
  slice_type i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 0; i!=nr_slice_types; ++i)
    if (highest_structural_type[i]==type)
      st->map.visitors[i] = visitor;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Override the behavior of a moves traversal at slices of a functional type
 * @param st to be initialised
 * @param type type for which to override the visitor
 * @param visitor overrider
 */
void stip_moves_traversal_override_by_function(stip_moves_traversal *st,
                                               slice_functional_type type,
                                               stip_moves_visitor visitor)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 0; i!=nr_slice_types; ++i)
    if (functional_type[i]==type)
      st->map.visitors[i] = visitor;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Override the behavior of a moves traversal at slices of a structural type
 * @param st to be initialised
 * @param type type for which to override the visitor
 * @param visitor overrider
 */
void stip_moves_traversal_override_single(stip_moves_traversal *st,
                                          slice_type type,
                                          stip_moves_visitor visitor)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  st->map.visitors[type] = visitor;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* (Approximately) depth-first traversl of the stipulation
 * @param root start of the stipulation (sub)tree
 * @param st address of data structure holding parameters for the operation
 */
void stip_traverse_moves(slice_index root, stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",st->remaining);

  TraceEnumerator(slice_type,slices[root].type,"\n");
  assert(slices[root].type<=nr_slice_types);

  {
    stip_moves_visitor const operation = st->map.visitors[slices[root].type];
    assert(operation!=0);
    (*operation)(root,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* No-op callback for move traversals
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_noop(slice_index si, stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* (Approximately) depth-first traversl of a stipulation sub-tree
 * @param root root of the sub-tree to traverse
 * @param st address of structure defining traversal
 */
void stip_traverse_moves_children(slice_index si,
                                  stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(slice_type,slices[si].type,"\n");
  assert(slices[si].type<=nr_slice_types);

  {
    slice_type const type = slices[si].type;
    stip_moves_visitor const operation = moves_children_traversers.visitors[type];
    assert(operation!=0);
    (*operation)(si,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
