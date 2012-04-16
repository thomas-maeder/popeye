#include "pystip.h"
#include "pydata.h"
#include "pybrafrk.h"
#include "pyselfcg.h"
#include "pypipe.h"
#include "stipulation/move_inverter.h"
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
#include "options/maxthreatlength.h"
#include "options/maxsolutions/initialiser.h"
#include "optimisations/hash.h"
#include "optimisations/keepmating.h"
#include "optimisations/goals/enpassant/filter.h"
#include "optimisations/intelligent/mate/filter.h"
#include "optimisations/intelligent/stalemate/filter.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

#include "stipulation/slice_type.h"
#define ENUMERATION_MAKESTRINGS
#include "utilities/enumeration.h"


#define ENUMERATION_TYPENAME branch_level
#define ENUMERATORS \
  ENUMERATOR(toplevel_branch),                  \
    ENUMERATOR(nested_branch)

#define ENUMERATION_MAKESTRINGS

#include "utilities/enumeration.h"

#define ENUMERATION_TYPENAME has_solution_type
#define ENUMERATORS                                              \
    ENUMERATOR(has_solution_type_0),                             \
    ENUMERATOR(opponent_self_check),                             \
    ENUMERATOR(has_solution_type_2),                             \
    ENUMERATOR(has_solution),                                    \
    ENUMERATOR(has_solution_type_4),                             \
    ENUMERATOR(has_no_solution),                                 \
    ASSIGNED_ENUMERATOR(length_unspecified=slack_length)

#define ENUMERATION_MAKESTRINGS

#include "utilities/enumeration.h"


Slice slices[max_nr_slices];


/* Keep track of allocated slice indices
 */
static boolean is_slice_index_free[max_nr_slices];


#define ENUMERATION_TYPENAME slice_structural_type
#define ENUMERATORS                             \
    ENUMERATOR(slice_structure_pipe),                            \
    ENUMERATOR(slice_structure_leaf),                            \
    ENUMERATOR(slice_structure_binary),                          \
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
    init_one_type(binary);
    init_one_type(branch);
    init_one_type(fork);
#undef init_one_type
  }
}

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

#define ENUMERATION_TYPENAME slice_functional_type
#define ENUMERATORS                             \
    ENUMERATOR(slice_function_unspecified),                        \
    ENUMERATOR(slice_function_proxy),                              \
    ENUMERATOR(slice_function_move_generator),                     \
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
  init_one_type(testing_pipe);
  init_one_type(conditional_pipe);
  init_one_type(writer);
#undef init_one_type
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

  stip_traverse_structure_children_pipe(si,st);

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

  stip_traverse_structure_children_pipe(fork,st);

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

static void hack_fork_apply_setplay(slice_index si, stip_structure_traversal *st)
{
  spin_off_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);
  TraceValue("%u\n",state->spun_off[slices[si].u.fork.next]);

  state->spun_off[si] = state->spun_off[slices[si].u.fork.next];
  TraceValue("%u\n",state->spun_off[si]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


static structure_traversers_visitors setplay_appliers[] =
{
  { STMoveInverter,      &pipe_spin_off_copy           },
  { STConstraintSolver,  &stip_traverse_structure_children_pipe },
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
    branch_insert_slices(si,&set_fork,1);
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

static structure_traversers_visitors const special_children_traversers[] =
{
   { STAttackAdapter,   &stip_traverse_structure_children_attack_adpater    },
   { STDefenseAdapter,  &stip_traverse_structure_children_defense_adapter   },
   { STReadyForAttack,  &stip_traverse_structure_children_ready_for_attack  },
   { STReadyForDefense, &stip_traverse_structure_children_ready_for_defense },
   { STHelpAdapter,     &stip_traverse_structure_children_help_adpater      },
   { STSetplayFork,     &stip_traverse_structure_children_setplay_fork      },
   { STMovePlayed,      &stip_traverse_structure_children_move_played       },
   { STThreatSolver,    &stip_traverse_structure_children_binary            }
};

enum { nr_special_children_traversers = sizeof special_children_traversers
                                        / sizeof special_children_traversers[0] };

static stip_structure_visitor structure_children_traversers[nr_slice_types];

static stip_structure_visitor get_default_children_structure_visitor(slice_type type)
{
  stip_structure_visitor result;

  switch (highest_structural_type[type])
  {
    case slice_structure_pipe:
    case slice_structure_branch:
      result = &stip_traverse_structure_children_pipe;
      break;

    case slice_structure_leaf:
      result = &stip_structure_visitor_noop;
      break;

    case slice_structure_binary:
      result = &stip_traverse_structure_children_binary;
      break;

    case slice_structure_fork:
      if (functional_type[type]==slice_function_testing_pipe)
        result = &stip_traverse_structure_children_testing_pipe;
      else
        result = &stip_traverse_structure_children_fork;
      break;

    default:
      assert(0);
      break;
  }

  return result;
}

static void init_structure_children_visitors(void)
{
  {
    slice_type i;
    for (i = 0; i!=nr_slice_types; ++i)
      structure_children_traversers[i] = get_default_children_structure_visitor(i);
  }

  {
    unsigned int i;
    for (i = 0; i!=nr_special_children_traversers; ++i)
      structure_children_traversers[special_children_traversers[i].type] = special_children_traversers[i].visitor;
  }
}

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

static moves_visitor_map_type moves_children_traversers = { { 0 } };

static stip_moves_visitor get_default_children_moves_visitor(slice_type type)
{
  stip_moves_visitor result;

  switch (highest_structural_type[type])
  {
    case slice_structure_pipe:
    case slice_structure_branch:
      result = &stip_traverse_moves_pipe;
      break;

    case slice_structure_leaf:
      result = &stip_traverse_moves_noop;
      break;

    case slice_structure_binary:
      result = &stip_traverse_moves_binary;
      break;

    case slice_structure_fork:
      if (functional_type[type]==slice_function_conditional_pipe)
        result = &stip_traverse_moves_conditional_pipe;
      else
        result = &stip_traverse_moves_end_of_branch;
      break;

    default:
      assert(0);
      break;
  }

  return result;
}

static moves_traversers_visitors const special_moves_children_traversers[] =
{
   { STAttackAdapter,    &stip_traverse_moves_attack_adapter    },
   { STDefenseAdapter,   &stip_traverse_moves_defense_adapter   },
   { STReadyForAttack,   &stip_traverse_moves_ready_for_attack  },
   { STReadyForDefense,  &stip_traverse_moves_ready_for_defense },
   { STHelpAdapter,      &stip_traverse_moves_help_adapter      },
   { STSetplayFork,      &stip_traverse_moves_setplay_fork      },
   { STConstraintSolver, &stip_traverse_moves_setplay_fork      },
   { STMovePlayed,       &stip_traverse_moves_move_played       },
   { STHelpMovePlayed,   &stip_traverse_moves_move_played       },
   { STForkOnRemaining,  &stip_traverse_moves_fork_on_remaining },
   { STDeadEnd,          &stip_traverse_moves_dead_end          },
   { STDeadEndGoal,      &stip_traverse_moves_dead_end          }
};

enum { nr_special_moves_children_traversers = sizeof special_moves_children_traversers
                                              / sizeof special_moves_children_traversers[0] };

/* it's not so clear whether it is a good idea to traverse moves once slices
 * of the following types have been inserted ... */
static slice_type const dubiously_traversed_slice_types[] =
{
    STContinuationSolver,
    STThreatSolver,
    STThreatEnforcer,
    STNoShortVariations,
    STMaxNrNonTrivial,
    STMaxThreatLength,
    STTrivialEndFilter
};

enum { nr_dubiously_traversed_slice_types = sizeof dubiously_traversed_slice_types
                                            / sizeof dubiously_traversed_slice_types[0] };

static void init_moves_children_visitors(void)
{
  {
    slice_type i;
    for (i = 0; i!=nr_slice_types; ++i)
      moves_children_traversers.visitors[i] = get_default_children_moves_visitor(i);
  }

  {
    unsigned int i;
    for (i = 0; i!=nr_special_moves_children_traversers; ++i)
      moves_children_traversers.visitors[special_moves_children_traversers[i].type] = special_moves_children_traversers[i].visitor;
  }

  {
    unsigned int i;
    for (i = 0; i!=nr_dubiously_traversed_slice_types; ++i)
      moves_children_traversers.visitors[dubiously_traversed_slice_types[i]] = &stip_traverse_moves_pipe;
  }
}

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

  for (unsigned int i = 0; i!=max_nr_slices; ++i)
    st->remaining_watermark[i] = 0;

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

  if (st->remaining_watermark[root]<=st->remaining)
  {
    stip_moves_visitor const operation = st->map.visitors[slices[root].type];
    assert(operation!=0);
    (*operation)(root,st);
    st->remaining_watermark[root] = st->remaining+1;
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

/* Initialise slice properties at start of program */
void initialise_slice_properties(void)
{
  init_highest_structural_type();
  init_functional_type();
  init_structure_children_visitors();
  init_moves_children_visitors();
  init_slice_allocator();
}
