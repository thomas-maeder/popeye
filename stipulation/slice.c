#include "stipulation/slice.h"
#include "debugging/trace.h"

#include <assert.h>


/* Keep track of allocated slice indices
 */
static boolean is_slice_index_free[max_nr_slices];


#define ENUMERATION_TYPENAME slice_structural_type
#define ENUMERATORS                             \
    ENUMERATOR(slice_structure_pipe),                            \
    ENUMERATOR(slice_structure_leaf),                            \
    ENUMERATOR(slice_structure_branch),                          \
    ENUMERATOR(slice_structure_fork),                            \
    ENUMERATOR(nr_slice_structure_types)

#define ENUMERATION_MAKESTRINGS

#include "utilities/enumeration.h"

static slice_structural_type highest_structural_type[nr_slice_types];

static slice_type const leaf_slice_types[] =
{
    STFalse,
    STTrue
};

static slice_type const binary_slice_types[] =
{
    STEndOfBranchGoal,
    STEndOfBranchGoalImmobile,
    STAvoidUnsolvable,
    STCheckZigzagJump,
    STAnd,
    STOr,
    STForkOnRemaining,
    STRefutationsSolver
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
    STReadyForDummyMove,
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
    STEndOfBranchGoal,
    STEndOfBranchGoalImmobile,
    STAvoidUnsolvable,
    STCheckZigzagJump,
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
    STGoalImmobileReachedTester,
    STCastlingIntermediateMoveLegalityTester,
    STContinuationSolver,
    STThreatSolver,
    STThreatEnforcer,
    STDoubleMateFilter,
    STCounterMateFilter,
    STNoShortVariations,
    STIntelligentMateFilter,
    STIntelligentStalemateFilter,
    STMaxFlightsquares,
    STMaxNrNonTrivial,
    STMaxThreatLength,
    STOpponentMovesCounterFork,
    STExclusiveChessMatingMoveCounter,
    STBrunnerDefenderFinder,
    STIsardamDefenderFinder,
    STCageCirceNonCapturingMoveFinder,
    STMaximummerCandidateMoveTester,
    STTrivialEndFilter
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

#define ENUMERATION_TYPENAME slice_functional_type
#define ENUMERATORS                             \
    ENUMERATOR(slice_function_unspecified),                        \
    ENUMERATOR(slice_function_proxy),                              \
    ENUMERATOR(slice_function_move_generator),                     \
    ENUMERATOR(slice_function_binary),                             \
    ENUMERATOR(slice_function_testing_pipe),                       \
    ENUMERATOR(slice_function_conditional_pipe),                   \
    ENUMERATOR(slice_function_writer),                             \
    ENUMERATOR(nr_slice_functional_types)

#define ENUMERATION_MAKESTRINGS

#include "utilities/enumeration.h"

static slice_functional_type functional_type[nr_slice_types];

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
    STReadyForDummyMove,
    STShortSolutionsStart,
    STCheckZigzagLanding,
    STGoalMateReachedTester,
    STGoalStalemateReachedTester,
    STGoalDoubleStalemateReachedTester,
    STGoalAutoStalemateReachedTester,
    STGeneratingMoves,
    STEndOfRefutationSolvingBranch,
    STSolvingContinuation,
    STThreatStart,
    STThreatEnd,
    STTestingPrerequisites,
    STMaxThreatLengthStart,
    STOutputModeSelector
};

static slice_type const move_generator_slice_types[] =
{
    STMoveGenerator,
    STKingMoveGenerator,
    STNonKingMoveGenerator,
    STCastlingIntermediateMoveGenerator,
    STOrthodoxMatingMoveGenerator,
    STKillerMoveFinalDefenseMove,
    STSingleMoveGeneratorWithKingCapture,
    STSingleMoveGenerator
};

static slice_type const testing_pipe_slice_types[] =
{
    STContinuationSolver,
    STThreatEnforcer,
    STNoShortVariations,
    STMaxNrNonTrivial,
    STMaxThreatLength,
    STTrivialEndFilter
};

static slice_type const conditional_pipe_slice_types[] =
{
    STTemporaryHackFork,
    STEndOfBranchTester,
    STEndOfBranchGoalTester,
    STConstraintTester,
    STGoalConstraintTester,
    STGoalReachedTester,
    STGoalImmobileReachedTester,
    STCastlingIntermediateMoveLegalityTester,
    STDoubleMateFilter,
    STCounterMateFilter,
    STIntelligentMateFilter,
    STIntelligentStalemateFilter,
    STMaxFlightsquares,
    STOpponentMovesCounterFork,
    STExclusiveChessMatingMoveCounter,
    STBrunnerDefenderFinder,
    STIsardamDefenderFinder,
    STCageCirceNonCapturingMoveFinder,
    STMaximummerCandidateMoveTester
};

static slice_type const writer_slice_types[] =
{
    STIllegalSelfcheckWriter,
    STEndOfPhaseWriter,
    STEndOfSolutionWriter,
    STThreatWriter,
    STMoveWriter,
    STKeyWriter,
    STTryWriter,
    STZugzwangWriter,
    STRefutingVariationWriter,
    STRefutationsIntroWriter,
    STRefutationWriter,
    STOutputPlaintextTreeCheckWriter,
    STOutputPlaintextLineLineWriter,
    STOutputPlaintextTreeGoalWriter
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
  init_one_type(binary);
  init_one_type(testing_pipe);
  init_one_type(conditional_pipe);
  init_one_type(writer);
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
      TraceValue("leaked:%u",i);
    assert(is_slice_index_free[i]);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialize the slice allocation machinery. To be called once at
 * program start
 */
static void init_slice_allocator(void)
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
 * @return a so far unused slice index
 */
slice_index alloc_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (result = 0; result!=max_nr_slices; ++result)
    if (is_slice_index_free[result])
      break;

  assert(result<max_nr_slices);

  is_slice_index_free[result] = false;

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

  assert(!is_slice_index_free[si]);
  is_slice_index_free[si] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise slice properties at start of program */
void initialise_slice_properties(void)
{
  init_highest_structural_type();
  init_functional_type();
  init_slice_allocator();
}
