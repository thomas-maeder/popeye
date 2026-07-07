#include "platform/heartbeat.h"
#include "solving/machinery/solve.h"
#include "solving/pipe.h"
#include "solving/incomplete.h"
#include "stipulation/pipe.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/branch.h"

#include "debugging/assert.h"
#include "debugging/trace.h"

#include <stdlib.h>

static heartbeat_type heartBeatCommandLine = no_rate_set;

/* Inform the heartbeat module about the value of the -heartbeat command
 * line parameter
 * @param commandlineValue value of the -heartbeat command line parameter
 */
void platform_set_commandline_heartbeat(heartbeat_type commandlineValue)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",commandlineValue);
  TraceFunctionParamListEnd();

  heartBeatCommandLine = commandlineValue;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a STHeartBeatWriter slice.
 * @return allocated slice
 */
static slice_index alloc_heartbeat_guard(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STHeartBeatWriter);

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
void heartbeat_writer_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();


  static unsigned int count = 0;
  ++count;
  if (count%heartBeatCommandLine==0)
  {
    if (fprintf(stderr,"heartbeat:%5d\n",count/heartBeatCommandLine)<0)
      exit(1);
    else
      ++count;
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a STHeartBeatGuard slice after a STHelpMove slice
 */
static void insert_heartbeat_help_guard(slice_index si,
                                        stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_heartbeat_guard();
    help_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_heartbeat_defender_guard(slice_index si,
                                            stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_heartbeat_guard();
    attack_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor inserters[] =
{
  { STReadyForHelpMove, &insert_heartbeat_help_guard     },
  { STReadyForAttack,   &insert_heartbeat_defender_guard }
};
enum { nr_guard_inserters = sizeof inserters / sizeof inserters[0] };

/* Instrument a stipulation with STHeartBeatGuard slices
 * @param si identifies slice where to start
 */
static void insert_guards(slice_index si)
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
  stip_structure_traversal_override(&st,inserters,nr_guard_inserters);
  stip_traverse_structure(si,&st);

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
void heartbeat_set(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  insert_guards(si);
  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery
 * @param si identifies the slice where to start instrumenting
 */
void heartbeat_problem_instrumenter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    slice_index const prototype = alloc_pipe(STHeartBeatSetter);
    slice_insertion_insert(si,&prototype,1);
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with option maxsolutions
 * @param si identifies the slice where to start instrumenting
 */
void heartbeat_instrument_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (heartBeatCommandLine!=no_rate_set)
  {
    slice_index const prototype = alloc_pipe(STHeartBeatProblemInstrumenter);
    slice_insertion_insert(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
