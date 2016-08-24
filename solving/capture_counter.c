#include "solving/capture_counter.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "solving/move_effect_journal.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* current value of the count */
unsigned int capture_counter_count;

/* stop the move iteration once capture_counter_count exceeds this number */
unsigned int capture_counter_interesting;

/* Allocate a STCaptureCounter slice.
 * @return index of allocated slice
 */
slice_index alloc_capture_counter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STCaptureCounter);

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
void capture_counter_solve(slice_index si)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = top+move_effect_journal_index_offset_capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (TSTFLAG(move_effect_journal[capture].u.piece_removal.flags,
              SLICE_STARTER(si)))
    ++capture_counter_count;

  TraceValue("%u",capture_counter_count);
  TraceValue("%u",capture_counter_interesting);
  TraceEOL();
  /* stop the iteration */
  pipe_this_move_doesnt_solve_if(si,capture_counter_count>capture_counter_interesting);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
