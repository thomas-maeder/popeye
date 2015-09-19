#include "options/maxtime.h"
#include "stipulation/pipe.h"
#include "platform/maxtime.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/slice_insertion.h"
#include "solving/pipe.h"
#include "solving/incomplete.h"
#include "output/plaintext/message.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Allocate a STMaxTimeGuard slice.
 * @return allocated slice
 */
slice_index alloc_maxtime_guard(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STMaxTimeGuard);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
void maxtime_guard_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_this_move_doesnt_solve_if(si,hasMaxtimeElapsed());

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
void maxtime_set(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (dealWithMaxtime())
  {
    pipe_solve_delegate(si);
    resetMaxtimeTimer();
  }
  else
  {
    output_plaintext_verifie_message(NoMaxTime);
    pipe_solve_delegate(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a STMaxTimeGuard slice after a STHelpMove slice
 */
static void insert_maxtime_help_guard(slice_index si,
                                      stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_maxtime_guard();
    help_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_maxtime_defender_guard(slice_index si,
                                          stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_maxtime_guard();
    attack_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor maxtime_guard_inserters[] =
{
  { STReadyForHelpMove, &insert_maxtime_help_guard     },
  { STReadyForAttack,   &insert_maxtime_defender_guard }
};
enum
{
  nr_maxtime_guard_inserters = (sizeof maxtime_guard_inserters
                                / sizeof maxtime_guard_inserters[0])
};

/* Instrument a stipulation with STMaxTimeGuard slices
 * @param si identifies slice where to start
 */
void solving_insert_maxtime_guards(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_conditional_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override(&st,
                                    maxtime_guard_inserters,
                                    nr_maxtime_guard_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* Propagate our findings to STProblemSolvingInterrupted
 * @param si identifies the slice where to start instrumenting
 */
void maxtime_propagator_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    slice_index const prototype = alloc_pipe(STMaxTimeSetter);
    slice_insertion_insert(si,&prototype,1);
  }

  pipe_solve_delegate(si);

  if (hasMaxtimeElapsed())
    phase_solving_remember_incompleteness(SLICE_NEXT2(si),
                                          solving_interrupted);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with option maxsolutions
 * @param si identifies the slice where to start instrumenting
 * @param maxtime
 */
void maxtime_instrument_solving(slice_index si, maxtime_type maxtime)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",maxtime);
  TraceFunctionParamListEnd();

  setOptionMaxtime(maxtime);

  {
    slice_index const interruption = branch_find_slice(STPhaseSolvingInterrupted,
                                                       si,
                                                       stip_traversal_context_intro);
    slice_index const prototype = alloc_pipe(STMaxTimePropagator);
    SLICE_NEXT2(prototype) = interruption;
    assert(interruption!=no_slice);
    slice_insertion_insert(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
