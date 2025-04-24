#include "options/movenumbers/restart_guard_intelligent.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "solving/machinery/slack_length.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "options/movenumbers.h"
#include "optimisations/intelligent/moves_left.h"
#include "platform/maxtime.h"
#include "options/options.h"
#include "output/output.h"
#include "output/plaintext/protocol.h"
#include "output/plaintext/message.h"

#include "debugging/assert.h"

static unsigned long nr_potential_target_positions;

/* Allocate a STRestartGuardIntelligent slice.
 * @return allocated slice
 */
slice_index alloc_restart_guard_intelligent(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STRestartGuardIntelligent);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_length_ruled_out_by_option_restart(void)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (OptFlag[startmovenumber])
  {
    stip_length_type min_length = 2*get_restart_number(movenumbers_start);
    if ((solve_nr_remaining-slack_length)%2==1)
      --min_length;
    if (solve_nr_remaining-slack_length<min_length)
      result = true;
  }

  if (OptFlag[uptomovenumber])
  {
    stip_length_type max_length = 2*get_restart_number(movenumbers_end);
    if ((solve_nr_remaining-slack_length)%2==1)
      ++max_length;
    if (solve_nr_remaining-slack_length>max_length)
      result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void print_nr_potential_target_positions(void)
{
  protocol_fputc('\n',stdout);
  output_plaintext_message(PotentialMates,
          nr_potential_target_positions,MovesLeft[White],MovesLeft[Black]);
  output_plaintext_print_time("  (",")");
  protocol_fflush(stdout);
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
void restart_guard_intelligent_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (is_length_ruled_out_by_option_restart())
    solve_result = MOVE_HAS_NOT_SOLVED_LENGTH();
  else
  {
    nr_potential_target_positions = 0;
    pipe_solve_delegate(si);
    /* TODO this should depend on the return value of phase_solving_completeness() */
    if (!platform_has_maxtime_elapsed())
      print_nr_potential_target_positions();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* Allocate a STIntelligentTargetCounter slice.
 * @return allocated slice
 */
slice_index alloc_intelligent_target_counter(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STIntelligentTargetCounter);

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
void intelligent_target_counter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  ++nr_potential_target_positions;
  TraceValue("%u",nr_potential_target_positions);
  TraceEOL();

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
