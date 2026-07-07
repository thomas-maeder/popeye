#include "stipulation/slice_insertion.h"
#include "stipulation/if_then_else.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/move.h"
#include "stipulation/pipe.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

#include <stdio.h>  /* included for fprintf(FILE *, char const *, ...) */
#include <stdlib.h> /* included for exit(int) */

void deallocate_slice_insertion_prototypes(slice_index const prototypes[],
                                           unsigned int nr_prototypes)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  for (i = 0; i!=nr_prototypes; ++i)
    dealloc_slice(prototypes[i]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static unsigned int get_slice_rank_recursive(slice_type type,
                                             branch_slice_insertion_state_type const *state)
{
  unsigned int result = no_slice_rank;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type);
  TraceFunctionParamListEnd();

  TraceValue("%u",state->base_rank);
  TraceValue("%u",state->nr_slice_rank_order_elmts);
  TraceEOL();

  for (i = 0; i!=state->nr_slice_rank_order_elmts; ++i)
    if (state->slice_rank_order[(i+state->base_rank)%state->nr_slice_rank_order_elmts]==type)
    {
      result = i+state->base_rank;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int get_slice_rank_nonrecursive(slice_type type,
                                                branch_slice_insertion_state_type const *state)
{
  unsigned int result = no_slice_rank;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type);
  TraceFunctionParamListEnd();

  TraceValue("%u",state->base_rank);
  TraceValue("%u",state->nr_slice_rank_order_elmts);
  TraceEOL();

  for (i = state->base_rank; i!=state->nr_slice_rank_order_elmts; ++i)
    if (state->slice_rank_order[i]==type)
    {
      result = i;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine the rank of a slice type
 * @param type slice type
 * @return rank of type; no_slice_rank if the rank can't be determined
 */
unsigned int get_slice_rank(slice_type type,
                            branch_slice_insertion_state_type const *state)
{
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(slice_type,type);
  TraceFunctionParamListEnd();

  if (state->type==branch_slice_rank_order_recursive)
    result = get_slice_rank_recursive(type,state);
  else
    result = get_slice_rank_nonrecursive(type,state);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void start_nested_insertion_traversal(slice_index si,
                                             branch_slice_insertion_state_type *state,
                                             stip_structure_traversal *outer)
{
  stip_structure_traversal st_nested;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init_nested(&st_nested,outer,state);
  st_nested.map = outer->map;
  stip_traverse_structure_children_pipe(si,&st_nested);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void next_insertion(slice_index si,
                           unsigned int prototype_rank,
                           stip_structure_traversal *st)
{
  branch_slice_insertion_state_type const * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",prototype_rank);
  TraceFunctionParamListEnd();

  if (state->nr_prototypes>1)
  {
    branch_slice_insertion_state_type nested_state =
    {
        state->prototypes+1, state->nr_prototypes-1,
        state->slice_rank_order, state->nr_slice_rank_order_elmts, state->nr_exit_slice_types,
        state->type,
        prototype_rank+1,
        si,
        state->parent
    };
    start_nested_insertion_traversal(si,&nested_state,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean insert_before(slice_index si,
                             unsigned int rank,
                             stip_structure_traversal *st)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",rank);
  TraceFunctionParamListEnd();

  assert(rank!=no_slice_rank);

  {
    branch_slice_insertion_state_type * const state = st->param;
    slice_index const prototype = state->prototypes[0];
    slice_type const prototype_type = SLICE_TYPE(prototype);
    unsigned int const prototype_rank = get_slice_rank(prototype_type,state);
    if (SLICE_TYPE(si)==SLICE_TYPE(state->prototypes[0]))
    {
      next_insertion(si,prototype_rank,st);
      result = true;
    }
    else if (rank>prototype_rank)
    {
      slice_index const copy = copy_slice(prototype);
      if (copy==no_slice)
      {
        fprintf(stderr, "\nOUT OF SPACE: Unable to copy slice in %s in %s -- aborting.\n", __func__, __FILE__);
        exit(2); /* TODO: Do we have to exit here? */
      }
      pipe_append(state->prev,copy);
      next_insertion(copy,prototype_rank,st);
      result = true;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try insertion before a slice
 * @param st structure representing the insertion traversal
 */
boolean slice_insertion_insert_before(slice_index si,
                                      stip_structure_traversal *st)
{
  branch_slice_insertion_state_type const * const state = st->param;
  unsigned int const rank = get_slice_rank(SLICE_TYPE(si),state);
  return insert_before(si,rank,st);
}

static void insert_visit_leaf(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    branch_slice_insertion_state_type * const state = st->param;
    unsigned int const rank = get_slice_rank(SLICE_TYPE(si),state);
    if (rank!=no_slice_rank)
      insert_before(si,rank,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_beyond(slice_index si, stip_structure_traversal *st)
{
  branch_slice_insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (SLICE_NEXT1(si)==no_slice)
  {
    slice_index curr = si;
    unsigned int i;
    for (i = 0; i!=state->nr_prototypes; ++i)
    {
      slice_index const copy = copy_slice(state->prototypes[i]);
      if (copy==no_slice)
      {
        fprintf(stderr, "\nOUT OF SPACE: Unable to copy slice in %s in %s -- aborting.\n", __func__, __FILE__);
        exit(2); /* TODO: Do we have to exit here? */
      }
      pipe_link(curr,copy);
      curr = copy;
    }
  }
  else
  {
    state->prev = si;
    stip_traverse_structure_children_pipe(si,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Visist a pipe slice during an insertion traversal
 * @param si identifies the pipe slice
 * @param st holds the current state of the insertion traversal
 * @note this function is typically called automatically by insertion traversal;
 *       user code make take its address to override behviour of an insertion
 *       traversal at a specific slice type after having initialised the
 *       traversal using slice_insertion_init_traversal()
 */
void slice_insertion_visit_pipe(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    branch_slice_insertion_state_type * const state = st->param;
    unsigned int const rank = get_slice_rank(SLICE_TYPE(si),state);
    if (rank==no_slice_rank)
      insert_beyond(si,st);
    else if (insert_before(si,rank,st))
    {
      ; /* nothing - work is done*/
    }
    else
    {
      state->base_rank = rank;
      insert_beyond(si,st);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Visit a binary slice during an insertion traversal; don't insert into next1,
 * but continue into next2 if the slice types to be inserted have higher rank
 * than the binary slice.
 * @param si identifies the pipe slice
 * @param st holds the current state of the insertion traversal
 * @note this function is typically called automatically by insertion traversal;
 *       user code make take its address to override behviour of an insertion
 *       traversal at a specific slice type after having initialised the
 *       traversal using slice_insertion_init_traversal()
 */
void slice_insertion_visit_binary_skip_next1(slice_index si,
                                             stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    branch_slice_insertion_state_type * const state = st->param;
    unsigned int const rank = get_slice_rank(SLICE_TYPE(si),state);
    if (rank==no_slice_rank)
    {
      if (SLICE_NEXT2(si)!=no_slice)
      {
        assert(SLICE_TYPE(SLICE_NEXT2(si))==STProxy);
        insert_beyond(SLICE_NEXT2(si),st);
      }
    }
    else if (insert_before(si,rank,st))
      ; /* nothing - work is done*/
    else
    {
      state->base_rank = rank+1;
      if (SLICE_NEXT2(si)!=no_slice)
      {
        assert(SLICE_TYPE(SLICE_NEXT2(si))==STProxy);
        insert_beyond(SLICE_NEXT2(si),st);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_visit_binary_operands(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    branch_slice_insertion_state_type * const state = st->param;
    branch_slice_insertion_state_type const save_state = *state;

    if (SLICE_NEXT1(si)!=no_slice)
      insert_beyond(si,st);

    *state = save_state;

    if (SLICE_NEXT2(si)!=no_slice)
    {
      assert(SLICE_TYPE(SLICE_NEXT2(si))==STProxy);
      insert_beyond(SLICE_NEXT2(si),st);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_visit_binary(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    branch_slice_insertion_state_type * const state = st->param;
    unsigned int const rank = get_slice_rank(SLICE_TYPE(si),state);
    if (rank==no_slice_rank)
      insert_visit_binary_operands(si,st);
    else if (insert_before(si,rank,st))
      ; /* nothing - work is done*/
    else
    {
      state->base_rank = rank+1;
      insert_visit_binary_operands(si,st);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_visit_setplay_fork(slice_index si,
                                      stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    branch_slice_insertion_state_type * const state = st->param;
    unsigned int const rank = get_slice_rank(SLICE_TYPE(si),state);
    if (insert_before(si,rank,st))
      ; /* nothing - work is done*/
    else
    {
      state->base_rank = rank;
      insert_beyond(si,st);

      state->base_rank = rank;
      state->prev = si;
      start_nested_insertion_traversal(SLICE_NEXT2(si),state,st);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_visit_battle_adapter(slice_index si,
                                        stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    branch_slice_insertion_state_type * const state = st->param;
    unsigned int const rank = get_slice_rank(SLICE_TYPE(si),state);
    if (rank==no_slice_rank || !insert_before(si,rank,st))
      battle_branch_insert_slices_nested(si,
                                         state->prototypes,
                                         state->nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_visit_help_adapter(slice_index si,
                                      stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    branch_slice_insertion_state_type * const state = st->param;
    unsigned int const rank = get_slice_rank(SLICE_TYPE(si),state);
    if (rank==no_slice_rank || !insert_before(si,rank,st))
      help_branch_insert_slices_nested(si,
                                       state->prototypes,
                                       state->nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_return_from_factored_order(slice_index si, stip_structure_traversal *st)
{
  branch_slice_insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(state->parent!=0);

  {
    unsigned int const rank = get_slice_rank(SLICE_TYPE(si),state);
    slice_index const prototype = state->prototypes[0];
    slice_type const prototype_type = SLICE_TYPE(prototype);
    unsigned int const prototype_rank = get_slice_rank(prototype_type,state);

    if (rank>prototype_rank)
    {
      slice_index const copy = copy_slice(prototype);
      if (copy==no_slice)
      {
        fprintf(stderr, "\nOUT OF SPACE: Unable to copy slice in %s in %s -- aborting.\n", __func__, __FILE__);
        exit(2); /* TODO: Do we have to exit here? */
      }
      pipe_append(state->prev,copy);
      next_insertion(copy,prototype_rank,st);
    }
    else
    {
      branch_slice_insertion_state_type * const state_parent = state->parent->param;
      state_parent->prototypes = state->prototypes;
      state_parent->nr_prototypes = state->nr_prototypes;
      state_parent->prev = si;
      stip_traverse_structure_children_pipe(si,state->parent);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void end_insertion(slice_index si, stip_structure_traversal *st)
{
  branch_slice_insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(state->parent==0);

  {
    unsigned int const rank = get_slice_rank(SLICE_TYPE(si),state);
    slice_index const prototype = state->prototypes[0];
    slice_type const prototype_type = SLICE_TYPE(prototype);
    unsigned int const prototype_rank = get_slice_rank(prototype_type,state);

    if (rank>prototype_rank)
    {
      slice_index const copy = copy_slice(prototype);
      if (copy==no_slice)
      {
        fprintf(stderr, "\nOUT OF SPACE: Unable to copy slice in %s in %s -- aborting.\n", __func__, __FILE__);
        exit(2); /* TODO: Do we have to exit here? */
      }
      pipe_append(state->prev,copy);
      next_insertion(copy,prototype_rank,st);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void init_slice_insertion_traversal_common(stip_structure_traversal *st,
                                                  branch_slice_insertion_state_type *state,
                                                  stip_traversal_context_type context)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(st,state);
  st->context = context;
  stip_structure_traversal_override_by_structure(st,
                                                 slice_structure_leaf,
                                                 &insert_visit_leaf);
  stip_structure_traversal_override_by_structure(st,
                                                 slice_structure_pipe,
                                                 &slice_insertion_visit_pipe);
  stip_structure_traversal_override_by_structure(st,
                                                 slice_structure_branch,
                                                 &slice_insertion_visit_pipe);
  stip_structure_traversal_override_by_structure(st,
                                                 slice_structure_fork,
                                                 &slice_insertion_visit_pipe);
  stip_structure_traversal_override_by_contextual(st,
                                                  slice_contextual_binary,
                                                  &insert_visit_binary);
  stip_structure_traversal_override_single(st,STProxy,&insert_beyond);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Prepare a structure traversal object for slice insertion according to a
 * factored out slice type order. After slice_insertion_prepare_factored_order()
 * has returned:
 * - further customise the traversal object according to the respective neeeds
 * - invoke stip_traverse_structure_children_pipe(si,st_nested)
 * @param si identifies starting point of insertion
 * @param st insertion traversal where we come from and will return to
 * @param st_nested traversal object to be prepared
 * @param state_nested to hold state of *st_nested; *state_nested must be
 *                     defined in the same scope as *st_nested
 * @param order factored slice type order
 * @param nr_order number of elements of order
 * @param end_of_factored_order slice type where to return to insertion defined
 *                              by st
 */
void slice_insertion_prepare_factored_order(slice_index si,
                                            stip_structure_traversal *st,
                                            stip_structure_traversal *st_nested,
                                            branch_slice_insertion_state_type *state_nested,
                                            slice_index const order[],
                                            unsigned int nr_order,
                                            unsigned int nr_exit_slice_types)
{
  branch_slice_insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *state_nested = *state;

  state_nested->parent = st;
  state_nested->slice_rank_order = order;
  state_nested->nr_slice_rank_order_elmts = nr_order;
  state_nested->nr_exit_slice_types = nr_exit_slice_types;
  state_nested->type = branch_slice_rank_order_nonrecursive;
  state_nested->base_rank = 0;
  state_nested->prev = si;

  state_nested->base_rank = get_slice_rank(SLICE_TYPE(si),state_nested);
  assert(state_nested->base_rank!=no_slice_rank);

  init_slice_insertion_traversal_common(st_nested,state_nested,st->context);

  {
    unsigned int i;
    for (i = nr_order-nr_exit_slice_types; i!=nr_order; ++i)
      stip_structure_traversal_override_single(st_nested,
                                               order[i],
                                               &insert_return_from_factored_order);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise a structure traversal for inserting slices into a branch
 * @param st address of structure representing the traversal
 * @param state address of structure representing the insertion
 * @param context initial context of the insertion traversal; typically the
 *                current context of a surrounding traversal that has arrived
 *                at the slice where the insertion is to start
 */
void slice_insertion_init_traversal(stip_structure_traversal *st,
                                    branch_slice_insertion_state_type *state,
                                    stip_traversal_context_type context)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  init_slice_insertion_traversal_common(st,state,context);

  {
    unsigned int i;
    for (i = state->nr_slice_rank_order_elmts-state->nr_exit_slice_types;
         i!=state->nr_slice_rank_order_elmts;
         ++i)
      stip_structure_traversal_override_single(st,
                                               state->slice_rank_order[i],
                                               &end_insertion);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Order in which the slice types at the root appear
 */
static slice_index const slice_rank_order[] =
{
  STProxy,
  /* slices from here on are created once by main() (mostly) */
  STCommandLineOptionsParser,
  STInputPlainTextOpener,
  STPlatformInitialiser,
  STOutputLaTeXCloser,
  STHashTableDimensioner,
  STTimerStarter,
  STCheckDirInitialiser,
  STOutputPlainTextWriter,
  STInputPlainTextUserLanguageDetector,
  STInputPlainTextProblemsIterator,
  STConditionsResetter,
  STOptionsResetter,
  /* enter problem stage */
  STInputPlainTextProblemHandler,
  STTimerStarter,
  STOutputPlaintextProblemWriter,
  STInputPlainTextInitialTwinReader,
  STOutputLaTeXProblemWriter,
  STProblemSolvingIncomplete,
  /* slices from here on are created for each problem
   * mostly by STInputPlainTextProblemHandler */
  STInputPlainTextTwinsHandler,
  /* enter twin stage */
  STOutputLaTeXTwinningWriterBuilder,
  STOutputPlaintextTwinIntroWriterBuilder,
  STTwinIdAdjuster,
  STQuodlibetStipulationModifier,
  STGoalIsEndStipulationModifier,
  STWhiteToPlayStipulationModifier,
  STPostKeyPlayStipulationModifier,
  STStipulationStarterDetector,
  STOutputPlainTextOptionNoboard,
  STDuplexSolver,
  STHalfDuplexSolver,
  /* enter phase stage */
  STPhaseSolvingIncomplete,
  STStipulationCopier,
  STProofSolverBuilder,
  STAToBSolverBuilder,
  STEndOfStipulationSpecific,
  STZeroPositionInitialiser,
  STOutputPlainTextInstrumentSolversBuilder,
  STOutputLaTeXInstrumentSolversBuilder,
  STSolvingMachineryIntroBuilder,
  STMaxTimeProblemInstrumenter,
  STHeartBeatProblemInstrumenter,
  STIntelligentSolutionsPerTargetPosProblemInstrumenter,
  STMaxSolutionsProblemInstrumenter,
  STStopOnShortSolutionsProblemInstrumenter,
  STOutputLaTeXProblemIntroWriter,
  STOutputLaTeXTwinningWriter,
  STStartOfCurrentTwin,
  /* slices from here on are created for each phase mostly by
   * STSolvingMachineryIntroBuilder */
  STCreateBuilderSetupPly,
  STPieceWalkCacheInitialiser,
  STPiecesCounter,
  STRoyalsLocator,
  STPiecesFlagsInitialiser,
  STProofgameVerifyUniqueGoal,
  STProofgameStartPositionInitialiser,
  STProofSolver,
  STOutputPlainTextMetaWriter,
  STOutputPlainTextBoardWriter,
  STOutputPlainTextPieceCountsWriter,
  STOutputPlainTextRoyalPiecePositionsWriter,
  STOutputPlainTextNonRoyalAttributesWriter,
  STOutputPlainTextAToBIntraWriter,
  STOutputPlainTextStartOfTargetWriter,
  STOutputPlainTextBoardWriter,
  STOutputPlainTextStipulationWriter,
  STOutputPlainTextPieceCountsWriter,
  STOutputPlainTextRoyalPiecePositionsWriter,
  STOutputPlainTextNonRoyalAttributesWriter,
  STOutputPlainTextConditionsWriter,
  STOutputPlainTextMutuallyExclusiveCastlingsWriter,
  STOutputPlainTextDuplexWriter,
  STOutputPlainTextHalfDuplexWriter,
  STOutputPlainTextQuodlibetWriter,
  STOutputPlainTextGridWriter,
  STOutputPlainTextEndOfPositionWriters,
  STOutputPlaintextTwinIntroWriter,
  STZeropositionSolvingStopper,
  STPiecesCounter,
  STRoyalsLocator, /* above for target position, now for starting position */
  STPiecesFlagsInitialiser,
#if defined(DOMEASURE)
  STCountersWriter,
#endif
  STInputVerification,
  STMoveEffectsJournalReset,
  STSolversBuilder1,
  STOutputPlainTextInstrumentSolvers,
  STOutputLaTeXInstrumentSolvers,
  STSolversBuilder2,
  STTotalInvisibleInstrumenter,
  STHashOpener,
  STMaxSolutionsSolvingInstrumenter,
  STStopOnShortSolutionsSolvingInstrumenter,
  STIntelligentSolutionsPerTargetPosSolvingInstrumenter,
  STSlackLengthAdjuster,
  STMaxTimeSetter,
  STHeartBeatSetter,
  STProxyResolver,
  STTemporaryHackFork,
  STStrictSATInitialiser,
  STHopperAttributeSpecificPromotionInitialiser,
  STSetplayFork,
  STMoveInverter,
  STTotalInvisibleMoveSequenceTester,
  STOutputPlaintextMoveInversionCounter,
  STMoveInverterSetPlay,
  STOutputPlaintextMoveInversionCounterSetPlay,
  STRetroStartRetractionPly,
  STRetroRetractLastCapture,
  STRetroUndoLastPawnMultistep,
  STRetroStartRetroMovePly,
  STRetroPlayNullMove,
  STRetroRedoLastCapture,
  STRetroRedoLastPawnMultistep,
  STTotalInvisibleInvisiblesAllocator,
  STIllegalSelfcheckWriter,
  STSelfCheckGuard,
  STAliceSelfCheckGuard,
  STTotalInvisibleUninterceptableSelfCheckGuard,
  STMaxSolutionsInitialiser,
  STStopOnShortSolutionsWasShortSolutionFound,
  STMagicViewsInitialiser,
  STMaffImmobilityTesterKing,
  STImmobilityTester,
  STRecursionStopper,
  STAnd,
  STOr,
  STNot,
  STOutputModeSelector,
  STOutputPlainTextEndOfPhaseWriter,
  STAttackAdapter,
  STDefenseAdapter,
  STHelpAdapter
};

enum
{
  nr_slice_rank_order_elmts = sizeof slice_rank_order / sizeof slice_rank_order[0]
};

static structure_traversers_visitor const insertion_visitors[] =
{
  { STSetplayFork,    &insert_visit_setplay_fork   },
  { STAttackAdapter,  &insert_visit_battle_adapter },
  { STDefenseAdapter, &insert_visit_battle_adapter },
  { STHelpAdapter,    &insert_visit_help_adapter   }
};

enum
{
  nr_insertion_visitors = sizeof insertion_visitors / sizeof insertion_visitors[0]
};

/* Insert slices into a branch.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by slice_insertion_insert().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void slice_insertion_insert(slice_index si,
                            slice_index const prototypes[],
                            unsigned int nr_prototypes)
{
  stip_structure_traversal st;
  branch_slice_insertion_state_type state = {
      prototypes, nr_prototypes,
      slice_rank_order, nr_slice_rank_order_elmts, 0,
      branch_slice_rank_order_nonrecursive,
      0,
      si,
      0
  };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  state.base_rank = get_slice_rank(SLICE_TYPE(si),&state);
  assert(state.base_rank!=no_slice_rank);
  slice_insertion_init_traversal(&st,&state,stip_traversal_context_intro);
  move_init_slice_insertion_traversal(&st);
  stip_structure_traversal_override(&st,insertion_visitors,nr_insertion_visitors);
  stip_traverse_structure(si,&st);

  deallocate_slice_insertion_prototypes(prototypes,nr_prototypes);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert slices into a branch, taking into account the context of a structure
 * traversal that led to the insertion point.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by leaf_branch_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param context context of a traversal at slice is
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void slice_insertion_insert_contextually(slice_index si,
                                         stip_traversal_context_type context,
                                         slice_index const prototypes[],
                                         unsigned int nr_prototypes)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",context);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  switch (context)
  {
    case stip_traversal_context_intro:
      slice_insertion_insert(si,prototypes,nr_prototypes);
      break;

    case stip_traversal_context_attack:
      attack_branch_insert_slices(si,prototypes,nr_prototypes);
      break;

    case stip_traversal_context_defense:
      defense_branch_insert_slices(si,prototypes,nr_prototypes);
      break;

    case stip_traversal_context_help:
      help_branch_insert_slices(si,prototypes,nr_prototypes);
      break;

    case stip_traversal_context_move_generation:
      move_generation_branch_insert_slices(si,prototypes,nr_prototypes);
      break;

    case stip_traversal_context_test_square_observation:
      observation_branch_insert_slices(si,prototypes,nr_prototypes);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
