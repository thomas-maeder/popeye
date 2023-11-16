#include "options/stoponshortsolutions/stoponshortsolutions.h"
#include "options/stoponshortsolutions/filter.h"
#include "solving/incomplete.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/slice_insertion.h"
#include "solving/pipe.h"
#include "output/plaintext/message.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* remember that a short solution has been found
 * @param si identifies the slice that remembers
 */
void short_solution_found(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(SLICE_TYPE(si)==STStopOnShortSolutionsWasShortSolutionFound);

  SLICE_U(si).flag_handler.value = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the problem with a STStopOnShortSolutionsSolvingInstrumenter slice
 * @param si identifies the slice where to start instrumenting
 */
void stoponshortsolutions_problem_instrumenter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    slice_index const interruption = SLICE_NEXT2(si);
    slice_index const prototype = alloc_pipe(STStopOnShortSolutionsSolvingInstrumenter);
    assert(interruption!=no_slice);
    SLICE_NEXT2(prototype) = interruption;
    slice_insertion_insert(si,&prototype,1);
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the twin with option stop on short solutions
 * @param si identifies the slice where to start instrumenting
 */
void stoponshortsolutions_instrument_twin(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const interruption = branch_find_slice(STPhaseSolvingIncomplete,
                                                       si,
                                                       stip_traversal_context_intro);
    slice_index const prototype = alloc_pipe(STStopOnShortSolutionsProblemInstrumenter);
    SLICE_NEXT2(prototype) = interruption;
    assert(interruption!=no_slice);
    slice_insertion_insert(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Has a short solution been found in the current phase?
 */
boolean has_short_solution_been_found_in_phase(slice_index si)
{
  boolean const result = SLICE_U(si).flag_handler.value;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(SLICE_TYPE(si)==STStopOnShortSolutionsWasShortSolutionFound);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Propagate our findings to the phase solving interruption machinery
 * @param si slice index
 */
void stoponshortsolutions_was_short_solution_found_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  SLICE_U(si).flag_handler.value = false;

  pipe_solve_delegate(si);

  if (SLICE_U(si).flag_handler.value)
    phase_solving_remember_incompleteness(si,solving_partial);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


typedef struct
{
    boolean inserted;
    slice_index initialiser;
} insertion_type;

static void insert_filter(slice_index si, stip_structure_traversal *st)
{
  insertion_type * const insertion = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(insertion->initialiser!=no_slice);

  {
    stip_length_type const length = SLICE_U(si).branch.length;
    stip_length_type const min_length = SLICE_U(si).branch.min_length;
    slice_index const prototype = alloc_stoponshortsolutions_filter(length,
                                                                    min_length);
    SLICE_NEXT2(prototype) = insertion->initialiser;
    slice_insertion_insert(si,&prototype,1);
  }

  insertion->inserted = true;

  stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_initialiser(slice_index si, stip_structure_traversal *st)
{
  insertion_type * const insertion = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  insertion->initialiser = si;

  stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument help play
 * @param si identifies the slice where to start instrumenting
 */
void stoponshortsolutions_solving_instrumenter_solve(slice_index si)
{
  insertion_type insertion = { false, no_slice };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototype = alloc_pipe(STStopOnShortSolutionsWasShortSolutionFound);
    SLICE_NEXT2(prototype) = SLICE_NEXT2(si);
    slice_insertion_insert(si,&prototype,1);
  }

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,&insertion);
    stip_structure_traversal_override_by_contextual(&st,
                                                    slice_contextual_conditional_pipe,
                                                    &stip_traverse_structure_children_pipe);
    stip_structure_traversal_override_single(&st,STStopOnShortSolutionsWasShortSolutionFound,&remember_initialiser);
    stip_structure_traversal_override_single(&st,STHelpAdapter,&insert_filter);
    stip_traverse_structure(si,&st);
  }

  if (!insertion.inserted)
  {
    output_plaintext_message(NoStopOnShortSolutions);
    output_plaintext_message(NewLine);
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
