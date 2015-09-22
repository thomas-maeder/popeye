#include "optimisations/intelligent/limit_nr_solutions_per_target.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/help_play/branch.h"
#include "solving/has_solution_type.h"
#include "solving/pipe.h"
#include "solving/incomplete.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

typedef struct
{
    slice_index solving_instrumenter;
    slice_index initialiser;
} insertion_struct_type;

static void insert_initialiser(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->level==structure_traversal_level_top)
  {
    insertion_struct_type const * const insertion = st->param;
    slice_index const problem_instrumenter = SLICE_NEXT2(insertion->solving_instrumenter);
    slice_index const prototype = alloc_pipe(STIntelligentSolutionsPerTargetPosInitialiser);
    SLICE_NEXT2(prototype) = problem_instrumenter;
    help_branch_insert_slices(si,&prototype,1);
  }

  stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_initialiser(slice_index si, stip_structure_traversal *st)
{
  insertion_struct_type * const insertion = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  insertion->initialiser = si;

  stip_traverse_structure_children_pipe(si,st);

  insertion->initialiser = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_limiter(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    insertion_struct_type const * const insertion = st->param;
    slice_index const prototype = alloc_pipe(STIntelligentLimitNrSolutionsPerTargetPosLimiter);
    SLICE_NEXT2(prototype) = insertion->initialiser;
    help_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_counter(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    insertion_struct_type const * const insertion = st->param;
    slice_index const prototype = alloc_pipe(STIntelligentSolutionsPerTargetPosCounter);
    SLICE_NEXT2(prototype) = insertion->initialiser;
    help_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor inserters[] =
{
  { STHelpAdapter,       &insert_initialiser                    },
  { STIntelligentSolutionsPerTargetPosInitialiser, &remember_initialiser },
  { STReadyForHelpMove,  &insert_limiter                        },
  { STGoalReachedTester, &insert_counter                        },
  { STTemporaryHackFork, &stip_traverse_structure_children_pipe }
};

enum { nr_inserters = sizeof inserters / sizeof inserters[0] };

/* Propagate our findings to STProblemSolvingIncomplete
 * @param si identifies the slice where to start instrumenting
 */
void intelligent_nr_solutions_per_target_position_solving_instrumenter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    insertion_struct_type insertion = { si, no_slice };
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,&insertion);
    stip_structure_traversal_override_by_contextual(&st,
                                                    slice_contextual_conditional_pipe,
                                                    &stip_traverse_structure_children_pipe);
    stip_structure_traversal_override(&st,inserters,nr_inserters);
    stip_traverse_structure(si,&st);
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_nr_solutions_per_target_position_problem_instrumenter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    slice_index const prototype = alloc_pipe(STIntelligentSolutionsPerTargetPosSolvingInstrumenter);
    SLICE_NEXT2(prototype) = si;
    slice_insertion_insert(si,&prototype,1);
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with option intelligent n
 * @param si identifies the slice where to start instrumenting
 * @param max_nr_solutions_per_target_position
 */
void intelligent_nr_solutions_per_target_position_instrument_solving(slice_index si,
                                                                     unsigned long i)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",i);
  TraceFunctionParamListEnd();

  {
    slice_index const interruption = branch_find_slice(STPhaseSolvingIncomplete,
                                                       si,
                                                       stip_traversal_context_intro);
    slice_index const prototype = alloc_pipe(STIntelligentSolutionsPerTargetPosProblemInstrumenter);
    SLICE_NEXT2(prototype) = interruption;
    SLICE_U(prototype).value_handler.value = i;
    assert(interruption!=no_slice);
    slice_insertion_insert(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void intelligent_nr_solutions_per_target_position_counter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_solve_delegate(si);

  if (move_has_solved())
    ++SLICE_U(SLICE_NEXT2(si)).value_handler.value;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void intelligent_limit_nr_solutions_per_target_position_solve(slice_index si)
{
  slice_index const initialiser = SLICE_NEXT2(si);
  slice_index const problem_instrumenter = SLICE_NEXT2(initialiser);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (SLICE_U(SLICE_NEXT2(si)).value_handler.value
      >=SLICE_U(problem_instrumenter).value_handler.value)
    solve_result = MOVE_HAS_NOT_SOLVED_LENGTH();
  else
    pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void intelligent_nr_solutions_per_target_position_initialiser_solve(slice_index si)
{
  slice_index const problem_instrumenter = SLICE_NEXT2(si);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  SLICE_U(si).value_handler.value = 0;

  pipe_solve_delegate(si);

  if (SLICE_U(si).value_handler.value
      >=SLICE_U(problem_instrumenter).value_handler.value)
    phase_solving_remember_incompleteness(SLICE_NEXT2(problem_instrumenter),
                                          solving_partial);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
