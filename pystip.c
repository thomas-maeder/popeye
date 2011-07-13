#include "pystip.h"
#include "pydata.h"
#include "pybrafrk.h"
#include "pyhash.h"
#include "pymovein.h"
#include "pykeepmt.h"
#include "pyselfcg.h"
#include "pyselfgd.h"
#include "pydirctg.h"
#include "pypipe.h"
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
#include "stipulation/move.h"
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
#include "optimisations/goals/enpassant/filter.h"
#include "optimisations/intelligent/filter.h"
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
  slice_structure_pipe,   /* STProxy */
  slice_structure_fork,   /* STTemporaryHackFork */
  slice_structure_branch, /* STAttackAdapter */
  slice_structure_branch, /* STDefenseAdapter */
  slice_structure_branch, /* STReadyForAttack */
  slice_structure_branch, /* STReadyForDefense */
  slice_structure_branch, /* STMinLengthOptimiser */
  slice_structure_branch, /* STHelpAdapter */
  slice_structure_branch, /* STReadyForHelpMove */
  slice_structure_fork,   /* STSetplayFork */
  slice_structure_fork,   /* STEndOfBranch */
  slice_structure_fork,   /* STEndOfBranchForced */
  slice_structure_fork,   /* STEndOfBranchGoal */
  slice_structure_fork,   /* STConstraint */
  slice_structure_pipe,   /* STEndOfRoot */
  slice_structure_pipe,   /* STEndOfIntro */
  slice_structure_pipe,   /* STDeadEnd */
  slice_structure_pipe,   /* STMove */
  slice_structure_pipe,   /* STDummyMove */
  slice_structure_branch, /* STReadyForDummyMove */
  slice_structure_pipe,   /* STShortSolutionsStart*/
  slice_structure_binary, /* STCheckZigzagJump */
  slice_structure_pipe,   /* STCheckZigzagLanding */
  slice_structure_fork,   /* STGoalReachedTester */
  slice_structure_pipe,   /* STGoalMateReachedTester */
  slice_structure_pipe,   /* STGoalStalemateReachedTester */
  slice_structure_pipe,   /* STGoalDoubleStalemateReachedTester */
  slice_structure_pipe,   /* STGoalTargetReachedTester */
  slice_structure_pipe,   /* STGoalCheckReachedTester */
  slice_structure_pipe,   /* STGoalCaptureReachedTester */
  slice_structure_pipe,   /* STGoalSteingewinnReachedTester */
  slice_structure_pipe,   /* STGoalEnpassantReachedTester */
  slice_structure_pipe,   /* STGoalDoubleMateReachedTester */
  slice_structure_pipe,   /* STGoalCounterMateReachedTester */
  slice_structure_pipe,   /* STGoalCastlingReachedTester */
  slice_structure_pipe,   /* STGoalAutoStalemateReachedTester */
  slice_structure_pipe,   /* STGoalCircuitReachedTester */
  slice_structure_pipe,   /* STGoalExchangeReachedTester */
  slice_structure_pipe,   /* STGoalCircuitByRebirthReachedTester */
  slice_structure_pipe,   /* STGoalExchangeByRebirthReachedTester */
  slice_structure_pipe,   /* STGoalAnyReachedTester */
  slice_structure_pipe,   /* STGoalProofgameReachedTester */
  slice_structure_pipe,   /* STGoalAToBReachedTester */
  slice_structure_pipe,   /* STGoalMateOrStalemateReachedTester */
  slice_structure_fork,   /* STGoalImmobileReachedTester */
  slice_structure_pipe,   /* STGoalNotCheckReachedTester */
  slice_structure_leaf,   /* STTrue */
  slice_structure_leaf,   /* STFalse */
  slice_structure_binary, /* STAnd */
  slice_structure_binary, /* STOr */
  slice_structure_pipe,   /* STCheckDetector */
  slice_structure_pipe,   /* STNot */
  slice_structure_pipe,   /* STSelfCheckGuard */
  slice_structure_pipe,   /* STOhneschachCheckGuard */
  slice_structure_pipe,   /* STMoveInverter */
  slice_structure_branch, /* STMinLengthGuard */
  slice_structure_binary, /* STForkOnRemaining */
  slice_structure_branch, /* STFindShortest */
  slice_structure_branch, /* STFindByIncreasingLength */
  slice_structure_pipe,   /* STGeneratingMoves */
  slice_structure_pipe,   /* STMoveGenerator */
  slice_structure_pipe,   /* STKingMoveGenerator */
  slice_structure_pipe,   /* STNonKingMoveGenerator */
  slice_structure_pipe,   /* STRefutationsAllocator */
  slice_structure_pipe,   /* STTrySolver */
  slice_structure_pipe,   /* STRefutationsSolver */
  slice_structure_pipe,   /* STPlaySuppressor */
  slice_structure_pipe,   /* STContinuationSolver */
  slice_structure_fork,   /* STThreatSolver */
  slice_structure_fork,   /* STThreatEnforcer */
  slice_structure_pipe,   /* STThreatStart */
  slice_structure_pipe,   /* STThreatCollector */
  slice_structure_pipe,   /* STRefutationsCollector */
  slice_structure_pipe,   /* STLegalMoveCounter */
  slice_structure_pipe,   /* STAnyMoveCounter */
  slice_structure_pipe,   /* STCaptureCounter */
  slice_structure_pipe,   /* STTestingPrerequisites */
  slice_structure_fork,   /* STDoubleMateFilter */
  slice_structure_fork,   /* STCounterMateFilter */
  slice_structure_pipe,   /* STPrerequisiteOptimiser */
  slice_structure_pipe,   /* STNoShortVariations */
  slice_structure_pipe,   /* STRestartGuard */
  slice_structure_pipe,   /* STMaxTimeGuard */
  slice_structure_pipe,   /* STMaxSolutionsInitialiser */
  slice_structure_pipe,   /* STMaxSolutionsGuard */
  slice_structure_fork,   /* STEndOfBranchGoalImmobile */
  slice_structure_pipe,   /* STDeadEndGoal */
  slice_structure_pipe,   /* STOrthodoxMatingMoveGenerator */
  slice_structure_pipe,   /* STKillerMoveCollector */
  slice_structure_pipe,   /* STKillerMoveMoveGenerator */
  slice_structure_pipe,   /* STKillerMoveFinalDefenseMove */
  slice_structure_pipe,   /* STCountNrOpponentMovesMoveGenerator */
  slice_structure_pipe,   /* STEnPassantFilter */
  slice_structure_pipe,   /* STCastlingFilter */
  slice_structure_branch, /* STAttackHashed */
  slice_structure_branch, /* STHelpHashed */
  slice_structure_fork,   /* STIntelligentFilter */
  slice_structure_pipe,   /* STGoalReachableGuardFilter */
  slice_structure_pipe,   /* STIntelligentDuplicateAvoider */
  slice_structure_pipe,   /* STKeepMatingFilter */
  slice_structure_fork,   /* STMaxFlightsquares */
  slice_structure_pipe,   /* STFlightsquaresCounter */
  slice_structure_pipe,   /* STDegenerateTree */
  slice_structure_pipe,   /* STMaxNrNonTrivial */
  slice_structure_pipe,   /* STMaxNrNonTrivialCounter */
  slice_structure_fork,   /* STMaxThreatLength */
  slice_structure_pipe,   /* STMaxThreatLengthStart */
  slice_structure_pipe,   /* STStopOnShortSolutionsInitialiser */
  slice_structure_branch, /* STStopOnShortSolutionsFilter */
  slice_structure_pipe,   /* STAmuMateFilter */
  slice_structure_pipe,   /* STUltraschachzwangGoalFilter */
  slice_structure_pipe,   /* STCirceSteingewinnFilter */
  slice_structure_pipe,   /* STCirceCircuitSpecial */
  slice_structure_pipe,   /* STCirceExchangeSpecial */
  slice_structure_pipe,   /* STAnticirceTargetSquareFilter */
  slice_structure_pipe,   /* STAnticirceCircuitSpecial */
  slice_structure_pipe,   /* STAnticirceExchangeSpecial */
  slice_structure_pipe,   /* STAnticirceExchangeFilter */
  slice_structure_pipe,   /* STPiecesParalysingMateFilter */
  slice_structure_pipe,   /* STPiecesParalysingStalemateSpecial */
  slice_structure_pipe,   /* STPiecesKamikazeTargetSquareFilter */
  slice_structure_pipe,   /* STImmobilityTester */
  slice_structure_pipe,   /* STOhneschachSuspender */
  slice_structure_fork,   /* STExclusiveChessMatingMoveCounter */
  slice_structure_pipe,   /* STExclusiveChessUnsuspender */
  slice_structure_pipe,   /* STMaffImmobilityTesterKing */
  slice_structure_pipe,   /* STOWUImmobilityTesterKing) */
  slice_structure_pipe,   /* STSingleMoveGeneratorWithKingCapture */
  slice_structure_fork,   /* STBrunnerDefenderFinder */
  slice_structure_fork,   /* STIsardamDefenderFinder */
  slice_structure_pipe,   /* STOutputModeSelector */
  slice_structure_pipe,   /* STIllegalSelfcheckWriter */
  slice_structure_pipe,   /* STEndOfPhaseWriter */
  slice_structure_pipe,   /* STEndOfSolutionWriter */
  slice_structure_pipe,   /* STMoveWriter */
  slice_structure_pipe,   /* STKeyWriter */
  slice_structure_pipe,   /* STTryWriter */
  slice_structure_pipe,   /* STZugzwangWriter */
  slice_structure_pipe,   /* STTrivialEndFilter */
  slice_structure_pipe,   /* STRefutingVariationWriter */
  slice_structure_pipe,   /* STRefutationWriter */
  slice_structure_pipe,   /* STOutputPlaintextTreeCheckWriter */
  slice_structure_pipe,   /* STOutputPlaintextTreeDecorationWriter */
  slice_structure_pipe,   /* STOutputPlaintextLineLineWriter */
  slice_structure_pipe,   /* STOutputPlaintextTreeGoalWriter */
  slice_structure_pipe,   /* STOutputPlaintextMoveInversionCounter */
  slice_structure_pipe    /* STOutputPlaintextLineEndOfIntroSeriesMarker */
};

static slice_functional_type functional_type[nr_slice_types] =
{
  slice_function_unspecified,    /* STProxy */
  slice_function_unspecified,    /* STTemporaryHackFork */
  slice_function_unspecified,    /* STAttackAdapter */
  slice_function_unspecified,    /* STDefenseAdapter */
  slice_function_unspecified,    /* STReadyForAttack */
  slice_function_unspecified,    /* STReadyForDefense */
  slice_function_unspecified,    /* STMinLengthOptimiser */
  slice_function_unspecified,    /* STHelpAdapter */
  slice_function_unspecified,    /* STReadyForHelpMove */
  slice_function_unspecified,    /* STSetplayFork */
  slice_function_unspecified,    /* STEndOfBranch */
  slice_function_unspecified,    /* STEndOfBranchForced */
  slice_function_unspecified,    /* STEndOfBranchGoal */
  slice_function_unspecified,    /* STConstraint */
  slice_function_unspecified,    /* STEndOfRoot */
  slice_function_unspecified,    /* STEndOfIntro */
  slice_function_unspecified,    /* STDeadEnd */
  slice_function_unspecified,    /* STMove */
  slice_function_unspecified,    /* STDummyMove */
  slice_function_unspecified,    /* STReadyForDummyMove */
  slice_function_unspecified,    /* STShortSolutionsStart*/
  slice_function_unspecified,    /* STCheckZigzagJump */
  slice_function_unspecified,    /* STCheckZigzagLanding */
  slice_function_unspecified,    /* STGoalReachedTester */
  slice_function_unspecified,    /* STGoalMateReachedTester */
  slice_function_unspecified,    /* STGoalStalemateReachedTester */
  slice_function_unspecified,    /* STGoalDoubleStalemateReachedTester */
  slice_function_unspecified,    /* STGoalTargetReachedTester */
  slice_function_unspecified,    /* STGoalCheckReachedTester */
  slice_function_unspecified,    /* STGoalCaptureReachedTester */
  slice_function_unspecified,    /* STGoalSteingewinnReachedTester */
  slice_function_unspecified,    /* STGoalEnpassantReachedTester */
  slice_function_unspecified,    /* STGoalDoubleMateReachedTester */
  slice_function_unspecified,    /* STGoalCounterMateReachedTester */
  slice_function_unspecified,    /* STGoalCastlingReachedTester */
  slice_function_unspecified,    /* STGoalAutoStalemateReachedTester */
  slice_function_unspecified,    /* STGoalCircuitReachedTester */
  slice_function_unspecified,    /* STGoalExchangeReachedTester */
  slice_function_unspecified,    /* STGoalCircuitByRebirthReachedTester */
  slice_function_unspecified,    /* STGoalExchangeByRebirthReachedTester */
  slice_function_unspecified,    /* STGoalAnyReachedTester */
  slice_function_unspecified,    /* STGoalProofgameReachedTester */
  slice_function_unspecified,    /* STGoalAToBReachedTester */
  slice_function_unspecified,    /* STGoalMateOrStalemateReachedTester */
  slice_function_unspecified,    /* STGoalImmobileReachedTester */
  slice_function_unspecified,    /* STGoalNotCheckReachedTester */
  slice_function_unspecified,    /* STTrue */
  slice_function_unspecified,    /* STFalse */
  slice_function_unspecified,    /* STAnd */
  slice_function_unspecified,    /* STOr */
  slice_function_unspecified,    /* STCheckDetector */
  slice_function_unspecified,    /* STNot */
  slice_function_unspecified,    /* STSelfCheckGuard */
  slice_function_unspecified,    /* STOhneschachCheckGuard */
  slice_function_unspecified,    /* STMoveInverter */
  slice_function_unspecified,    /* STMinLengthGuard */
  slice_function_unspecified,    /* STForkOnRemaining */
  slice_function_unspecified,    /* STFindShortest */
  slice_function_unspecified,    /* STFindByIncreasingLength */
  slice_function_unspecified,    /* STGeneratingMoves */
  slice_function_move_generator, /* STMoveGenerator */
  slice_function_move_generator, /* STKingMoveGenerator */
  slice_function_move_generator, /* STNonKingMoveGenerator */
  slice_function_unspecified,    /* STRefutationsAllocator */
  slice_function_unspecified,    /* STTrySolver */
  slice_function_unspecified,    /* STRefutationsSolver */
  slice_function_unspecified,    /* STPlaySuppressor */
  slice_function_unspecified,    /* STContinuationSolver */
  slice_function_unspecified,    /* STThreatSolver */
  slice_function_unspecified,    /* STThreatEnforcer */
  slice_function_unspecified,    /* STThreatStart */
  slice_function_unspecified,    /* STThreatCollector */
  slice_function_unspecified,    /* STRefutationsCollector */
  slice_function_unspecified,    /* STLegalMoveCounter */
  slice_function_unspecified,    /* STAnyMoveCounter */
  slice_function_unspecified,    /* STCaptureCounter */
  slice_function_unspecified,    /* STTestingPrerequisites */
  slice_function_unspecified,    /* STDoubleMateFilter */
  slice_function_unspecified,    /* STCounterMateFilter */
  slice_function_unspecified,    /* STPrerequisiteOptimiser */
  slice_function_unspecified,    /* STNoShortVariations */
  slice_function_unspecified,    /* STRestartGuard */
  slice_function_unspecified,    /* STMaxTimeGuard */
  slice_function_unspecified,    /* STMaxSolutionsInitialiser */
  slice_function_unspecified,    /* STMaxSolutionsGuard */
  slice_function_unspecified,    /* STEndOfBranchGoalImmobile */
  slice_function_unspecified,    /* STDeadEndGoal */
  slice_function_move_generator, /* STOrthodoxMatingMoveGenerator */
  slice_function_unspecified,    /* STKillerMoveCollector */
  slice_function_move_generator, /* STKillerMoveMoveGenerator */
  slice_function_move_generator, /* STKillerMoveFinalDefenseMove */
  slice_function_move_generator, /* STCountNrOpponentMovesMoveGenerator */
  slice_function_unspecified,    /* STEnPassantFilter */
  slice_function_unspecified,    /* STCastlingFilter */
  slice_function_unspecified,    /* STAttackHashed */
  slice_function_unspecified,    /* STHelpHashed */
  slice_function_unspecified,    /* STIntelligentFilter */
  slice_function_unspecified,    /* STGoalReachableGuardFilter */
  slice_function_unspecified,    /* STIntelligentDuplicateAvoider */
  slice_function_unspecified,    /* STKeepMatingFilter */
  slice_function_unspecified,    /* STMaxFlightsquares */
  slice_function_unspecified,    /* STFlightsquaresCounter */
  slice_function_unspecified,    /* STDegenerateTree */
  slice_function_unspecified,    /* STMaxNrNonTrivial */
  slice_function_unspecified,    /* STMaxNrNonTrivialCounter */
  slice_function_unspecified,    /* STMaxThreatLength */
  slice_function_unspecified,    /* STMaxThreatLengthStart */
  slice_function_unspecified,    /* STStopOnShortSolutionsInitialiser */
  slice_function_unspecified,    /* STStopOnShortSolutionsFilter */
  slice_function_unspecified,    /* STAmuMateFilter */
  slice_function_unspecified,    /* STUltraschachzwangGoalFilter */
  slice_function_unspecified,    /* STCirceSteingewinnFilter */
  slice_function_unspecified,    /* STCirceCircuitSpecial */
  slice_function_unspecified,    /* STCirceExchangeSpecial */
  slice_function_unspecified,    /* STAnticirceTargetSquareFilter */
  slice_function_unspecified,    /* STAnticirceCircuitSpecial */
  slice_function_unspecified,    /* STAnticirceExchangeSpecial */
  slice_function_unspecified,    /* STAnticirceExchangeFilter */
  slice_function_unspecified,    /* STPiecesParalysingMateFilter */
  slice_function_unspecified,    /* STPiecesParalysingStalemateSpecial */
  slice_function_unspecified,    /* STPiecesKamikazeTargetSquareFilter */
  slice_function_unspecified,    /* STImmobilityTester */
  slice_function_unspecified,    /* STOhneschachSuspender */
  slice_function_unspecified,    /* STExclusiveChessMatingMoveCounter */
  slice_function_unspecified,    /* STExclusiveChessUnsuspender */
  slice_function_unspecified,    /* STMaffImmobilityTesterKing */
  slice_function_unspecified,    /* STOWUImmobilityTesterKing */
  slice_function_move_generator, /* STSingleMoveGeneratorWithKingCapture */
  slice_function_unspecified,    /* STBrunnerDefenderFinder */
  slice_function_unspecified,    /* STIsardamDefenderFinder */
  slice_function_unspecified,    /* STOutputModeSelector */
  slice_function_unspecified,    /* STIllegalSelfcheckWriter */
  slice_function_unspecified,    /* STEndOfPhaseWriter */
  slice_function_unspecified,    /* STEndOfSolutionWriter */
  slice_function_unspecified,    /* STMoveWriter */
  slice_function_unspecified,    /* STKeyWriter */
  slice_function_unspecified,    /* STTryWriter */
  slice_function_unspecified,    /* STZugzwangWriter */
  slice_function_unspecified,    /* STTrivialEndFilter */
  slice_function_unspecified,    /* STRefutingVariationWriter */
  slice_function_unspecified,    /* STRefutationWriter */
  slice_function_unspecified,    /* STOutputPlaintextTreeCheckWriter */
  slice_function_unspecified,    /* STOutputPlaintextTreeDecorationWriter */
  slice_function_unspecified,    /* STOutputPlaintextLineLineWriter */
  slice_function_unspecified,    /* STOutputPlaintextTreeGoalWriter */
  slice_function_unspecified,    /* STOutputPlaintextMoveInversionCounter */
  slice_function_unspecified     /* STOutputPlaintextLineEndOfIntroSeriesMarker */
};

/* Provide a subclass relationship between the values of slice_structural_type
 * @param derived
 * @param base
 * @return true iff derived is a subclass of base
 */
boolean slice_structure_is_subclass(slice_structural_type derived,
                                    slice_structural_type base)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_structural_type,derived,"");
  TraceEnumerator(slice_structural_type,base,"");
  TraceFunctionParamListEnd();

  switch (derived)
  {
    case slice_structure_leaf:
      result = base==slice_structure_leaf;
      break;

    case slice_structure_binary:
      result = base==slice_structure_binary;
      break;

    case slice_structure_pipe:
      result = base==slice_structure_pipe;
      break;

    case slice_structure_branch:
      result = base==slice_structure_pipe || base==slice_structure_branch;
      break;

    case slice_structure_fork:
      result = base==slice_structure_pipe || base==slice_structure_fork;
      break;

    default:
      assert(0);
      result = false;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice is of some structural type
 * @param si identifies slice
 * @param type identifies type
 * @return true iff slice si has (at least) structural type type
 */
boolean slice_has_structure(slice_index si, slice_structural_type type)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(slice_structural_type,type,"");
  TraceFunctionParamListEnd();

  TraceEnumerator(slice_type,slices[si].type,"");
  TraceEnumerator(slice_structural_type,
                  highest_structural_type[slices[si].type],
                  "\n");

  result = slice_structure_is_subclass(highest_structural_type[slices[si].type],
                                       type);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Retrieve the structural type of a slice
 * @param si identifies slice of which to retrieve structural type
 * @return structural type of slice si
 */
slice_structural_type slice_get_structural_type(slice_index si)
{
  /* no Trace instrumentation here - this is used by the Trace machinery! */
  return highest_structural_type[slices[si].type];
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
  {
    slice_structural_type i;
    for (i = 0; i!=nr_slice_structure_types; ++i)
      if (slice_structure_is_subclass(i,slice_structure_pipe))
        stip_structure_traversal_override_by_structure(&st,i,&move_to_root);
  }
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

static structure_traversers_visitors intro_slice_inserters[] =
{
  { STAttackAdapter,     &attack_adapter_make_intro   },
  { STDefenseAdapter,    &defense_adapter_make_intro  },
  { STHelpAdapter,       &help_adapter_make_intro     },
  { STGoalReachedTester, &stip_structure_visitor_noop }
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
  {
    slice_structural_type i;
    for (i = 0; i!=nr_slice_structure_types; ++i)
      if (slice_structure_is_subclass(i,slice_structure_pipe))
        stip_structure_traversal_override_by_structure(&st,i,&link_to_intro);
  }
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
  goal_type const goal = slices[si].u.goal_tester.goal.type;

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

/* Auxiliary data structor for deep_copy: remembers slice copies
 * already made
 */
typedef slice_index copies_type[max_nr_slices];

/* Recursive implementation of in-place deep copying a stipulation
 * sub-tree
 * @param si root of sub-tree
 * @param copies address of array remembering what copies have already
 *               been made
 */
static slice_index deep_copy_recursive(slice_index si, copies_type *copies)
{
  slice_index result = (*copies)[si];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (result==no_slice)
  {
    result = copy_slice(si);
    slices[result].starter = no_side;
    (*copies)[si] = result;

    TraceEnumerator(slice_type,slices[si].type,"\n");
    switch (highest_structural_type[slices[si].type])
    {
      case slice_structure_leaf:
        /* nothing */
        break;

      case slice_structure_pipe:
      case slice_structure_branch:
      {
        slice_index const next = slices[si].u.pipe.next;
        if (next!=no_slice)
        {
          slice_index const next_copy = deep_copy_recursive(next,copies);
          if (slices[next].prev==si)
            pipe_link(result,next_copy);
          else
            pipe_set_successor(result,next_copy);
        }
        break;
      }

      case slice_structure_fork:
      {
        slice_index const fork = slices[si].u.fork.fork;
        slice_index const next = slices[si].u.pipe.next;
        if (fork!=no_slice)
        {
          slice_index const fork_copy = deep_copy_recursive(fork,copies);
          slices[result].u.fork.fork = fork_copy;
        }
        if (next!=no_slice)
        {
          slice_index const next_copy = deep_copy_recursive(next,copies);
          if (slices[next].prev==si)
            pipe_link(result,next_copy);
          else
            pipe_set_successor(result,next_copy);
        }
        break;
      }

      case slice_structure_binary:
      {
        slice_index const op1 = slices[si].u.binary.op1;
        slice_index const op2 = slices[si].u.binary.op2;
        slices[result].u.binary.op1 = deep_copy_recursive(op1,copies);
        slices[result].u.binary.op2 = deep_copy_recursive(op2,copies);
        break;
      }

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* in-place deep copying a stipulation sub-tree
 * @param si root of sub-tree
 * @return index of root of copy
 */
slice_index stip_deep_copy(slice_index si)
{
  copies_type copies;
  slice_index i;
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  for (i = 0; i!=max_nr_slices; ++i)
    copies[i] = no_slice;

  result = deep_copy_recursive(si,&copies);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

typedef struct
{
  slice_index to_goal;
  boolean has_attack_ended;
} quodlibet_transformation_state;

static void remember_end_of_attack(slice_index si, stip_structure_traversal *st)
{
  quodlibet_transformation_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->has_attack_ended = true;
  stip_traverse_structure_children(si,st);

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
      && slices[si].u.branch.length>slack_length_battle
      && state->to_goal!=no_slice)
  {
    stip_make_direct_goal_branch(state->to_goal);
    battle_branch_insert_direct_end_of_branch_goal(si,state->to_goal);
  }

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

static structure_traversers_visitors setplay_appliers[] =
{
  { STMoveInverter,   &move_inverter_apply_setplay  },
  { STConstraint,     &stip_traverse_structure_pipe },
  { STAttackAdapter,  &attack_adapter_apply_setplay },
  { STDefenseAdapter, &stip_structure_visitor_noop  },
  { STHelpAdapter,    &help_adapter_apply_setplay   }
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

  {
    slice_structural_type i;
    for (i = 0; i!=nr_slice_structure_types; ++i)
      if (slice_structure_is_subclass(i,slice_structure_pipe))
        stip_structure_traversal_override_by_structure(&st,i,&pipe_apply_setplay);
  }

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
  goal_type const goal = slices[si].u.goal_tester.goal.type;

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
  { STMove,              &move_detect_starter          },
  { STDummyMove,         &move_detect_starter          },
  { STMoveInverter,      &move_inverter_detect_starter },
  { STThreatSolver,      &pipe_detect_starter          },
  { STMaxThreatLength,   &pipe_detect_starter          },
  { STTemporaryHackFork, &pipe_detect_starter          }
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
  TraceFunctionParam("%u",*starter);
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
  STMove,
  STDummyMove,
  STMoveInverter
};

enum
{
  nr_starter_inverters = (sizeof starter_inverters
                          / sizeof starter_inverters[0])
};

/* Set the starting side of the stipulation
 * @param si identifies slice where to start
 * @param starter starting side at the root of the stipulation
 */
void stip_impose_starter(slice_index si, Side starter)
{
  stip_structure_traversal st;
  unsigned int i;
  slice_type type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,starter,"");
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&starter);

  for (type = 0; type!=nr_slice_types; ++type)
    stip_structure_traversal_override_single(&st,
                                             type,
                                             &default_impose_starter);

  for (i = 0; i!=nr_starter_inverters; ++i)
    stip_structure_traversal_override_single(&st,
                                             starter_inverters[i],
                                             &impose_inverted_starter);
  stip_structure_traversal_override_single(&st,
                                           STIntelligentFilter,
                                           &impose_starter_intelligent_filter);
 stip_structure_traversal_override_single(&st,
                                          STGoalImmobileReachedTester,
                                          &impose_starter_immobility_tester);

  stip_traverse_structure(si,&st);

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

  if (root!=no_slice)
    if (st->traversed[root]==slice_not_traversed)
    {
      /* avoid infinite recursion */
      st->traversed[root] = slice_being_traversed;
      stip_structure_visit_slice(root,&st->map.visitors,st);
      st->traversed[root] = slice_traversed;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor structure_children_traversers[] =
{
  &stip_traverse_structure_pipe,              /* STProxy */
  &stip_traverse_structure_end_of_branch,     /* STTemporaryHackFork */
  &stip_traverse_structure_attack_adpater,    /* STAttackAdapter */
  &stip_traverse_structure_defense_adapter,   /* STDefenseAdapter */
  &stip_traverse_structure_ready_for_attack,  /* STReadyForAttack */
  &stip_traverse_structure_ready_for_defense, /* STReadyForDefense */
  &stip_traverse_structure_pipe,              /* STMinLengthOptimiser */
  &stip_traverse_structure_help_adpater,      /* STHelpAdapter */
  &stip_traverse_structure_pipe,              /* STReadyForHelpMove */
  &stip_traverse_structure_setplay_fork,      /* STSetplayFork */
  &stip_traverse_structure_end_of_branch,     /* STEndOfBranch */
  &stip_traverse_structure_end_of_branch,     /* STEndOfBranchForced */
  &stip_traverse_structure_end_of_branch,     /* STEndOfBranchGoal */
  &stip_traverse_structure_end_of_branch,     /* STConstraint */
  &stip_traverse_structure_pipe,              /* STEndOfRoot */
  &stip_traverse_structure_pipe,              /* STEndOfIntro */
  &stip_traverse_structure_pipe,              /* STDeadEnd */
  &stip_traverse_structure_pipe,              /* STMove */
  &stip_traverse_structure_pipe,              /* STDummyMove */
  &stip_traverse_structure_pipe,              /* STReadyForDummyMove */
  &stip_traverse_structure_pipe,              /* STShortSolutionsStart*/
  &stip_traverse_structure_binary,            /* STCheckZigzagJump */
  &stip_traverse_structure_pipe,              /* STCheckZigzagLanding */
  &stip_traverse_structure_goal_reached_tester, /* STGoalReachedTester */
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
  &stip_traverse_structure_end_of_branch,     /* STGoalImmobileReachedTester */
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
  &stip_traverse_structure_pipe,              /* STRefutationsAllocator */
  &stip_traverse_structure_pipe,              /* STTrySolver */
  &stip_traverse_structure_pipe,              /* STRefutationsSolver */
  &stip_traverse_structure_pipe,              /* STPlaySuppressor */
  &stip_traverse_structure_pipe,              /* STContinuationSolver */
  &stip_traverse_structure_check_threat_solver, /* STThreatSolver */
  &stip_traverse_structure_pipe,              /* STThreatEnforcer */
  &stip_traverse_structure_pipe,              /* STThreatStart */
  &stip_traverse_structure_pipe,              /* STThreatCollector */
  &stip_traverse_structure_pipe,              /* STRefutationsCollector */
  &stip_traverse_structure_pipe,              /* STLegalMoveCounter */
  &stip_traverse_structure_pipe,              /* STAnyMoveCounter */
  &stip_traverse_structure_pipe,              /* STCaptureCounter */
  &stip_traverse_structure_pipe,              /* STTestingPrerequisites */
  &stip_traverse_structure_end_of_branch,     /* STDoubleMateFilter */
  &stip_traverse_structure_end_of_branch,     /* STCounterMateFilter */
  &stip_traverse_structure_pipe,              /* STPrerequisiteOptimiser */
  &stip_traverse_structure_pipe,              /* STNoShortVariations */
  &stip_traverse_structure_pipe,              /* STRestartGuard */
  &stip_traverse_structure_pipe,              /* STMaxTimeGuard */
  &stip_traverse_structure_pipe,              /* STMaxSolutionsInitialiser */
  &stip_traverse_structure_pipe,              /* STMaxSolutionsGuard */
  &stip_traverse_structure_end_of_branch,     /* STEndOfBranchGoalImmobile */
  &stip_traverse_structure_pipe,              /* STDeadEndGoal */
  &stip_traverse_structure_pipe,              /* STOrthodoxMatingMoveGenerator */
  &stip_traverse_structure_pipe,              /* STKillerMoveCollector */
  &stip_traverse_structure_pipe,              /* STKillerMoveMoveGenerator */
  &stip_traverse_structure_pipe,              /* STKillerMoveFinalDefenseMove */
  &stip_traverse_structure_pipe,              /* STCountNrOpponentMovesMoveGenerator */
  &stip_traverse_structure_pipe,              /* STEnPassantFilter */
  &stip_traverse_structure_pipe,              /* STCastlingFilter */
  &stip_traverse_structure_pipe,              /* STAttackHashed */
  &stip_traverse_structure_pipe,              /* STHelpHashed */
  &stip_traverse_structure_end_of_branch,     /* STIntelligentFilter */
  &stip_traverse_structure_pipe,              /* STGoalReachableGuardFilter */
  &stip_traverse_structure_pipe,              /* STIntelligentDuplicateAvoider */
  &stip_traverse_structure_pipe,              /* STKeepMatingFilter */
  &stip_traverse_structure_end_of_branch,     /* STMaxFlightsquares */
  &stip_traverse_structure_pipe,              /* STFlightsquaresCounter */
  &stip_traverse_structure_pipe,              /* STDegenerateTree */
  &stip_traverse_structure_pipe,              /* STMaxNrNonTrivial */
  &stip_traverse_structure_pipe,              /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_pipe,              /* STMaxThreatLength */
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
  &stip_traverse_structure_pipe,              /* STPiecesParalysingStalemateSpecial */
  &stip_traverse_structure_pipe,              /* STPiecesKamikazeTargetSquareFilter */
  &stip_traverse_structure_pipe,              /* STImmobilityTester */
  &stip_traverse_structure_pipe,              /* STOhneschachSuspender */
  &stip_traverse_structure_goal_reached_tester, /* STExclusiveChessMatingMoveCounter */
  &stip_traverse_structure_pipe,              /* STExclusiveChessUnsuspender */
  &stip_traverse_structure_pipe,              /* STMaffImmobilityTesterKing */
  &stip_traverse_structure_pipe,              /* STOWUImmobilityTesterKing */
  &stip_traverse_structure_pipe,              /* STSingleMoveGeneratorWithKingCapture */
  &stip_traverse_structure_goal_reached_tester,/* STBrunnerDefenderFinder */
  &stip_traverse_structure_goal_reached_tester,/* STIsardamDefenderFinder */
  &stip_traverse_structure_pipe,              /* STOutputModeSelector */
  &stip_traverse_structure_pipe,              /* STIllegalSelfcheckWriter */
  &stip_traverse_structure_pipe,              /* STEndOfPhaseWriter */
  &stip_traverse_structure_pipe,              /* STEndOfSolutionWriter */
  &stip_traverse_structure_pipe,              /* STMoveWriter */
  &stip_traverse_structure_pipe,              /* STKeyWriter */
  &stip_traverse_structure_pipe,              /* STTryWriter */
  &stip_traverse_structure_pipe,              /* STZugzwangWriter */
  &stip_traverse_structure_pipe,              /* STTrivialEndFilter */
  &stip_traverse_structure_pipe,              /* STRefutingVariationWriter */
  &stip_traverse_structure_pipe,              /* STRefutationWriter */
  &stip_traverse_structure_pipe,              /* STOutputPlaintextTreeCheckWriter */
  &stip_traverse_structure_pipe,              /* STOutputPlaintextTreeDecorationWriter */
  &stip_traverse_structure_pipe,              /* STOutputPlaintextLineLineWriter */
  &stip_traverse_structure_pipe,              /* STOutputPlaintextTreeGoalWriter */
  &stip_traverse_structure_pipe,              /* STOutputPlaintextMoveInversionCounter */
  &stip_traverse_structure_pipe               /* STOutputPlaintextLineEndOfIntroSeriesMarker */
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
    &stip_traverse_moves_end_of_branch,     /* STTemporaryHackFork */
    &stip_traverse_moves_attack_adapter,    /* STAttackAdapter */
    &stip_traverse_moves_defense_adapter,   /* STDefenseAdapter */
    &stip_traverse_moves_ready_for_attack,  /* STReadyForAttack */
    &stip_traverse_moves_ready_for_defense, /* STReadyForDefense */
    &stip_traverse_moves_pipe,              /* STMinLengthOptimiser */
    &stip_traverse_moves_help_adapter,      /* STHelpAdapter */
    &stip_traverse_moves_pipe,              /* STReadyForHelpMove */
    &stip_traverse_moves_setplay_fork,      /* STSetplayFork */
    &stip_traverse_moves_end_of_branch,     /* STEndOfBranch */
    &stip_traverse_moves_end_of_branch,     /* STEndOfBranchForced */
    &stip_traverse_moves_end_of_branch,     /* STEndOfBranchGoal */
    &stip_traverse_moves_setplay_fork,      /* STConstraint */
    &stip_traverse_moves_pipe,              /* STEndOfRoot */
    &stip_traverse_moves_pipe,              /* STEndOfIntro */
    &stip_traverse_moves_dead_end,          /* STDeadEnd */
    &stip_traverse_moves_move,              /* STMove */
    &stip_traverse_moves_move,              /* STDummyMove */
    &stip_traverse_moves_pipe,              /* STReadyForDummyMove */
    &stip_traverse_moves_pipe,              /* STShortSolutionsStart*/
    &stip_traverse_moves_binary,            /* STCheckZigzagJump */
    &stip_traverse_moves_pipe,              /* STCheckZigzagLanding */
    &stip_traverse_moves_setplay_fork,      /* STGoalReachedTester */
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
    &stip_traverse_moves_end_of_branch,     /* STGoalImmobileReachedTester */
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
    &stip_traverse_moves_pipe,              /* STRefutationsAllocator */
    &stip_traverse_moves_pipe,              /* STTrySolver */
    &stip_traverse_moves_pipe,              /* STRefutationsSolver */
    &stip_traverse_moves_pipe,              /* STPlaySuppressor */
    &stip_traverse_moves_pipe,              /* STContinuationSolver */
    &stip_traverse_moves_pipe,              /* STThreatSolver */
    &stip_traverse_moves_pipe,              /* STThreatEnforcer */
    &stip_traverse_moves_pipe,              /* STThreatStart */
    &stip_traverse_moves_pipe,              /* STThreatCollector */
    &stip_traverse_moves_pipe,              /* STRefutationsCollector */
    &stip_traverse_moves_pipe,              /* STLegalMoveCounter */
    &stip_traverse_moves_pipe,              /* STAnyMoveCounter */
    &stip_traverse_moves_pipe,              /* STCaptureCounter */
    &stip_traverse_moves_pipe,              /* STTestingPrerequisites */
    &stip_traverse_moves_end_of_branch,     /* STDoubleMateFilter */
    &stip_traverse_moves_end_of_branch,     /* STCounterMateFilter */
    &stip_traverse_moves_pipe,              /* STPrerequisiteOptimiser */
    &stip_traverse_moves_pipe,              /* STNoShortVariations */
    &stip_traverse_moves_pipe,              /* STRestartGuard */
    &stip_traverse_moves_pipe,              /* STMaxTimeGuard */
    &stip_traverse_moves_pipe,              /* STMaxSolutionsInitialiser */
    &stip_traverse_moves_pipe,              /* STMaxSolutionsGuard */
    &stip_traverse_moves_end_of_branch,     /* STEndOfBranchGoalImmobile */
    &stip_traverse_moves_dead_end,          /* STDeadEndGoal */
    &stip_traverse_moves_pipe,              /* STOrthodoxMatingMoveGenerator */
    &stip_traverse_moves_pipe,              /* STKillerMoveCollector */
    &stip_traverse_moves_pipe,              /* STKillerMoveMoveGenerator */
    &stip_traverse_moves_pipe,              /* STKillerMoveFinalDefenseMove */
    &stip_traverse_moves_pipe,              /* STCountNrOpponentMovesMoveGenerator */
    &stip_traverse_moves_pipe,              /* STEnPassantFilter */
    &stip_traverse_moves_pipe,              /* STCastlingFilter */
    &stip_traverse_moves_pipe,              /* STAttackHashed */
    &stip_traverse_moves_pipe,              /* STHelpHashed */
    &stip_traverse_moves_end_of_branch,     /* STIntelligentFilter */
    &stip_traverse_moves_pipe,              /* STGoalReachableGuardFilter */
    &stip_traverse_moves_pipe,              /* STIntelligentDuplicateAvoider */
    &stip_traverse_moves_pipe,              /* STKeepMatingFilter */
    &stip_traverse_moves_end_of_branch,     /* STMaxFlightsquares */
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
    &stip_traverse_moves_pipe,              /* STPiecesParalysingStalemateSpecial */
    &stip_traverse_moves_pipe,              /* STPiecesKamikazeTargetSquareFilter */
    &stip_traverse_moves_pipe,              /* STImmobilityTester */
    &stip_traverse_moves_pipe,              /* STOhneschachSuspender */
    &stip_traverse_moves_setplay_fork,      /* STExclusiveChessMatingMoveCounter */
    &stip_traverse_moves_pipe,              /* STExclusiveChessUnsuspender */
    &stip_traverse_moves_pipe,              /* STMaffImmobilityTesterKing */
    &stip_traverse_moves_pipe,              /* STOWUImmobilityTesterKing */
    &stip_traverse_moves_pipe,              /* STSingleMoveGeneratorWithKingCapture */
    &stip_traverse_moves_setplay_fork,      /* STBrunnerDefenderFinder */
    &stip_traverse_moves_setplay_fork,      /* STIsardamDefenderFinder */
    &stip_traverse_moves_pipe,              /* STOutputModeSelector */
    &stip_traverse_moves_pipe,              /* STIllegalSelfcheckWriter */
    &stip_traverse_moves_pipe,              /* STEndOfPhaseWriter */
    &stip_traverse_moves_pipe,              /* STEndOfSolutionWriter */
    &stip_traverse_moves_pipe,              /* STMoveWriter */
    &stip_traverse_moves_pipe,              /* STKeyWriter */
    &stip_traverse_moves_pipe,              /* STTryWriter */
    &stip_traverse_moves_pipe,              /* STZugzwangWriter */
    &stip_traverse_moves_pipe,              /* STTrivialEndFilter */
    &stip_traverse_moves_pipe,              /* STRefutingVariationWriter */
    &stip_traverse_moves_pipe,              /* STRefutationWriter */
    &stip_traverse_moves_pipe,              /* STOutputPlaintextTreeCheckWriter */
    &stip_traverse_moves_pipe,              /* STOutputPlaintextTreeDecorationWriter */
    &stip_traverse_moves_pipe,              /* STOutputPlaintextLineLineWriter */
    &stip_traverse_moves_pipe,              /* STOutputPlaintextTreeGoalWriter */
    &stip_traverse_moves_pipe,              /* STOutputPlaintextMoveInversionCounter */
    &stip_traverse_moves_pipe               /* STOutputPlaintextLineEndOfIntroSeriesMarker */
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
